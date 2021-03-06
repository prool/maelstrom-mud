/**************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Envy Diku Mud improvements copyright (C) 1994 by Michael Quan, David   *
 *  Love, Guilherme 'Willie' Arnold, and Mitchell Tse.                     *
 *                                                                         *
 *  In order to use any part of this Envy Diku Mud, you must comply with   *
 *  the original Diku license in 'license.doc', the Merc license in        *
 *  'license.txt', as well as the Envy license in 'license.nvy'.           *
 *  In particular, you may not remove either of these copyright notices.   *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#define unix 1
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/*
 * Local functions.
 */
bool check_dodge( CHAR_DATA * ch, CHAR_DATA * victim );
void check_killer( CHAR_DATA * ch, CHAR_DATA * victim );
bool check_parry( CHAR_DATA * ch, CHAR_DATA * victim );
void dam_message( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt );
void death_cry( CHAR_DATA * ch );
void death_xp_loss( CHAR_DATA * victim );
void group_gain( CHAR_DATA * ch, CHAR_DATA * victim );
int xp_compute( CHAR_DATA * gch, CHAR_DATA * victim );
bool is_safe( CHAR_DATA * ch, CHAR_DATA * victim );
bool is_bare_hand( CHAR_DATA * ch );
bool is_wielding_poisoned( CHAR_DATA * ch );
void make_corpse( CHAR_DATA * ch );
void one_hit( CHAR_DATA * ch, CHAR_DATA * victi, int dt, bool dual );
void raw_kill( CHAR_DATA * ch, CHAR_DATA * victim );
void set_fighting( CHAR_DATA * ch, CHAR_DATA * victim );
void disarm( CHAR_DATA * ch, CHAR_DATA * victim );
void trip( CHAR_DATA * ch, CHAR_DATA * victim );
void item_damage( CHAR_DATA * ch, int dam );
void do_flip( CHAR_DATA * ch, char * argument );

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Slightly less efficient than Merc 2.2.  Takes 10% of
 *  total CPU time.
 */
void violence_update( void ) {
  CHAR_DATA * ch;
  CHAR_DATA * victim;
  CHAR_DATA * rch;
  bool        mobfighting;
  int         stun;

  for ( ch = char_list; ch; ch = ch->next ) {
    if ( !ch->in_room || ch->deleted ) {
      continue;
    }

    for ( stun = 0; stun < STUN_MAX; stun++ ) {
      if ( IS_STUNNED( ch, stun ) ) {
        ch->stunned[ stun ]--;
      }
    }

    if ( ( victim = ch->fighting ) ) {
      if ( IS_AWAKE( ch ) && ch->in_room == victim->in_room ) {
        multi_hit( ch, victim, TYPE_UNDEFINED );
      } else {
        stop_fighting( ch, FALSE );
      }

      continue;
    }

    if ( IS_AFFECTED( ch, AFF_BLIND )
         || ( IS_NPC( ch ) && ch->pIndexData->pShop ) ) {
      continue;
    }

    /* Ok. So ch is not fighting anyone.
     * Is there a fight going on?
     */

    mobfighting = FALSE;

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room ) {
      if ( rch->deleted
           || !IS_AWAKE( rch )
           || !( victim = rch->fighting ) ) {
        continue;
      }

      if ( !IS_NPC( ch )
           && ( !IS_NPC( rch ) || IS_AFFECTED( rch, AFF_CHARM ) )
           && is_same_group( ch, rch )
           && IS_NPC( victim ) ) {
        break;
      }

      if ( IS_NPC( ch )
           && IS_NPC( rch )
           && !IS_NPC( victim ) ) {
        mobfighting = TRUE;
        break;
      }
    }

    if ( !victim || !rch ) {
      continue;
    }

    /*
     * Now that someone is fighting, consider fighting another pc
     * or not at all.
     */
    if ( mobfighting ) {
      CHAR_DATA * vch;
      int         number;

      number = 0;

      for ( vch = ch->in_room->people; vch; vch = vch->next_in_room ) {
        if ( can_see( ch, vch )
             && ( vch->level > 5 )
             && is_same_group( vch, victim )
             && number_range( 0, number ) == 0 ) {
          victim = vch;
          number++;
        }
      }

      if ( ( rch->pIndexData != ch->pIndexData && number_bits( 3 ) != 0 )
           || ( IS_GOOD( ch ) && IS_GOOD( victim ) )
           || abs( victim->level - ch->level ) > 3 ) {
        continue;
      }
    }

    mprog_hitprcnt_trigger( ch, victim );
    mprog_fight_trigger( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );

  }

  return;
}

/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA * ch, CHAR_DATA * victim, int dt ) {
  if ( IS_NPC( ch ) ) {
    mprog_hitprcnt_trigger( ch, victim );
    mprog_fight_trigger( ch, victim );
  }

  if ( ( IS_AFFECTED2( ch, AFF_CONFUSED ) ) && number_percent() < 10 ) {
    act( AT_YELLOW, "$n looks around confused at what's going on.", ch, NULL, NULL, TO_ROOM );
    send_to_char( AT_YELLOW, "You stand confused.\n\r", ch );
    return;
  }

  one_hit( ch, victim, dt, FALSE );

  if ( ch->fighting != victim || dt == gsn_backstab ) {
    return;
  }

  one_hit( ch, victim, dt, TRUE );

  if ( ch->fighting != victim ) {
    return;
  }

  return;
}

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool dual ) {
  OBJ_DATA * wield;
  char       buf[ MAX_STRING_LENGTH ];
  int        victim_ac;
  int        thac0;
  int        dam;
  int        diceroll;

  /*
   * Can't beat a dead char!
   * Guard against weird room-leavings.
   */
  if ( victim->position == POS_DEAD || ch->in_room != victim->in_room ) {
    return;
  }

  if ( IS_STUNNED( ch, STUN_NON_MAGIC ) || IS_STUNNED( ch, STUN_TOTAL ) ) {
    return;
  }

  if ( dual ) {
    wield = get_eq_char( ch, WEAR_WIELD_2 );

    // gotta be dual wielding for a dual hit
    if ( !wield ) {
      return;
    }
  } else if ( !dual ) {
    wield = get_eq_char( ch, WEAR_WIELD );
  }

  /*
   * Figure out the type of damage message.
   */
  if ( dt == TYPE_UNDEFINED ) {
    dt = TYPE_HIT;

    if ( wield && wield->item_type == ITEM_WEAPON ) {
      dt += wield->value[ 3 ];
    }
  }

  /*
   * Calculate to-hit-armor-class-0 versus armor.
   */
  thac0 = GET_THAC0( ch );

  // add a penalty if they can't train in dual wielding
  if ( dual && !can_use_skpell( ch, gsn_dual ) ) {
    thac0 += class_table[ prime_class( ch ) ].skill_adept - (ch->pcdata->learned[ gsn_dual ] / 3);
  }

  victim_ac = UMAX( -15, GET_AC( victim ) / 10 );

  if ( !can_see( ch, victim ) ) {
    victim_ac -= 4;
  }

  /*
   * The moment of excitement!
   */
  diceroll = dice(1, 20);

  if ( diceroll == 1 || ( diceroll != 20 && diceroll < thac0 - victim_ac ) ) {
    damage( ch, victim, 0, dt );
    tail_chain();
    return;
  }

  if ( dual ) {
    update_skpell( ch, gsn_dual );
  }

  /*
   * Hit.
   * Calc damage.
   */
  if ( IS_NPC( ch ) ) {
    dam = number_range( ch->level / 3, ch->level * 3 / 2 );

    if ( wield ) {
      dam += dam / 3;
    }
  } else {
    if ( wield ) {
      dam = number_range( wield->value[ 1 ], wield->value[ 2 ] );
    } else {
      dam = number_range( 1, 4 );
    }

    if ( wield && dam > 1000 && !IS_IMMORTAL( ch ) ) {
      sprintf( buf, "One_hit dam range > 1000 from %d to %d", wield->value[ 1 ], wield->value[ 2 ] );
      bug( buf, 0 );

      if ( wield->name ) {
        bug( wield->name, 0 );
      }
    }
  }

  /*
   * Bonuses.
   */
  dam += GET_DAMROLL( ch );

  if ( wield && IS_SET( wield->extra_flags, ITEM_POISONED ) ) {
    dam += dam / 8;
  }

  if ( !IS_AWAKE( victim ) ) {
    dam *= 2;
  }

  if ( dt == gsn_backstab ) {
    dam *= 2 + UMIN( ( ch->level / 8 ), 4 );
  }

  if ( dam <= 0 ) {
    dam = 1;
  }

  damage( ch, victim, dam, dt );
  tail_chain();
  return;
}

/*
 * Inflict damage from a hit.
 */
void damage( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt ) {
  if ( victim->position == POS_DEAD ) {
    return;
  }

  if ( !IS_NPC( ch ) && !IS_NPC( victim ) ) {
    if ( !( ch == victim ) ) {
      ch->combat_timer     = 90;
      victim->combat_timer = 90;
    }
  }

  /*
   * Stop up any residual loopholes.
   */
  if ( dam > 3500 ) {
    char buf[ MAX_STRING_LENGTH ];

    if ( dt != 91 && ch->level <= LEVEL_HERO
         && dt != 40 ) {
      if ( IS_NPC( ch ) && ch->desc && ch->desc->original ) {
        sprintf( buf, "Damage: %d from %s by %s: > 3500 points with %d dt!", dam, ch->name, ch->desc->original->name, dt );
      } else {
        sprintf( buf, "Damage: %d from %s: > 3500 points with %d dt!", dam, ch->name, dt );
      }

      bug( buf, 0 );
    }
  }

  if ( victim != ch ) {
    /*
     * Certain attacks are forbidden.
     * Most other attacks are returned.
     */
    if ( is_safe( ch, victim ) ) {
      return;
    }

    check_killer( ch, victim );

    if ( victim->position > POS_STUNNED ) {
      if ( !victim->fighting ) {
        set_fighting( victim, ch );
      }

      victim->position = POS_FIGHTING;
    }

    if ( victim->position > POS_STUNNED ) {
      if ( !ch->fighting ) {
        set_fighting( ch, victim );
      }

      /*
       * If victim is charmed, ch might attack victim's master.
       */
      if (   IS_NPC( ch )
             && IS_NPC( victim )
             && IS_AFFECTED( victim, AFF_CHARM )
             && victim->master
             && victim->master->in_room == ch->in_room
             && number_bits( 3 ) == 0 ) {
        stop_fighting( ch, FALSE );
        set_fighting( ch, victim->master );
        return;
      }
    }

    /*
     * More charm stuff.
     */
    if ( victim->master == ch ) {
      stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED( ch, AFF_INVISIBLE ) ) {
      affect_strip( ch, gsn_invis );
      REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
      act( AT_GREY, "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * Check for disarm, trip, parry, and dodge.
     */
    if ( dt >= TYPE_HIT || dt == gsn_punch || dt == gsn_kick ) {
      int leveldiff = ch->level - victim->level;

      if ( IS_NPC( ch ) && number_percent() < ( leveldiff < -5 ? ch->level / 2 : UMAX( 10, leveldiff ) ) && dam == 0 && number_bits( 4 ) == 0 ) {
        disarm( ch, victim );
      }

      if ( IS_NPC( ch ) && number_percent() < ( leveldiff < -5 ? ch->level / 2 : UMAX( 20, leveldiff ) ) && dam == 0 && number_bits( 4 ) == 0 ) {
        trip( ch, victim );
      }

      if ( check_parry( ch, victim ) && dam > 0 ) {
        return;
      }

      if ( check_dodge( ch, victim ) && dam > 0 ) {
        return;
      }
    }

    if ( IS_SET( victim->act, UNDEAD_TYPE( victim ) ) ) {
      dam -= dam / 8;
    }

    if ( IS_AFFECTED( victim, AFF_PROTECT )
         && IS_EVIL( ch ) ) {
      dam -= dam / 4;
    }

    if ( dam < 0 ) {
      dam = 0;
    }

  }

  /* We moved dam_message out of the victim != ch if above
   * so self damage would show.  Other valid type_undefined
   * damage is ok to avoid like mortally wounded damage - Kahn
   */
  if ( ( !IS_NPC( ch ) ) && ( !IS_NPC( victim ) ) ) {
    dam -= dam / 4;
  }

  if ( dt != TYPE_UNDEFINED ) {
    dam_message( ch, victim, dam, dt );
  }

  /*
   * Hurt the victim.
   * Inform the victim of his new state.
   */
  if ( !IS_NPC( ch ) && prime_class( ch ) == CLASS_FIGHTER ) {
    dam += dam / 2;
  }

  if ( !IS_NPC( ch ) && !IS_NPC( victim ) ) {
    dam /= number_range( 2, 4 );
  }

  if ( dam > 25 && number_range( 0, 100 ) <= 15 ) {
    item_damage( victim, dam );
  }

  victim->hit -= dam;

  if ( ( ( !IS_NPC( victim )                    /* so imms only die by */
           && IS_NPC( ch )                      /* the hands of a PC   */
           && victim->level >= LEVEL_IMMORTAL )
         ||
         ( !IS_NPC( victim )                  /* so imms don,t die  */
           && victim->level >= LEVEL_IMMORTAL /* by poison type dmg */
           && ch == victim ) )                /* since an imm == pc */
       && victim->hit < 1 ) {
    victim->hit = 1;
  }

  if ( dam > 0 && dt > TYPE_HIT && ( ( is_wielding_poisoned( ch ) && !saves_spell( ch->level, victim ) ) ) ) {
    AFFECT_DATA af;

    af.type      = gsn_poison;
    af.duration  = 1;
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );
  }

  update_pos( victim );

  switch ( victim->position ) {
    case POS_MORTAL:
      send_to_char( AT_RED,
                    "You are mortally wounded, and will die soon, if not aided.\n\r",
                    victim );
      act( AT_RED, "$n is mortally wounded, and will die soon, if not aided.",
           victim, NULL, NULL, TO_ROOM );
      break;

    case POS_INCAP:
      send_to_char( AT_RED,
                    "You are incapacitated and will slowly die, if not aided.\n\r",
                    victim );
      act( AT_RED, "$n is incapacitated and will slowly die, if not aided.",
           victim, NULL, NULL, TO_ROOM );
      break;

    case POS_STUNNED:
      send_to_char( AT_WHITE, "You are stunned, but will probably recover.\n\r",
                    victim );
      act( AT_WHITE, "$n is stunned, but will probably recover.",
           victim, NULL, NULL, TO_ROOM );
      break;

    case POS_DEAD:
      send_to_char( AT_BLOOD, "You have been KILLED!!\n\r\n\r", victim );
      act( AT_BLOOD, "$n is DEAD!!", victim, NULL, NULL, TO_ROOM );
      break;

    default:

      if ( dam > MAX_HIT( victim ) / 4 ) {
        send_to_char( AT_RED, "That really did HURT!\n\r", victim );
      }

      if ( victim->hit < MAX_HIT( victim ) / 4 ) {
        send_to_char( AT_RED, "You sure are BLEEDING!\n\r", victim );
      }

      break;
  }

  /*
   * Sleep spells and extremely wounded folks.
   */
  if ( !IS_AWAKE( victim ) ) {
    stop_fighting( victim, FALSE );
  }

  /*
   * Payoff for killing things.
   */
  if ( victim->position == POS_DEAD ) {
    group_gain( ch, victim );

    if ( ( !IS_NPC( ch ) ) && ( !IS_NPC( victim ) ) ) {
      CLAN_DATA * pClan;
      CLAN_DATA * Cland;

      if ( ch->clan != victim->clan ) {
        if ( ( pClan = get_clan_index( ch->clan ) ) != NULL ) {
          pClan->pkills++;
        }

        if ( ( Cland = get_clan_index( victim->clan ) ) != NULL ) {
          Cland->pdeaths++;
        }
      }

      /*            REMOVE_BIT(victim->act, PLR_THIEF);*/
    }

    if ( ( !IS_NPC( ch ) ) && ( IS_NPC( victim ) ) ) {
      CLAN_DATA * pClan;

      if ( ( pClan = get_clan_index( ch->clan ) ) != NULL ) {
        pClan->mkills++;
      }
    }

    if ( ( IS_NPC( ch ) ) && ( !IS_NPC( victim ) ) ) {
      CLAN_DATA * pClan;

      if ( ( pClan = get_clan_index( victim->clan ) ) != NULL ) {
        pClan->mdeaths++;
      }
    }

    if ( !IS_NPC( victim ) ) {
      if ( !IS_NPC( ch ) && ch != victim ) {
        ch->pkills++;
        victim->pkilled++;
      }

      /*
        * Dying penalty:
        * 1/2 way back to previous level.
        */
      if ( victim->level < LEVEL_HERO
            || ( victim->level >= LEVEL_HERO && IS_NPC( ch ) ) ) {
        /*		death_xp_loss( victim ); */
        sprintf( log_buf, "%s killed by %s at %d.", victim->name,
                  ch->name, victim->in_room->vnum );
      }

      log_string( log_buf, CHANNEL_LOG, -1 );
      wiznet( log_buf, NULL, NULL, WIZ_DEATHS, 0, 0 );
      info( "%s gets slaughtered by %s!", (int)victim->name, (int)( IS_NPC( ch ) ? ch->short_descr : ch->name ) );
      save_clans();
    }

    raw_kill( ch, victim );

    /* Ok, now we want to remove the deleted flag from the
     * PC victim.
     */
    if ( !IS_NPC( victim ) ) {
      victim->deleted = FALSE;
    }

    if ( !IS_NPC( ch ) && IS_NPC( victim ) ) {
      if ( IS_SET( ch->act, PLR_AUTOLOOT ) ) {
        do_get( ch, "all corpse" );
      } else {
        do_look( ch, "in corpse" );
      }

      if ( IS_SET( ch->act, PLR_AUTOCOINS ) ) {
        do_get( ch, "all.coin corpse" );
      }

      if ( IS_SET( ch->act, PLR_AUTOSAC ) ) {
        do_sacrifice( ch, "corpse" );
      }
    }

    return;
  }

  if ( victim == ch ) {
    return;
  }

  /*
   * Take care of link dead people.
   */
  if ( !IS_NPC( victim ) && !victim->desc ) {
    if ( number_range( 0, victim->wait ) == 0 ) {
      do_recall( victim, "" );
      return;
    }
  }

  /*
   * Wimp out?
   */
  if ( IS_NPC( victim ) && dam > 0 ) {
    if ( ( IS_SET( victim->act, ACT_WIMPY ) && number_bits( 1 ) == 0
           && victim->hit < MAX_HIT( victim ) / 2 )
         || ( IS_AFFECTED( victim, AFF_CHARM ) && victim->master
              && victim->master->in_room != victim->in_room ) ) {
      do_flee( victim, "" );
    }
  }

  if ( !IS_NPC( victim )
       && victim->hit   > 0
       && victim->hit  <= victim->wimpy
       && victim->wait == 0 ) {
    do_flee( victim, "" );
  }

  tail_chain();
  return;
}

void item_damage( CHAR_DATA * ch, int dam ) {
  OBJ_DATA * obj_lose;
  OBJ_DATA * obj_next;

  for ( obj_lose = ch->carrying; obj_lose; obj_lose = obj_next ) {
    char * msg;

    obj_next = obj_lose->next_content;

    if ( obj_lose->deleted ) {
      continue;
    }

    if ( number_bits( 2 ) != 0 ) {
      continue;
    }

    /* Check if total cost in copper is less than 500,000 */

    if ( ( ( obj_lose->pIndexData->cost.gold * C_PER_G ) +
           ( obj_lose->pIndexData->cost.silver * S_PER_G ) +
           ( obj_lose->pIndexData->cost.copper ) ) < 5000 * 100 ) {
      continue;
    }

    if ( obj_lose->wear_loc == WEAR_NONE ) {
      continue;
    }

    if ( IS_SET( obj_lose->extra_flags, ITEM_NO_DAMAGE ) ) {
      continue;
    }

    switch ( obj_lose->item_type ) {
      default:
        msg = "Your $p gets ruined!";
        extract_obj( obj_lose );
        break;
      case ITEM_DRINK_CON:
      case ITEM_POTION:
      case ITEM_CONTAINER:
      case ITEM_LIGHT:
        msg = "Your $p shatters!";
        extract_obj( obj_lose );
        break;
      case ITEM_WEAPON:
      case ITEM_ARMOR:

        /*		 if ( ( obj_lose->cost.gold +
             (obj_lose->cost.silver/SILVER_PER_GOLD) +
             (obj_lose->cost.copper/COPPER_PER_GOLD) ) != 0 )
         */
        if ( ( ( obj_lose->cost.gold * 100 ) +
               ( obj_lose->cost.silver * 10 ) +
               ( obj_lose->cost.copper ) ) != 0 ) {
          obj_lose->cost.gold = ( obj_lose->cost.gold > 0 ) ?
                                ( obj_lose->cost.gold - dam / 6 ) : 0;
          obj_lose->cost.silver = ( obj_lose->cost.silver > 0 ) ?
                                  ( obj_lose->cost.silver - dam / 6 ) : 0;
          obj_lose->cost.copper = ( obj_lose->cost.copper > 0 ) ?
                                  ( obj_lose->cost.copper - dam / 6 ) : 0;
        }

        /*		 if ( (obj_lose->cost.gold +
             (obj_lose->cost.silver/SILVER_PER_GOLD) +
             (obj_lose->cost.copper/COPPER_PER_GOLD) ) < 0 )
         */
        if ( ( ( obj_lose->cost.gold * 100 ) +
               ( obj_lose->cost.silver * 10 ) +
               ( obj_lose->cost.copper ) ) < 0 ) {
          OBJ_DATA       * pObj;
          OBJ_INDEX_DATA * pObjIndex;
          char           * name;
          char             buf[ MAX_STRING_LENGTH ];

          pObjIndex = get_obj_index( OBJ_VNUM_TRASH );
          pObj      = create_object( pObjIndex, obj_lose->level );
          name      = obj_lose->short_descr;
          sprintf( buf, pObj->description, name );
          free_string( pObj->description );
          pObj->description = str_dup( buf );
          pObj->weight      = obj_lose->weight;
          pObj->timer       = obj_lose->level;
          msg               = "$p has been destroyed!";
          extract_obj( obj_lose );
          obj_to_room( pObj, ch->in_room );
        } else {
          msg = "$p has been damaged!";
        }

        break;

    }

    act( AT_YELLOW, msg, ch, obj_lose, NULL, TO_CHAR );
    return;
  }

  return;
}

bool is_safe( CHAR_DATA * ch, CHAR_DATA * victim ) {
  CLAN_DATA * pClan;

  if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
       IS_SET( victim->in_room->room_flags, ROOM_SAFE ) ) {
    return TRUE;
  }

  if ( !IS_NPC( ch ) && !IS_NPC( victim ) &&
       ( IS_SET( ch->in_room->room_flags, ROOM_NO_PKILL ) ||
         IS_SET( victim->in_room->room_flags, ROOM_NO_PKILL ) ) ) {
    return TRUE;
  }

  if ( IS_AFFECTED( ch, AFF_PEACE ) ) {
    return TRUE;
  }

  if ( IS_SET( ch->in_room->room_flags, ROOM_PKILL ) &&
       IS_SET( victim->in_room->room_flags, ROOM_PKILL ) ) {
    return FALSE;
  }

  if ( IS_NPC( victim ) ) {
    return FALSE;
  }

  if ( abs( ch->level - victim->level ) > 5 && ( !IS_NPC( ch ) ) ) {
    send_to_char( AT_WHITE, "That is not in the pkill range... valid range is +/- 5 levels.\n\r", ch );
    return TRUE;
  }

  if ( IS_NPC( ch ) ) {
    if ( IS_SET( ch->affected_by, AFF_CHARM ) && ch->master ) {
      CHAR_DATA * nch;

      for ( nch = ch->in_room->people; nch; nch = nch->next ) {
        if ( nch == ch->master ) {
          break;
        }
      }

      if ( nch == NULL ) {
        return FALSE;
      } else {
        ch = nch; /* Check person who ordered mob for clan stuff.. */
      }
    } else {
      return FALSE;
    }
  }

  pClan = get_clan_index( ch->clan );

  if ( ( ch->clan == 0 ) && ( !IS_SET( pClan->settings, CLAN_PKILL ) ) ) {
    send_to_char( AT_WHITE, "You must be clanned to murder.\n\r", ch );
    return TRUE;
  }

  pClan = get_clan_index( victim->clan );

  if ( ( victim->clan == 0 ) && ( !IS_SET( pClan->settings, CLAN_PKILL ) ) ) {
    send_to_char( AT_WHITE, "You can only murder clanned players.\n\r", ch );
    return TRUE;
  }

  pClan = get_clan_index( ch->clan );

  if ( ch->clan == victim->clan &&
       IS_SET( pClan->settings, CLAN_CIVIL_PKILL ) ) {
    return FALSE;
  }

  /* can murder self for testing =) */
  if ( ch->clan == victim->clan && ch != victim && ch->clan != 0 ) {
    send_to_char( AT_WHITE, "You cannot murder your own clan member.\n\r", ch );
    return TRUE;
  }

  if ( !IS_SET( pClan->settings, CLAN_PKILL ) ) {
    send_to_char( AT_WHITE, "Peaceful clan members cannot murder.\n\r", ch );
    return TRUE;
  }

  pClan = get_clan_index( victim->clan );

  if ( !IS_SET( pClan->settings, CLAN_PKILL ) ) {
    send_to_char( AT_WHITE, "You may not murder peaceful clan members.\n\r", ch );
    return TRUE;
  }

  if ( IS_SET( victim->act, PLR_KILLER ) ) {
    return FALSE;
  }

  return FALSE;
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA * ch, CHAR_DATA * victim ) {
  CLAN_DATA * pClan;
  char        buf[ MAX_STRING_LENGTH ];

  /*
   * NPC's are fair game.
   * So are killers and thieves.
   */
  if (   IS_NPC( victim )
         || IS_SET( victim->act, PLR_KILLER )
         || IS_SET( victim->act, PLR_THIEF ) ) {
    return;
  }

  /*
   * NPC's are cool of course
   * Hitting yourself is cool too (bleeding).
   * And current killers stay as they are.
   */
  if ( IS_NPC( ch )
       || ch == victim
       || IS_SET( ch->act, PLR_KILLER )
       || IS_SET( ch->act, PLR_THIEF ) ) {
    return;
  }

  pClan = get_clan_index( ch->clan );

  if ( /*ch->clan != 0 ||*/ ( IS_SET( pClan->settings, CLAN_PKILL ) ) ||
                            ( ch->clan == victim->clan && IS_SET( pClan->settings, CLAN_CIVIL_PKILL ) ) ) {
    return;
  }

  send_to_char( AT_RED, "*** You are now a KILLER!! ***\n\r", ch );
  sprintf( buf, "$N is attempting to murder %s", victim->name );
  wiznet( buf, ch, NULL, WIZ_FLAGS, 0, 0 );
  SET_BIT( ch->act, PLR_KILLER );
  save_char_obj( ch );
  return;
}

bool is_bare_hand( CHAR_DATA * ch ) {
  if ( !get_eq_char( ch, WEAR_WIELD )
       && !get_eq_char( ch, WEAR_WIELD_2 ) ) {
    return TRUE;
  }

  return FALSE;
}

/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned( CHAR_DATA * ch ) {
  OBJ_DATA * obj;

  if ( ( obj = get_eq_char( ch, WEAR_WIELD ) )
       && IS_SET( obj->extra_flags, ITEM_POISONED ) ) {
    return TRUE;
  }

  return FALSE;

}

/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA * ch, CHAR_DATA * victim ) {
  int chance;

  if ( !IS_AWAKE( victim ) ) {
    return FALSE;
  }

  // can't parry without a weapon
  if ( !get_eq_char( victim, WEAR_WIELD ) ) {
    return FALSE;
  }

  // can't really accidentally parry, gotta learn how first
  if ( !IS_NPC(victim) && victim->pcdata->learned[ gsn_parry ] <= 0 ) {
    return FALSE;
  }

  if ( IS_NPC( victim ) ) {
    chance = UMIN( 60, 2 * victim->level );
  } else {
    chance = victim->pcdata->learned[ gsn_parry ] / 2;
  }

  if ( ch->wait != 0 ) {
    chance /= 4;
  }

  if ( number_percent() >= chance + victim->level - ch->level ) {
    return FALSE;
  }

  update_skpell( victim, gsn_parry );

  if ( IS_SET( ch->act, PLR_COMBAT ) ) {
    act( AT_GREEN, "$N parries your attack.", ch, NULL, victim, TO_CHAR );
  }

  if ( IS_SET( victim->act, PLR_COMBAT ) ) {
    act( AT_GREEN, "You parry $n's attack.", ch, NULL, victim, TO_VICT );
  }

  return TRUE;
}

/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA * ch, CHAR_DATA * victim ) {
  int  chance;

  if ( !IS_AWAKE( victim ) ) {
    return FALSE;
  }

  if ( IS_NPC( victim ) ) {
    chance = UMIN( 60, 2 * victim->level );
  } else {
    // always a miniscule chance of dodging, at least through sheer dumb luck
    chance = UMAX(2, victim->pcdata->learned[ gsn_dodge ] / 2);
  }

  if ( ch->wait != 0 ) {
    chance /= 4;
  }

  if ( number_percent() >= chance + victim->level - ch->level ) {
    return FALSE;
  }

  update_skpell( victim, gsn_dodge );

  if ( IS_SET( ch->act, PLR_COMBAT ) ) {
    act( AT_GREEN, "$N dodges your attack.", ch, NULL, victim, TO_CHAR );
  }

  if ( IS_SET( victim->act, PLR_COMBAT ) ) {
    act( AT_GREEN, "You dodge $n's attack.", ch, NULL, victim, TO_VICT );
  }

  return TRUE;
}

/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA * victim ) {
  if ( victim->hit > 0 ) {
    if ( victim->position < POS_STUNNED ) {
      victim->position = POS_STANDING;
    }

    return;
  }

  if ( IS_NPC( victim ) || victim->hit <= -11 ) {
    victim->position = POS_DEAD;
    return;
  }

  if ( victim->hit <= -6 ) {
    victim->position = POS_MORTAL;
  } else if ( victim->hit <= -3 ) {
    victim->position = POS_INCAP;
  } else {
    victim->position = POS_STUNNED;
  }

  return;
}

/*
 * Start fights.
 */
void set_fighting( CHAR_DATA * ch, CHAR_DATA * victim ) {

  char buf[ MAX_STRING_LENGTH ];

  if ( ch->fighting ) {
    bug( "Set_fighting: already fighting", 0 );
    sprintf( buf, "...%s attacking %s at %d",
             ( IS_NPC( ch )     ? ch->short_descr     : ch->name     ),
             ( IS_NPC( victim ) ? victim->short_descr : victim->name ),
             victim->in_room->vnum );
    bug( buf, 0 );
    return;
  }

  if ( IS_AFFECTED( ch, AFF_SLEEP ) ) {
    affect_strip( ch, gsn_sleep );
  }

  ch->fighting = victim;
  ch->position = POS_FIGHTING;

  return;
}

/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA * ch, bool fBoth ) {
  CHAR_DATA * fch;

  for ( fch = char_list; fch; fch = fch->next ) {
    if ( fch == ch || ( fBoth && fch->fighting == ch ) ) {
      fch->fighting = NULL;
      fch->hunting  = NULL;
      fch->position = POS_STANDING;

      update_pos( fch );
    }
  }

  return;
}

/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA * ch ) {
  OBJ_DATA * corpse;
  OBJ_DATA * obj;
  OBJ_DATA * obj_next;
  /*    ROOM_INDEX_DATA *location;*/
  char     * name;
  char       buf[ MAX_STRING_LENGTH ];
  OBJ_DATA * random;
  int        level = ch->level;

  if ( !IS_NPC( ch ) && ch->level <= 20 ) {
    char_from_room( ch );
    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
  }

  if ( IS_NPC( ch ) ) {
    /*
     * This longwinded corpse creation routine comes about because
     * we dont want anything created AFTER a corpse to be placed
     * INSIDE a corpse.  This had caused crashes from obj_update()
     * in extract_obj() when the updating list got shifted from
     * object_list to obj_free.          --- Thelonius (Monk)
     */
    if ( ( ch->money.gold > 0 ) || ( ch->money.silver > 0 ) ||
         ( ch->money.copper > 0 ) ) {
      OBJ_DATA * coins;
      coins  = create_money( &ch->money );
      name   = ch->short_descr;
      corpse = create_object(
        get_obj_index( OBJ_VNUM_CORPSE_NPC ),
        0 );
      corpse->timer = number_range( 2, 4 );
      obj_to_obj( coins, corpse );
      ch->money.gold = ch->money.silver = ch->money.copper = 0;
    } else {
      name   = ch->short_descr;
      corpse = create_object(
        get_obj_index( OBJ_VNUM_CORPSE_NPC ),
        0 );
      corpse->timer = number_range( 2, 4 );
    }
  } else {
    name   = ch->name;
    corpse = create_object(
      get_obj_index( OBJ_VNUM_CORPSE_PC ),
      0 );
    corpse->timer = number_range( 25, 40 );
    /* Check if ch has any money, doesn't matter about converting */

    if ( ( ( ch->money.gold + ch->money.silver +
             ch->money.copper ) > 0 ) &&
         ( ch->level > 5 ) ) {
      OBJ_DATA * coins;
      coins = create_money( &ch->money );
      obj_to_obj( coins, corpse );
      ch->money.gold = ch->money.silver = ch->money.copper = 0;
    }
  }

  sprintf( buf, corpse->short_descr, name );
  free_string( corpse->short_descr );
  corpse->short_descr = str_dup( buf );

  sprintf( buf, corpse->description, name );
  free_string( corpse->description );
  corpse->description = str_dup( buf );

  for ( obj = ch->carrying; obj; obj = obj_next ) {
    obj_next = obj->next_content;

    if ( obj->deleted ) {
      continue;
    }

    obj_from_char( obj );

    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) ) {
      extract_obj( obj );
    } else {
      obj_to_obj( obj, corpse );
    }
  }

  if ( ( number_percent() > 50 ) && IS_NPC( ch ) ) {
    random = random_object( level );
    obj_to_obj( random, corpse );
  }

  if ( ( IS_NPC( ch ) ) && ( !IS_SET( ch->act, UNDEAD_TYPE( ch ) ) ) ) {
    corpse->ac_vnum = ch->pIndexData->vnum;
  }

  obj_to_room( corpse, ch->in_room );

  if ( !IS_NPC( ch ) ) {
    corpse_back( ch, corpse );
  }

  return;
}

/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA * ch ) {
  ROOM_INDEX_DATA * was_in_room;
  char            * msg;
  int               vnum;
  int               door;
  OBJ_DATA        * obj;

  vnum = 0;

  switch ( number_bits( 4 ) ) {
    default:
      msg = "You hear $n's death cry.";
      break;
    case 0:
      msg = "$n hits the ground ... DEAD.";
      break;
    case 1:
      msg = "$n splatters blood on your armor.";
      break;
    case 2:
      msg = "$n's innards fall to the ground with a wet splat.";
      break;
    case 3:
      msg  = "$n's severed head plops on the ground.";
      vnum = OBJ_VNUM_SEVERED_HEAD;
      break;
    case 4:
      msg  = "$n's heart is torn from $s chest.";
      vnum = OBJ_VNUM_TORN_HEART;
      break;
    case 5:
      msg  = "$n's arm is sliced from $s dead body.";
      vnum = OBJ_VNUM_SLICED_ARM;
      break;
    case 6:
      msg  = "$n's leg is sliced from $s dead body.";
      vnum = OBJ_VNUM_SLICED_LEG;
      break;
  }

  act( AT_BLOOD, msg, ch, NULL, NULL, TO_ROOM );

  if ( vnum != 0 ) {
    char * name;
    char   buf[ MAX_STRING_LENGTH ];

    name       = IS_NPC( ch ) ? ch->short_descr : ch->name;
    obj        = create_object( get_obj_index( vnum ), 0 );
    obj->timer = number_range( 4, 7 );

    sprintf( buf, obj->short_descr, name );
    free_string( obj->short_descr );
    obj->short_descr = str_dup( buf );

    sprintf( buf, obj->description, name );
    free_string( obj->description );
    obj->description = str_dup( buf );

    obj_to_room( obj, ch->in_room );
  }

  obj        = create_object( get_obj_index( OBJ_VNUM_FINAL_TURD ), 0 );
  obj->timer = number_range( 3, 5 );
  obj_to_room( obj, ch->in_room );

  if ( IS_NPC( ch ) ) {
    msg = "You hear something's death cry.";
  } else {
    msg = "You hear someone's death cry.";
  }

  was_in_room = ch->in_room;

  for ( door = 0; door < MAX_DIR; door++ ) {
    EXIT_DATA * pexit;

    if ( ( pexit = was_in_room->exit[ door ] )
         && pexit->to_room
         && pexit->to_room != was_in_room ) {
      ch->in_room = pexit->to_room;
      act( AT_BLOOD, msg, ch, NULL, NULL, TO_ROOM );
    }
  }

  ch->in_room = was_in_room;

  return;
}

void raw_kill( CHAR_DATA * ch, CHAR_DATA * victim ) {
  AFFECT_DATA * paf;
  AFFECT_DATA * paf_next;

  stop_fighting( victim, TRUE );

  if ( ch != victim ) {
    mprog_death_trigger( victim, ch );
  }

  rprog_death_trigger( victim->in_room, victim );
  make_corpse( victim );

  for ( paf = victim->affected; paf; paf = paf_next ) {
    paf_next = paf->next;
    affect_remove( victim, paf );
  }

  for ( paf = victim->affected2; paf; paf = paf_next ) {
    paf_next = paf->next;
    affect_remove2( victim, paf );
  }

  victim->affected_by  = 0;
  victim->affected_by2 = 0;

  if ( IS_NPC( victim ) ) {
    victim->pIndexData->killed++;
    kill_table[ URANGE( 0, victim->level, MAX_LEVEL - 1 ) ].killed++;
    extract_char( victim, TRUE );
    return;
  }

  extract_char( victim, FALSE );

  victim->armor        = 100;
  victim->hitroll      = 0;
  victim->damroll      = 0;
  victim->saving_throw = 0;
  victim->carry_weight = 0;
  victim->carry_number = 0;

  victim->position = POS_RESTING;
  victim->hit      = UMAX( 1, victim->hit );
  victim->mana     = UMAX( 1, victim->mana );
  victim->move     = UMAX( 1, victim->move );
  save_char_obj( victim );
  return;
}

void group_gain( CHAR_DATA * ch, CHAR_DATA * victim ) {
  CHAR_DATA * gch;
  CHAR_DATA * lch;
  char        buf[ MAX_STRING_LENGTH ];
  int         members;
  int         xp;

  /*
   * Monsters don't get kill xp's or alignment changes.
   * P-killing doesn't help either.
   * Dying of mortal wounds or poison doesn't give xp to anyone!
   */
  if ( IS_NPC( ch ) || victim == ch ) {
    return;
  }

  members = 0;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room ) {
    if ( is_same_group( gch, ch ) ) {
      members++;
    }
  }

  if ( members == 0 ) {
    bug( "Group_gain: members.", members );
    members = 1;
  }

  lch = ( ch->leader ) ? ch->leader : ch;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room ) {
    OBJ_DATA * obj;
    OBJ_DATA * obj_next;

    if ( !is_same_group( gch, ch ) ) {
      continue;
    }

    if ( gch->level - lch->level >= 6 ) {
      send_to_char( AT_BLUE, "You are too high level for this group.\n\r", gch );
      continue;
    }

    if ( gch->level - lch->level <= -6 ) {
      send_to_char( AT_BLUE, "You are too low level for this group.\n\r", gch );
      continue;
    }

    xp = xp_compute( gch, victim ) / members;
    /*    sprintf( buf, "%s -> gains %dxp", gch->name, xp);
        log_string( buf, CHANNEL_NONE, -1 );*/
    sprintf( buf, "You receive %d experience points.\n\r", xp );
    send_to_char( AT_WHITE, buf, gch );
    gain_exp( gch, xp );

    for ( obj = ch->carrying; obj; obj = obj_next ) {
      obj_next = obj->next_content;

      if ( obj->deleted ) {
        continue;
      }

      if ( obj->wear_loc == WEAR_NONE ) {
        continue;
      }
    }
  }

  if ( IS_SET( ch->act, PLR_QUESTOR ) && IS_NPC( victim ) ) {
    if ( ch->questmob && victim == ch->questmob ) {
      send_to_char( AT_WHITE, "You have almost completed your QUEST!\n\r", ch );
      send_to_char( AT_WHITE, "Return to the QuestMaster before your time runs out!\n\r", ch );
      ch->questmob = NULL;
    }
  }

  return;
}

/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA * gch, CHAR_DATA * victim ) {
  int xp;
  int xp_cap = 750;
  int align;

  align = gch->alignment - victim->alignment;

  if ( align > 500 ) {
    gch->alignment = UMIN( gch->alignment + ( align - 500 ) / 4, 1000 );
  } else if ( align < -500 ) {
    gch->alignment = UMAX( gch->alignment + ( align + 500 ) / 4, -1000 );
  } else {
    gch->alignment -= victim->alignment / 3;
  }

  /* mob lvl is 5 lvls lower than pc or more */
  if ( victim->level + 5 <= gch->level ) {
    return 0;
  }

  /* 3-4 levels lower */
  if ( victim->level + 3 == gch->level
       || victim->level + 4 == gch->level ) {
    xp = ( gch->level < 10 ) ? number_range( 50, 100 ) + 10 : 0;
    return xp;
  }

  /* if same lvl or up to 2 lvls lower */
  if ( victim->level > gch->level - 3
       && victim->level <= gch->level ) {
    xp = number_range( 0, 10 ) + number_range( 10, 15 );
    xp = ( gch->level < 10 ) ? xp + number_range( 50, 100 ) : xp;
    return xp;
  }

  /* if higher lvl then... */
  xp = ( victim->level - gch->level ) * number_range( 25, 40 );
  xp = ( gch->level < 10 ) ? xp + number_range( 35, 50 ) : xp;

  /* if they kill 5 lvls bigger then them or more add 0-50xp */
  xp = ( victim->level >= gch->level + 5 ) ? xp + number_range( 0, 50 )
       : xp;
  /* Enforce xp cap */
  xp = UMIN( xp_cap, xp );
  xp = UMAX( 0, xp );

  /* Xp Boost */
  if ( doubleexp() == TRUE && gch->level <= 15 ) {
    xp = xp * 4;
  } else if ( doubleexp() == TRUE && gch->level <= 30 && gch->level > 15 ) {
    xp = xp * 3;
  } else if ( doubleexp() == TRUE && gch->level <= 50 && gch->level > 30 ) {
    xp = xp * 2;
  }

  return xp;
}

void dam_message( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt ) {
  static char * const attack_table [] = {
    "hit",
    "slice", "stab",   "slash", "whip",     "claw",
    "blast", "pound",  "crush", "grep",     "bite",
    "pierce", "suction", "chop", "left fist", "right fist"
  };
  const char        * vs;
  const char        * vp;
  const char        * attack;
  char                buf[ MAX_STRING_LENGTH ];
  char                buf1[ 256 ];
  char                buf2[ 256 ];
  char                buf3[ 256 ];
  char                buf4[ 256 ];
  char                buf5[ 256 ];
  char                punct;

  if ( dam ==   0 ) {
    vs = "miss";
    vp = "misses";
  } else if ( dam <=   4 ) {
    vs = "scratch";
    vp = "scratches";
  } else if ( dam <=   8 ) {
    vs = "graze";
    vp = "grazes";
  } else if ( dam <=  12 ) {
    vs = "hit";
    vp = "hits";
  } else if ( dam <=  16 ) {
    vs = "injure";
    vp = "injures";
  } else if ( dam <=  20 ) {
    vs = "wound";
    vp = "wounds";
  } else if ( dam <=  24 ) {
    vs = "maul";
    vp = "mauls";
  } else if ( dam <=  28 ) {
    vs = "decimate";
    vp = "decimates";
  } else if ( dam <=  32 ) {
    vs = "devastate";
    vp = "devastates";
  } else if ( dam <=  36 ) {
    vs = "maim";
    vp = "maims";
  } else if ( dam <=  40 ) {
    vs = "MUTILATE";
    vp = "MUTILATES";
  } else if ( dam <=  44 ) {
    vs = "DISEMBOWEL";
    vp = "DISEMBOWELS";
  } else if ( dam <=  48 ) {
    vs = "EVISCERATE";
    vp = "EVISCERATES";
  } else if ( dam <=  52 ) {
    vs = "MASSACRE";
    vp = "MASSACRES";
  } else if ( dam <= 100 ) {
    vs = "*** DEMOLISH ***";
    vp = "*** DEMOLISHES ***";
  } else if ( dam <= 150 ) {
    vs = "*** DEVASTATE ***";
    vp = "*** DEVASTATES ***";
  } else if ( dam <= 250 ) {
    vs = "*** OBLITERATE ***";
    vp = "*** OBLITERATES ***";
  } else if ( dam <= 300 ) {
    vs = "=== OBLITERATE ===";
    vp = "=== OBLITERATES ===";
  } else if ( dam <= 500 ) {
    vs = "*** ANNIHILATE ***";
    vp = "*** ANNIHILATES ***";
  } else if ( dam <= 750 ) {
    vs = ">>> ANNIHILATE <<<";
    vp = ">>> ANNIHILATES <<<";
  } else if ( dam <= 1000 ) {
    vs = "<<< ERADICATE >>>";
    vp = "<<< ERADICATES >>>";
  } else {
    vs = "&Xdo &rUNSPEAKABLE&X things to";
    vp = "&Xdoes &rUNSPEAKABLE&X things to";
  }

  punct = ( dam <= 24 ) ? '.' : '!';

  if ( dt == TYPE_HIT ) {
    sprintf( buf1, "You &r%s&X $N%c (%d)", vs, punct, dam );
    sprintf( buf2, "$n &G%s&X you%c (%d)", vp, punct, dam );
    sprintf( buf3, "$n &z%s&X $N%c (%d)", vp, punct, dam );
    sprintf( buf4, "You &G%s&X yourself%c (%d)", vs, punct, dam );
    sprintf( buf5, "$n &z%s&X $mself%c (%d)", vp, punct, dam );
  } else {
    if ( is_sn( dt ) ) {
      attack = skill_table[ dt ].noun_damage;
    } else if (   dt >= TYPE_HIT
                  && dt  < TYPE_HIT
                  + sizeof( attack_table ) / sizeof( attack_table[ 0 ] ) ) {
      attack = attack_table[ dt - TYPE_HIT ];
    } else {
      sprintf( buf, "Dam_message: bad dt %d caused by %s.", dt,
               ch->name );
      bug( buf, 0 );
      dt     = TYPE_HIT;
      attack = attack_table[ 0 ];
    }

    if ( dt > TYPE_HIT && is_wielding_poisoned( ch ) ) {
      sprintf( buf1, "Your &gpoisoned&X %s &r%s&X $N%c (%d)", attack, vp, punct, dam );
      sprintf( buf2, "$n's &gpoisoned&X %s &G%s&X you%c (%d)", attack, vp, punct, dam );
      sprintf( buf3, "$n's &gpoisoned&X %s &z%s&X $N%c (%d)", attack, vp, punct, dam );
      sprintf( buf4, "Your &gpoisoned&X %s &G%s&X you%c (%d)", attack, vp, punct, dam );
      sprintf( buf5, "$n's &gpoisoned&X %s &z%s&X $n%c (%d)", attack, vp, punct, dam );
    } else {
      sprintf( buf1, "Your %s &r%s&X $N%c (%d)", attack, vp, punct, dam );
      sprintf( buf2, "$n's %s &G%s&X you%c (%d)", attack, vp, punct, dam );
      sprintf( buf3, "$n's %s &z%s&X $N%c (%d)", attack, vp, punct, dam );
      sprintf( buf4, "Your %s &G%s&X you%c (%d)", attack, vp, punct, dam );
      sprintf( buf5, "$n's %s &z%s&X $n%c (%d)", attack, vp, punct, dam );
    }
  }

  if ( victim != ch ) {
    if ( dam != 0 || IS_SET( ch->act, PLR_COMBAT ) ) {
      act( AT_WHITE, buf1, ch, NULL, victim, TO_CHAR );
    }

    if ( dam != 0 || IS_SET( victim->act, PLR_COMBAT ) ) {
      act( AT_WHITE, buf2, ch, NULL, victim, TO_VICT );
    }

    act( AT_GREY, buf3, ch, NULL, victim,
         dam == 0 ? TO_COMBAT : TO_NOTVICT );
  } else {
    if ( dam != 0 || IS_SET( ch->act, PLR_COMBAT ) ) {
      act( AT_WHITE, buf4, ch, NULL, victim, TO_CHAR );
    }

    act( AT_GREY, buf5, ch, NULL, victim,
         dam == 0 ? TO_COMBAT : TO_NOTVICT );
  }

  return;
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA * ch, CHAR_DATA * victim ) {
  OBJ_DATA * obj;

  if ( !( obj = get_eq_char( victim, WEAR_WIELD ) ) ) {
    if ( !( obj = get_eq_char( victim, WEAR_WIELD_2 ) ) ) {
      return;
    }
  }

  if ( !get_eq_char( ch, WEAR_WIELD ) && number_bits( 1 ) == 0 ) {
    if ( !get_eq_char( ch, WEAR_WIELD_2 ) && number_bits( 1 ) == 0 ) {
      return;
    }
  }

  act( AT_YELLOW, "You disarm $N!", ch, NULL, victim, TO_CHAR );
  act( AT_YELLOW, "$n DISARMS you!", ch, NULL, victim, TO_VICT );
  act( AT_GREY, "$n DISARMS $N!", ch, NULL, victim, TO_NOTVICT );

  obj_from_char( obj );

  if ( IS_NPC( victim ) ) {
    obj_to_char( obj, victim );
  } else {
    obj_to_room( obj, victim->in_room );
  }

  return;
}

/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip( CHAR_DATA * ch, CHAR_DATA * victim ) {
  if ( ( IS_AFFECTED( victim, AFF_FLYING ) ) ) {
    return;
  }

  if ( !IS_STUNNED( victim, STUN_COMMAND ) && !IS_STUNNED( ch, STUN_TO_STUN ) ) {
    act( AT_CYAN, "You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR );
    act( AT_CYAN, "$n trips you and you go down!", ch, NULL, victim, TO_VICT );
    act( AT_GREY, "$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT );

    WAIT_STATE( ch, PULSE_VIOLENCE );
    STUN_CHAR( victim, 2, STUN_COMMAND );
    STUN_CHAR( ch, 3, STUN_TO_STUN );
    victim->position = POS_RESTING;
  }

  return;
}

void do_kill( CHAR_DATA * ch, char * argument ) {
  CHAR_DATA * victim;
  char        arg[ MAX_INPUT_LENGTH ];

  one_argument( argument, arg );

  if ( !( victim = get_char_room( ch, arg ) ) ) {
    send_to_char( AT_WHITE, "That person is not here.\n\r", ch );
    return;
  }

  if ( is_safe( ch, victim ) ) {
    send_to_char( AT_WHITE, "You cannot.\n\r", ch );
    return;
  }

  if ( arg[ 0 ] == '\0' ) {
    send_to_char( AT_WHITE, "Kill whom?\n\r", ch );
    return;
  }

  if ( !( victim = get_char_room( ch, arg ) ) ) {
    send_to_char( AT_WHITE, "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_AFFECTED( victim, AFF_PEACE ) ) {
    send_to_char( AT_WHITE, "A wave of peace overcomes you.\n\r", ch );
    return;
  }

  if ( IS_AFFECTED( ch, AFF_PEACE ) ) {
    affect_strip( ch, skill_lookup( "aura of peace" ) );
    REMOVE_BIT( ch->affected_by, AFF_PEACE );
  }

  if ( !IS_NPC( victim ) ) {
    if (   !IS_SET( victim->act, PLR_KILLER ) && !IS_SET( victim->act, PLR_THIEF ) ) {
      send_to_char( AT_WHITE, "You must MURDER a player.\n\r", ch );
      return;
    }
  } else {
    if ( IS_AFFECTED( victim, AFF_CHARM ) && victim->master ) {
      send_to_char( AT_WHITE, "You must MURDER a charmed creature.\n\r", ch );
      return;
    }
  }

  if ( victim == ch ) {
    send_to_char( AT_RED, "You hit yourself.  Stupid!\n\r", ch );
    multi_hit( ch, ch, TYPE_UNDEFINED );
    return;
  }

  if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) {
    act( AT_BLUE, "$N is your beloved master!", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( ch->position == POS_FIGHTING ) {
    send_to_char( C_DEFAULT, "You do the best you can!\n\r", ch );
    return;
  }

  WAIT_STATE( ch, PULSE_VIOLENCE );

  check_killer( ch, victim );

  multi_hit( ch, victim, TYPE_UNDEFINED );
  return;
}

void do_murde( CHAR_DATA * ch, char * argument ) {
  send_to_char( C_DEFAULT, "If you want to MURDER, spell it out.\n\r", ch );
  return;
}

void do_murder( CHAR_DATA * ch, char * argument ) {
  CHAR_DATA * victim;
  char        buf[ MAX_STRING_LENGTH ];
  char        arg[ MAX_INPUT_LENGTH  ];

  one_argument( argument, arg );

  if ( arg[ 0 ] == '\0' ) {
    send_to_char( C_DEFAULT, "Murder whom?\n\r", ch );
    return;
  }

  if ( !( victim = get_char_room( ch, arg ) ) ) {
    send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_AFFECTED( victim, AFF_PEACE ) ) {
    send_to_char( AT_WHITE, "A wave of peace overcomes you.\n\r", ch );
    return;
  }

  if ( IS_AFFECTED( ch, AFF_PEACE ) ) {
    affect_strip( ch, skill_lookup( "aura of peace" ) );
    REMOVE_BIT( ch->affected_by, AFF_PEACE );
  }

  if ( victim == ch ) {
    send_to_char( C_DEFAULT, "Suicide is a mortal sin.\n\r", ch );
    return;
  }

  if ( is_safe( ch, victim ) ) {
    return;
  }

  if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) {
    act( C_DEFAULT, "$N is your beloved master!", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( !IS_SET( ch->act, PLR_PKILLER ) && !IS_NPC( victim ) ) {
    send_to_char( C_DEFAULT, "You must be a Pkiller to kill another mortal!\n\r", ch );
    return;
  }

  if ( !IS_SET( victim->act, PLR_PKILLER ) && !IS_NPC( victim ) ) {
    send_to_char( C_DEFAULT, "You can only pkill other Pkillers.\n\r", ch );
    return;
  }

  if ( ch->position == POS_FIGHTING ) {
    send_to_char( C_DEFAULT, "You do the best you can!\n\r", ch );
    return;
  }

  WAIT_STATE( ch, PULSE_VIOLENCE );

  if ( !IS_NPC( victim ) ) {
    sprintf( buf, "Help!  I am being attacked by %s!", ch->name );
    do_yell( victim, buf );
  }

  multi_hit( ch, victim, TYPE_UNDEFINED );
  return;
}

void do_backstab( CHAR_DATA * ch, char * argument ) {
  OBJ_DATA  * obj;
  CHAR_DATA * victim;
  char        arg[ MAX_INPUT_LENGTH ];

  if ( !IS_NPC( ch )
       && !can_use_skpell( ch, gsn_backstab ) ) {
    send_to_char( C_DEFAULT,
                  "You better leave the assassin trade to rogues.\n\r", ch );
    return;
  }

  one_argument( argument, arg );

  if ( arg[ 0 ] == '\0' ) {
    send_to_char( C_DEFAULT, "Backstab whom?\n\r", ch );
    return;
  }

  if ( !( victim = get_char_room( ch, arg ) ) ) {
    send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
    return;
  }

  if ( victim == ch ) {
    send_to_char( C_DEFAULT, "How can you sneak up on yourself?\n\r", ch );
    return;
  }

  if ( is_safe( ch, victim ) ) {
    return;
  }

  if ( !( obj = get_eq_char( ch, WEAR_WIELD ) )
       || ( obj->value[ 3 ] != 11 && obj->value[ 3 ] != 2 ) ) {
    send_to_char( C_DEFAULT, "You need to wield a piercing or stabbing weapon.\n\r", ch );
    return;
  }

  if ( victim->fighting ) {
    send_to_char( C_DEFAULT, "You can't backstab a fighting person.\n\r", ch );
    return;
  }

  if ( victim->hit < MAX_HIT( victim ) * 0.4 ) {
    act( C_DEFAULT, "$N is hurt and suspicious ... you can't sneak up.",
         ch, NULL, victim, TO_CHAR );
    return;
  }

  check_killer( ch, victim );
  WAIT_STATE( ch, skill_table[ gsn_backstab ].beats );

  if ( !IS_AWAKE( victim ) || IS_NPC( ch ) || number_percent() < ch->pcdata->learned[ gsn_backstab ] ) {
    multi_hit( ch, victim, gsn_backstab );
    update_skpell( ch, gsn_backstab );
  } else {
    damage( ch, victim, 0, gsn_backstab );
  }

  return;
}

void do_flee( CHAR_DATA * ch, char * argument ) {
  CHAR_DATA       * victim;
  ROOM_INDEX_DATA * was_in;
  ROOM_INDEX_DATA * now_in;
  int               attempt;

  if ( IS_AFFECTED( ch, AFF_ANTI_FLEE ) ) {
    send_to_char( AT_RED, "You cannot!\n\r", ch );
    return;
  }

  if ( !( victim = ch->fighting ) ) {
    if ( ch->position == POS_FIGHTING ) {
      ch->position = POS_STANDING;
    }

    send_to_char( C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if ( IS_SET( ch->in_room->room_flags, ROOM_NO_FLEE ) ) {
    send_to_char( C_DEFAULT, "You failed!  You lose 10 exps.\n\r", ch );
    gain_exp( ch, -10 );

    return;
  }

  was_in = ch->in_room;

  for ( attempt = 0; attempt < 6; attempt++ ) {
    EXIT_DATA * pexit;
    int         door;

    door = number_door();

    if ( ( pexit = was_in->exit[ door ] ) == 0
         ||   !pexit->to_room
         ||   IS_SET( pexit->exit_info, EX_CLOSED )
         || ( IS_NPC( ch )
              && ( IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB )
                   || ( IS_SET( ch->act, ACT_STAY_AREA )
                        && pexit->to_room->area != ch->in_room->area ) ) ) ) {
      continue;
    }

    move_char( ch, door, FALSE );

    if ( ( now_in = ch->in_room ) == was_in ) {
      continue;
    }

    ch->in_room = was_in;
    act( C_DEFAULT, "$n has fled!", ch, NULL, NULL, TO_ROOM );

    ch->in_room = now_in;

    if ( !IS_NPC( ch ) ) {
      send_to_char( C_DEFAULT, "You flee from combat!  You lose 25 exps.\n\r", ch );
      gain_exp( ch, -25 );
    }

    if ( ch->fighting && IS_NPC( ch->fighting ) ) {
      if ( IS_SET( ch->fighting->act, ACT_TRACK ) ) {
        ch->fighting->hunting = ch;
      }
    }

    stop_fighting( ch, TRUE );
    return;
  }

  send_to_char( C_DEFAULT, "You failed!  You lose 10 exps.\n\r", ch );
  gain_exp( ch, -10 );

  return;
}

void do_rescue( CHAR_DATA * ch, char * argument ) {
  CHAR_DATA * victim;
  CHAR_DATA * fch;
  char        arg[ MAX_INPUT_LENGTH ];

  if ( !IS_NPC( ch )
       && !can_use_skpell( ch, gsn_rescue ) ) {
    send_to_char( C_DEFAULT,
                  "You'd better leave the heroic acts to warriors.\n\r", ch );
    return;
  }

  one_argument( argument, arg );

  if ( arg[ 0 ] == '\0' ) {
    send_to_char( C_DEFAULT, "Rescue whom?\n\r", ch );
    return;
  }

  if ( !( victim = get_char_room( ch, arg ) ) ) {
    send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
    return;
  }

  if ( victim == ch ) {
    send_to_char( C_DEFAULT, "What about fleeing instead?\n\r", ch );
    return;
  }

  if ( !IS_NPC( ch ) && IS_NPC( victim ) ) {
    send_to_char( C_DEFAULT, "Doesn't need your help!\n\r", ch );
    return;
  }

  if ( ch->fighting == victim ) {
    send_to_char( C_DEFAULT, "Too late.\n\r", ch );
    return;
  }

  if ( !( fch = victim->fighting ) ) {
    send_to_char( C_DEFAULT, "That person is not fighting right now.\n\r", ch );
    return;
  }

  if ( !is_same_group( ch, victim ) ) {
    send_to_char( C_DEFAULT, "Why would you want to?\n\r", ch );
    return;
  }

  if ( !check_blind( ch ) ) {
    return;
  }

  WAIT_STATE( ch, skill_table[ gsn_rescue ].beats );

  if ( !IS_NPC( ch ) && number_percent() > ch->pcdata->learned[ gsn_rescue ] ) {
    send_to_char( C_DEFAULT, "You fail the rescue.\n\r", ch );
    return;
  }

  update_skpell( ch, gsn_rescue );

  act( C_DEFAULT, "You rescue $N!", ch, NULL, victim, TO_CHAR );
  act( C_DEFAULT, "$n rescues you!", ch, NULL, victim, TO_VICT );
  act( C_DEFAULT, "$n rescues $N!", ch, NULL, victim, TO_NOTVICT );

  stop_fighting( fch, FALSE );

  set_fighting( fch, ch );

  return;
}

void do_gouge( CHAR_DATA * ch, char * argument ) {
  CHAR_DATA * victim;
  char        arg[ MAX_INPUT_LENGTH ];

  if ( !IS_NPC( ch )
       && !can_use_skpell( ch, gsn_gouge ) ) {
    send_to_char( C_DEFAULT,
                  "You'd better leave the dirty tricks to rogues.\n\r", ch );
    return;
  }

  if ( !ch->fighting ) {
    send_to_char( C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if ( !check_blind( ch ) ) {
    return;
  }

  one_argument( argument, arg );

  victim = ch->fighting;

  if ( arg[ 0 ] != '\0' ) {
    if ( !( victim = get_char_room( ch, arg ) ) ) {
      send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
      return;
    }
  }

  WAIT_STATE( ch, skill_table[ gsn_gouge ].beats );

  if ( IS_NPC( ch ) || number_percent() < ch->pcdata->learned[ gsn_gouge ] ) {
    update_skpell( ch, gsn_gouge );
    damage( ch, victim, number_range( 100, ch->level * 5 ), gsn_gouge );

    if ( number_percent() < 10 ) {
      AFFECT_DATA af;

      af.type      = gsn_blindness;
      af.duration  = 5;
      af.location  = APPLY_HITROLL;
      af.modifier  = -10;
      af.bitvector = AFF_BLIND;
      affect_join( victim, &af );
      act( AT_GREY, "$N is blinded!", ch, NULL, victim, TO_CHAR );
    }

    update_pos( victim );
  } else {
    damage( ch, victim, 0, gsn_gouge );
  }

  return;
}

void do_kick( CHAR_DATA * ch, char * argument ) {
  CHAR_DATA * victim;
  char        arg[ MAX_INPUT_LENGTH ];

  if ( !IS_NPC( ch )
       && !can_use_skpell( ch, gsn_kick ) ) {
    send_to_char( C_DEFAULT,
                  "You'd better leave the martial arts to warriors.\n\r", ch );
    return;
  }

  if ( !ch->fighting ) {
    send_to_char( C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if ( !check_blind( ch ) ) {
    return;
  }

  one_argument( argument, arg );

  victim = ch->fighting;

  if ( arg[ 0 ] != '\0' ) {
    if ( !( victim = get_char_room( ch, arg ) ) ) {
      send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
      return;
    }
  }

  WAIT_STATE( ch, skill_table[ gsn_kick ].beats );

  if ( IS_NPC( ch ) || number_percent() < ch->pcdata->learned[ gsn_kick ] ) {
    damage( ch, victim, number_range( 100, ch->level * 3 ), gsn_kick );
    update_skpell( ch, gsn_kick );
  } else {
    damage( ch, victim, 0, gsn_kick );
  }

  return;
}

void do_punch( CHAR_DATA * ch, char * argument ) {
  CHAR_DATA * victim;
  char        arg[ MAX_INPUT_LENGTH ];
  int         chance = 0;

  if ( !IS_NPC( ch )
       && !can_use_skpell( ch, gsn_punch ) ) {
    send_to_char( C_DEFAULT,
                  "You'd better leave the martial arts to fighters.\n\r", ch );
    return;
  }

  if ( !ch->fighting ) {
    send_to_char( C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if ( !check_blind( ch ) ) {
    return;
  }

  one_argument( argument, arg );

  victim = ch->fighting;

  if ( arg[ 0 ] != '\0' ) {
    if ( !( victim = get_char_room( ch, arg ) ) ) {
      send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
      return;
    }
  }

  WAIT_STATE( ch, skill_table[ gsn_punch ].beats );

  if ( IS_NPC( ch ) || number_percent() < ch->pcdata->learned[ gsn_punch ] ) {
    damage( ch, victim, number_range( 100, ch->level * 3 ), gsn_punch );
    update_skpell( ch, gsn_punch );
  } else {
    damage( ch, victim, 0, gsn_punch );
  }

  if ( !victim || victim->position == POS_DEAD || !victim->in_room
       || victim->in_room != ch->in_room ) {
    return;
  }

  if ( !IS_NPC( ch ) ) {
    chance = ch->pcdata->mod_str + ( ch->level - victim->level ) + ch->pcdata->perm_str;
  }

  if ( ( number_percent() < chance ) || IS_NPC( ch ) ) {
    act( AT_RED, "You hear a crunch as you connect with $N's head.", ch, NULL, victim, TO_CHAR );
    act( AT_RED, "$n's punch connects firmly with your head!", ch, NULL, victim, TO_VICT );
    act( C_DEFAULT, "$n's punch hit's home!", ch, NULL, victim, TO_ROOM );
    damage( ch, victim, number_range( 1, ch->level ), gsn_punch );
  }

  return;
}

void do_disarm( CHAR_DATA * ch, char * argument ) {
  OBJ_DATA  * obj;
  CHAR_DATA * victim;
  char        arg[ MAX_INPUT_LENGTH ];
  int         percent;

  if ( !IS_NPC( ch )
       && !can_use_skpell( ch, gsn_disarm ) ) {
    send_to_char( C_DEFAULT, "You don't know how to disarm opponents.\n\r", ch );
    return;
  }

  if ( ( !get_eq_char( ch, WEAR_WIELD ) )
       && ( !get_eq_char( ch, WEAR_WIELD_2 ) ) ) {
    send_to_char( C_DEFAULT, "You must wield a weapon to disarm.\n\r", ch );
    return;
  }

  if ( !ch->fighting ) {
    send_to_char( C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if ( !check_blind( ch ) ) {
    return;
  }

  one_argument( argument, arg );

  victim = ch->fighting;

  if ( arg[ 0 ] != '\0' ) {
    if ( !( victim = get_char_room( ch, arg ) ) ) {
      send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
      return;
    }
  }

  if ( victim->fighting != ch && ch->fighting != victim ) {
    act( C_DEFAULT, "$E is not fighting you!", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( !( obj = get_eq_char( victim, WEAR_WIELD ) ) ) {
    if ( !( obj = get_eq_char( victim, WEAR_WIELD_2 ) ) ) {
      send_to_char( C_DEFAULT, "Your opponent is not wielding a weapon.\n\r", ch );
      return;
    }
  }

  if ( number_percent() < victim->antidisarm ) {
    send_to_char( C_DEFAULT, "You failed.\n\r", ch );
    return;
  }

  WAIT_STATE( ch, skill_table[ gsn_disarm ].beats );
  percent = number_percent() + victim->level - ch->level;

  if ( ( IS_NPC( ch ) && percent < 20 ) || ( ( !IS_NPC( ch ) ) &&
                                             ( percent < ch->pcdata->learned[ gsn_disarm ] * 2 / 3 ) ) ) {
    disarm( ch, victim );
    update_skpell( ch, gsn_disarm );
  } else {
    send_to_char( C_DEFAULT, "You failed.\n\r", ch );
  }

  return;
}

void do_sla( CHAR_DATA * ch, char * argument ) {
  send_to_char( C_DEFAULT, "If you want to SLAY, spell it out.\n\r", ch );
  return;
}

void do_slay( CHAR_DATA * ch, char * argument ) {
  CHAR_DATA * victim;
  char        arg[ MAX_INPUT_LENGTH ];
  char        buf[ MAX_STRING_LENGTH ];

  one_argument( argument, arg );

  if ( arg[ 0 ] == '\0' ) {
    send_to_char( C_DEFAULT, "Slay whom?\n\r", ch );
    return;
  }

  if ( !( victim = get_char_room( ch, arg ) ) ) {
    send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
    return;
  }

  if ( ch == victim ) {
    send_to_char(C_DEFAULT, "Suicide is a mortal sin.\n\r", ch );
    return;
  }

  if ( ( !IS_NPC( victim ) && victim->level >= ch->level && victim != ch ) ||
       ( IS_NPC( ch ) && !IS_NPC( victim ) ) ) {
    send_to_char( C_DEFAULT, "You failed.\n\r", ch );
    return;
  }

  sprintf( buf, "You %s.", ( ch->pcdata && ch->pcdata->slayusee[ 0 ] != '\0' ) ? ch->pcdata->slayusee : "slay $N in cold blood." );
  act( AT_RED, buf, ch, NULL, victim, TO_CHAR );

  sprintf( buf, "%s %s.", ch->name, ( ch->pcdata && ch->pcdata->slayvict[ 0 ] != '\0' ) ? ch->pcdata->slayvict : "slays you in cold blood!" );
  act( AT_RED, buf, ch, NULL, victim, TO_VICT );

  sprintf( buf, "%s %s.", ch->name, ( ch->pcdata && ch->pcdata->slayroom[ 0 ] != '\0' ) ? ch->pcdata->slayroom : "slays $N in cold blood!" );
  act( AT_RED, buf, ch, NULL, victim, TO_NOTVICT );

  sprintf( log_buf, "%s slays %s at %d.\n\r", ch->name, victim->name, victim->in_room->vnum );
  log_string( log_buf, CHANNEL_LOG, ch->level - 1 );

  if ( !IS_NPC( victim ) ) {
    wiznet( log_buf, ch, NULL, WIZ_DEATHS, 0, 0 );
  }

  raw_kill( ch, victim );
  return;
}

int per_type( CHAR_DATA * ch, OBJ_DATA * Obj ) {
  switch ( Obj->item_type ) {
    case ITEM_WEAPON:
      return number_range( 5, ch->level + 5 );
    case ITEM_STAFF:
      return number_range( 3, ch->level + 3 );
    case ITEM_WAND:
      return number_range( 2, ch->level + 2 );
    default:
      return number_range( 1, ch->level );
  }

  return number_range( 1, ch->level );
}

void do_throw( CHAR_DATA * ch, char * argument ) {
  char              arg1[ MAX_INPUT_LENGTH ];
  char              arg2[ MAX_INPUT_LENGTH ];
  char              arg3[ MAX_INPUT_LENGTH ];
  char              buf[ MAX_STRING_LENGTH ];
  CHAR_DATA       * victim;
  ROOM_INDEX_DATA * to_room;
  ROOM_INDEX_DATA * in_room;
  OBJ_DATA        * Obj;
  EXIT_DATA       * pexit;
  int               dir      = 0;
  int               dist     = 0;
  int               MAX_DIST = 2;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if ( arg1[ 0 ] == '\0' ) {
    send_to_char( C_DEFAULT, "Throw what item?\n\r", ch );
    return;
  }

  if ( ( Obj = get_obj_wear( ch, arg1 ) ) == NULL ) {
    send_to_char( C_DEFAULT,
                  "You are not wearing, wielding, or holding that item.\n\r",
                  ch );
    return;
  }

  if ( Obj->wear_loc != WEAR_WIELD && Obj->wear_loc != WEAR_WIELD_2 &&
       Obj->wear_loc != WEAR_HOLD ) {
    send_to_char( C_DEFAULT,
                  "You are not wielding or holding that item.\n\r", ch );
    return;
  }

  if ( IS_SET( Obj->extra_flags, ITEM_NOREMOVE ) || IS_SET( Obj->extra_flags,
                                                            ITEM_NODROP ) ) {
    send_to_char( C_DEFAULT, "You can't let go of it!\n\r", ch );
    return;
  }

  in_room = ch->in_room;
  to_room = ch->in_room;

  if ( ( victim = ch->fighting ) == NULL ) {
    if ( arg2[ 0 ] == '\0' ) {
      send_to_char( C_DEFAULT, "Throw it at who?\n\r", ch );
      return;
    }

    if ( arg3[ 0 ] == '\0' ) {
      if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
        send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
        return;
      }
    } else {
      if ( get_curr_str( ch ) >= 20 ) {
        MAX_DIST = 3;

        if ( get_curr_str( ch ) == 25 ) {
          MAX_DIST = 4;
        }
      }

      for ( dir = 0; dir < MAX_DIR; dir++ ) {
        if ( arg2[ 0 ] == direction_table[ dir ].name[ 0 ] && !str_prefix( arg2,
                                                                           direction_table[ dir ].name ) ) {
          break;
        }
      }

      if ( dir == MAX_DIR ) {
        send_to_char( C_DEFAULT, "Throw in which direction?\n\r", ch );
        return;
      }

      if ( ( pexit = to_room->exit[ dir ] ) == NULL ||
           ( to_room = pexit->to_room ) == NULL ) {
        send_to_char( C_DEFAULT, "You cannot throw in that direction.\n\r",
                      ch );
        return;
      }

      if ( IS_SET( pexit->exit_info, EX_CLOSED ) ) {
        send_to_char( C_DEFAULT, "You cannot throw through a door.\n\r", ch );
        return;
      }

      for ( dist = 1; dist <= MAX_DIST; dist++ ) {
        char_from_room( ch );
        char_to_room( ch, to_room );

        if ( ( victim = get_char_room( ch, arg3 ) ) != NULL ) {
          break;
        }

        if ( ( pexit = to_room->exit[ dir ] ) == NULL ||
             ( to_room = pexit->to_room ) == NULL ||
             IS_SET( pexit->exit_info, EX_CLOSED ) ) {
          sprintf( buf, "A $p flys in from $T and hits the %s wall.",
                   direction_table[ dir ].name );
          act( AT_WHITE, buf, ch, Obj, direction_table[ direction_table[ dir ].reverse ].noun, TO_ROOM );
          sprintf( buf, "You throw your $p %d room%s $T, where it hits a wall.",
                   dist, dist > 1 ? "s" : "" );
          act( AT_WHITE, buf, ch, Obj, direction_table[ dir ].name, TO_CHAR );
          char_from_room( ch );
          char_to_room( ch, in_room );
          oprog_throw_trigger( Obj, ch );
          unequip_char( ch, Obj );
          obj_from_char( Obj );
          obj_to_room( Obj, to_room );
          return;
        }
      }

      if ( victim == NULL ) {
        act( AT_WHITE,
             "A $p flies in from $T and falls harmlessly to the ground.",
             ch, Obj, direction_table[ direction_table[ dir ].reverse ].noun, TO_ROOM );
        sprintf( buf,
                 "Your $p falls harmlessly to the ground %d room%s $T of here.",
                 dist, dist > 1 ? "s" : "" );
        act( AT_WHITE, buf, ch, Obj, direction_table[ dir ].name, TO_CHAR );
        char_from_room( ch );
        char_to_room( ch, in_room );
        oprog_throw_trigger( Obj, ch );
        unequip_char( ch, Obj );
        obj_from_char( Obj );
        obj_to_room( Obj, to_room );
        return;
      }
    }

    if ( dist > 0 ) {
      char_from_room( ch );
      char_to_room( ch, in_room );
      act( AT_WHITE, "A $p flys in from $T and hits $n!", victim, Obj,
           direction_table[ direction_table[ dir ].reverse ].noun, TO_NOTVICT );
      act( AT_WHITE, "A $p flys in from $T and hits you!", victim, Obj,
           direction_table[ direction_table[ dir ].reverse ].noun, TO_CHAR );
      sprintf( buf, "Your $p flew %d rooms %s and hit $N!", dist,
               direction_table[ dir ].name );
      act( AT_WHITE, buf, ch, Obj, victim, TO_CHAR );
      oprog_throw_trigger( Obj, ch );
      unequip_char( ch, Obj );
      obj_from_char( Obj );
      obj_to_room( Obj, to_room );
      damage( ch, victim, per_type( ch, Obj ), gsn_throw );

      update_skpell( ch, gsn_throw );    /* Throw not given to any class though */

      if ( IS_NPC( victim ) ) {
        if ( victim->level > 3 ) {
          victim->hunting = ch;
        }
      }

      return;
    }
  }

  unequip_char( ch, Obj );
  obj_from_char( Obj );
  obj_to_room( Obj, to_room );
  act( AT_WHITE, "$n threw a $p at $N!", ch, Obj, victim, TO_ROOM );
  act( AT_WHITE, "You throw your $p at $N.", ch, Obj, victim, TO_CHAR );
  oprog_throw_trigger( Obj, ch );
  damage( ch, victim, per_type( ch, Obj ), gsn_throw );
  multi_hit( victim, ch, TYPE_UNDEFINED );
  return;
}

void do_stun( CHAR_DATA * ch, char * argument ) {
  CHAR_DATA * victim;

  if ( !IS_NPC( ch ) && !can_use_skpell( ch, gsn_stun ) ) {
    send_to_char( C_DEFAULT, "You failed.\n\r", ch );
    return;
  }

  if ( !( victim = ch->fighting ) ) {
    send_to_char( C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if ( victim->position == POS_STUNNED || IS_STUNNED( ch, STUN_TO_STUN ) ) {
    return;
  }

  if ( ( IS_NPC( ch ) || number_percent() < ch->pcdata->learned[ gsn_stun ] ) &&
       number_percent() < ( ch->level * 75 ) / victim->level ) {
    STUN_CHAR( ch, 10, STUN_TO_STUN );
    STUN_CHAR( victim, 3, STUN_TOTAL );
    victim->position = POS_STUNNED;
    act( AT_WHITE, "You stun $N!", ch, NULL, victim, TO_CHAR );
    act( AT_WHITE, "$n stuns $N!", ch, NULL, victim, TO_NOTVICT );
    act( AT_WHITE, "$n stuns you!", ch, NULL, victim, TO_VICT );
    update_skpell( ch, gsn_stun );
    return;
  }

  send_to_char( C_DEFAULT, "You failed.\n\r", ch );
  return;
}

void do_rage( CHAR_DATA * ch, char * argument ) {
  OBJ_DATA  * wield;
  OBJ_DATA  * wield2;
  char        buf[ MAX_INPUT_LENGTH ];
  AFFECT_DATA af;

  if ( IS_AFFECTED2( ch, AFF_RAGE ) ) {
    return;
  }

  if ( !can_use_skpell( ch, gsn_rage ) || (!IS_NPC( ch ) && ch->pcdata->learned[ gsn_rage ] < number_percent()) ) {
    send_to_char( C_DEFAULT, "You cannot summon enough anger.\n\r", ch );
    return;
  }

  wield  = get_eq_char( ch, WEAR_WIELD );
  wield2 = get_eq_char( ch, WEAR_WIELD_2 );

  if ( wield || wield2 ) {
    sprintf( buf, "You become enraged and toss your weapon%s to the ground.\n\r", ( wield && wield2 ) ? "s" : "" );
    send_to_char( AT_RED, buf, ch );
    sprintf( buf, "$n tosses his weapon%s to the ground in a fit of rage.", ( wield && wield2 ) ? "s" : "" );
    act( C_DEFAULT, buf, ch, NULL, NULL, TO_ROOM );

    if ( wield ) {
      obj_from_char( wield );
      obj_to_room( wield, ch->in_room );
    }

    if ( wield2 ) {
      obj_from_char( wield2 );
      obj_to_room( wield2, ch->in_room );
    }
  } else {
    send_to_char( AT_RED, "You become enraged and surge with power.\n\r", ch );
  }

  update_skpell( ch, gsn_rage );

  af.type      = gsn_rage;
  af.level     = ch->level;
  af.duration  = ch->level * 0.4;
  af.location  = APPLY_STR;
  af.modifier  = 5;
  af.bitvector = AFF_RAGE;
  affect_to_char2( ch, &af );

  af.location = APPLY_HITROLL;
  af.modifier = ch->level;
  affect_to_char2( ch, &af );

  af.location = APPLY_DAMROLL;
  af.modifier = ch->level;
  affect_to_char2( ch, &af );

  return;
}

void do_trip( CHAR_DATA * ch, char * argument ) {
  CHAR_DATA * victim;

  if ( IS_NPC( ch ) ) {
    return;
  }

  if ( !can_use_skpell( ch, gsn_trip ) ) {
    send_to_char( AT_GREY, "Huh?\n\r", ch );
    return;
  }

  if ( !( victim = ch->fighting ) ) {
    send_to_char( AT_GREY, "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if ( ch->pcdata->learned[ gsn_trip ] > number_percent() ) {
    trip( ch, victim );
    update_skpell( ch, gsn_trip );
  }

  return;

}

void death_xp_loss( CHAR_DATA * victim ) {
  int base_xp, xp_lastlvl, xp_loss, classes, mod;
  classes = number_classes( victim );

  if ( victim->level < LEVEL_HERO ) {
    xp_lastlvl = classes == 1 ? 1000 * victim->level
                 : classes * 2000 * victim->level;
    xp_loss = ( xp_lastlvl - victim->exp ) / 2;

    if ( victim->exp > xp_lastlvl ) {
      gain_exp( victim, xp_loss );
    }
  } else if ( victim->level < L_CHAMP3 ) {
    mod        = 1;
    base_xp    = classes == 1 ? 100000 : 200000;
    xp_lastlvl = base_xp * classes;

    switch ( victim->level ) {
      case LEVEL_HERO:
        mod = 1;
      case L_CHAMP1:
        mod = 4;
      case L_CHAMP2:
        mod = 10;
    }

    xp_lastlvl = xp_lastlvl * mod;

    if ( victim->exp > xp_lastlvl ) {
      xp_loss = ( xp_lastlvl - victim->exp ) / 2;
      xp_loss = UMAX( -10000 * classes, xp_loss );
      gain_exp( victim, xp_loss );
    }
  }

  return;
}

void do_lure( CHAR_DATA * ch, char * argument ) {
  char        dt;
  CHAR_DATA * victim;
  char        arg[ MAX_INPUT_LENGTH ];

  if ( !IS_NPC( ch ) && !can_use_skpell( ch, gsn_lure ) ) {
    send_to_char( C_DEFAULT, "You cannot lure.\n\r", ch );
    return;
  }

  if ( !ch->fighting ) {
    send_to_char( C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if ( !check_blind( ch ) ) {
    return;
  }

  one_argument( argument, arg );
  victim = ch->fighting;

  if ( arg[ 0 ] != '\0' && !( victim = get_char_room( ch, arg ) ) ) {
    send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
    return;
  }

  dt = TYPE_UNDEFINED;

  WAIT_STATE( ch, skill_table[ gsn_lure ].beats );

  if ( IS_NPC( ch ) || number_percent() < ch->pcdata->learned[ gsn_lure ] ) {
    act( AT_WHITE, "$n lures $N into a vulnerable position.", ch, NULL, victim, TO_ROOM );
    send_to_char( AT_WHITE, "Your opponent has lured you into a vulnerable position.\n\r", victim );
    send_to_char( AT_WHITE, "You lure your opponent into a vulnerable position.\n\r", ch );

    one_hit( ch, victim, dt, FALSE );

    /* put more reasons to attack here */

    one_hit( ch, victim, dt, TRUE );
  }

  update_skpell( ch, gsn_lure );
  return;
}

void do_flip( CHAR_DATA * ch, char * argument ) {
  char              arg1[ MAX_INPUT_LENGTH ];
  char              arg2[ MAX_INPUT_LENGTH ];
  EXIT_DATA       * pexit;
  CHAR_DATA       * victim;
  ROOM_INDEX_DATA * from_room;
  int               door;
  char              buf1[ 256 ], buf2[ 256 ], buf3[ 256 ];

  argument = one_argument( argument, arg1 );
  one_argument( argument, arg2 );

  if ( arg1[ 0 ] == '\0' ) {
    send_to_char( AT_BLUE, "Flip who what where?", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg1 ) ) == NULL ) {
    send_to_char( AT_BLUE, "They aren't here.\n\r", ch );
    return;
  }

  if ( victim->position != POS_FIGHTING ) {
    send_to_char( AT_BLUE, "You aren't fighting anybody.\n\r", ch );
    return;
  }

  WAIT_STATE( ch, skill_table[ gsn_flip ].beats );

  if ( IS_NPC( ch ) || number_percent() < ch->pcdata->learned[ gsn_flip ] ) {
    damage( ch, victim, number_range( 100, ch->level * 6 ), gsn_flip );
  } else {
    damage( ch, victim, 0, gsn_flip );
  }

  if ( victim->position == POS_STUNNED || IS_STUNNED( ch, STUN_TO_STUN ) ) {
    return;
  }

  if ( ( IS_NPC( ch ) || number_percent() < ch->pcdata->learned[ gsn_flip ] ) &&
       number_percent() < ( ch->level * 100 ) / victim->level ) {
    STUN_CHAR( ch, 10, STUN_TO_STUN );
    STUN_CHAR( victim, 3, STUN_TOTAL );
    victim->position = POS_STUNNED;
    act( AT_WHITE, "You knock the wind out of $N!", ch, NULL, victim, TO_CHAR );
    act( AT_WHITE, "$n knocks the wind out of $N!", ch, NULL, victim, TO_NOTVICT );
    act( AT_WHITE, "$n knocks the wind out of you!", ch, NULL, victim, TO_VICT );
    update_skpell( ch, gsn_flip );

  }

  if ( ( door = get_direction( arg2 ) ) == -1 ) {
    door = number_door();
  }

  if ( ch == victim ) {
    send_to_char( AT_BLUE, "You attempt to flip yourself, oook.\n\r", ch );
    return;
  }

  pexit = ch->in_room->exit[ door ];

  if ( pexit == NULL || IS_SET( pexit->exit_info, EX_CLOSED ) ) {
    act( AT_BLUE, "There is no exit, but you flip $M anyways.", ch, NULL,
         victim, TO_CHAR );
    act( AT_BLUE, "$n flips $N.", ch, NULL, victim, TO_NOTVICT );
    act( AT_BLUE, "$n flips you, ouch.", ch, NULL, victim, TO_VICT );
    return;
  }

  if ( !can_use_skpell( ch, gsn_flip ) ) {
    send_to_char( AT_WHITE, "HEEEYaaaaAA!", ch );
    return;
  }

  if ( pexit->to_room->vnum == ROOM_VNUM_SMITHY ) {
    act( AT_BLUE, "You flip $N, but a force field prevents $S entry.",
         ch, NULL, victim, TO_CHAR );
    act( AT_BLUE, "$n flips $N, but a force field makes $M bounce back.", ch, NULL, victim, TO_NOTVICT );
    act( AT_BLUE, "$n flips you, but you bounce off of a force field.",
         ch, NULL, victim, TO_VICT );
    return;
  }

  if ( number_percent() < ch->pcdata->learned[ gsn_flip ] ) {
    sprintf( buf1, "You flip $N, sending $M %s.", direction_table[ door ].name );
    sprintf( buf2, "$n flips $N, sending $M %s.", direction_table[ door ].name );
    sprintf( buf3, "$n flips you, sending you %s.", direction_table[ door ].name );
    act( AT_BLUE, buf2, ch, NULL, victim, TO_NOTVICT );
    act( AT_BLUE, buf1, ch, NULL, victim, TO_CHAR );
    act( AT_BLUE, buf3, ch, NULL, victim, TO_VICT );
    from_room = victim->in_room;
    eprog_enter_trigger( pexit, victim->in_room, victim );
    char_from_room( victim );
    char_to_room( victim, pexit->to_room );

    act( AT_BLUE, "$n comes flying into the room.", victim, NULL, NULL,
         TO_ROOM );

    if ( ( pexit = pexit->to_room->exit[ direction_table[ door ].reverse ] ) &&
         pexit->to_room == from_room ) {
      eprog_exit_trigger( pexit, victim->in_room, victim );
    } else {
      rprog_enter_trigger( victim->in_room, victim );
    }
  }

  return;
}
