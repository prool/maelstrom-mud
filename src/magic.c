/***************************************************************************
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
 * External functions.
 */
bool is_safe( CHAR_DATA * ch, CHAR_DATA * victim );
void set_fighting( CHAR_DATA * ch, CHAR_DATA * victim );

/*
 * Local functions.
 */
void say_spell( CHAR_DATA * ch, int sn );
int blood_count( OBJ_DATA * list, int amount );
void magic_mob( CHAR_DATA * ch, OBJ_DATA * obj, int vnum );
int slot_lookup( int slot );
int sc_dam( CHAR_DATA * ch, int dam );
/*
 * "Permament sn's": slot loading for objects -- Altrag
 */
int slot_lookup( int slot ) {
  int sn;

  for ( sn = 0; skill_table[ sn ].name[ 0 ] != '\0'; sn++ ) {
    if ( skill_table[ sn ].slot == slot ) {
      return sn;
    }
  }

  bug( "Slot_lookup: no such slot #%d", slot );
  return 0;
}

/*
 * Replacement for MAX_SKILL -- Altrag
 */
bool is_sn( int sn ) {
  int cnt;

  for ( cnt = 0; skill_table[ cnt ].name[ 0 ] != '\0'; cnt++ ) {
    if ( cnt == sn ) {
      return TRUE;
    }
  }

  return FALSE;
}

void magic_mob( CHAR_DATA * ch, OBJ_DATA * obj, int vnum ) {
  CHAR_DATA      * victim;
  CHAR_DATA      * zombie;
  MOB_INDEX_DATA * ZombIndex;
  MOB_INDEX_DATA * pMobIndex;
  char           * name;
  char             buf[ MAX_STRING_LENGTH ];

  if ( !( pMobIndex = get_mob_index( vnum ) ) ) {
    send_to_char( AT_BLUE, "Nothing happens.\n\r", ch );
    return;
  }

  ZombIndex = get_mob_index( MOB_VNUM_ZOMBIE );
  victim    = create_mobile( pMobIndex );
  zombie    = create_mobile( ZombIndex );
  name      = victim->short_descr;
  sprintf( buf, zombie->short_descr, name );
  free_string( zombie->short_descr );
  zombie->short_descr = str_dup( buf );
  sprintf( buf, zombie->long_descr, name );
  free_string( zombie->long_descr );
  zombie->long_descr = str_dup( buf );
  victim->perm_hit  /= 2;
  victim->hit        = MAX_HIT( victim );
  zombie->mod_hit    = victim->mod_hit;
  zombie->perm_hit   = victim->perm_hit;
  zombie->hit        = victim->hit;
  zombie->level      = victim->level;
  SET_BIT( zombie->act, ACT_UNDEAD );
  SET_BIT( zombie->act, ACT_PET );
  SET_BIT( zombie->affected_by, AFF_CHARM );
  char_to_room( zombie, ch->in_room );
  add_follower( zombie, ch );
  update_pos( zombie );
  act( AT_BLUE, "$n passes $s hands over $p, $E slowly rises to serve $S new master.", ch, obj, zombie, TO_ROOM );
  act( AT_BLUE, "You animate $p, it rises to serve you.", ch, obj, NULL, TO_CHAR );
  char_to_room( victim, ch->in_room );
  extract_char( victim, TRUE );
  return;
}

int blood_count( OBJ_DATA * list, int amount ) {
  OBJ_DATA * obj;
  int        count;
  OBJ_DATA * obj_next;

  count = 0;

  for ( obj = list; obj; obj = obj_next ) {
    obj_next = obj->next_content;

    if ( obj->deleted ) {
      continue;
    }

    if ( ( obj->item_type == ITEM_LIQUID ) && ( obj->value[ 2 ] == LIQ_BLOOD ) && ( count != amount ) ) {
      count++;
      extract_obj( obj );
    }
  }

  return count;
}

void update_skpell( CHAR_DATA * ch, int sn ) {
  int  xp = 0;
  char buf[ MAX_STRING_LENGTH ];
  int  adept;

  if ( IS_NPC( ch ) ) {
    return;
  }

  adept = class_table[ prime_class( ch ) ].skill_adept;

  if ( ch->pcdata->learned[ sn ] >= adept ) {
    return;
  }

  ch->pcdata->learned[ sn ] += ( get_curr_wis( ch ) / 5 );

  if ( ch->pcdata->learned[ sn ] > adept ) {
    ch->pcdata->learned[ sn ] = adept;
  }

  xp = ch->level / 5;

  if ( xp < 2 ) {
    xp = 2;
  }

  xp = number_range( xp * 2, xp * 5 );

  sprintf( buf, "You gain %d experience for your success with %s.\n\r", xp, skill_table[ sn ].name );
  send_to_char( C_DEFAULT, buf, ch );

  gain_exp( ch, xp );

  return;
}

int skill_lookup( const char * name ) {
  int sn;

  for ( sn = 0; skill_table[ sn ].name[ 0 ] != '\0'; sn++ ) {
    if ( !skill_table[ sn ].name ) {
      break;
    }

    if ( LOWER( name[ 0 ] ) == LOWER( skill_table[ sn ].name[ 0 ] )
         && !str_prefix( name, skill_table[ sn ].name ) ) {
      return sn;
    }
  }

  return -1;
}

/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA * ch, int sn ) {
  char * pName;
  char   spell[ MAX_STRING_LENGTH ];
  char   room[ MAX_STRING_LENGTH ];
  char   caster[ MAX_STRING_LENGTH ];
  int    iSyl;
  int    length;
  int    spaces = 0;

  struct syl_type {
    char * old;
    char * new;
  };

  static const struct syl_type syl_table [] = {
    { " ",    " "       },
    { "ar",   "abra"    },
    { "au",   "kada"    },
    { "blind", "nose"    },
    { "bur",  "mosa"    },
    { "cu",   "judi"    },
    { "de",   "oculo"   },
    { "en",   "unso"    },
    { "light", "dies"    },
    { "lo",   "hi"      },
    { "mor",  "zak"     },
    { "move", "sido"    },
    { "ness", "lacri"   },
    { "ning", "illa"    },
    { "per",  "duda"    },
    { "ra",   "gru"     },
    { "re",   "candus"  },
    { "son",  "sabru"   },
    { "tect", "infra"   },
    { "tri",  "cula"    },
    { "ven",  "nofo"    },
    { "a",    "a"       }, { "b", "b" }, { "c", "q" }, { "d", "e" },
    { "e",    "z"       }, { "f", "y" }, { "g", "o" }, { "h", "p" },
    { "i",    "u"       }, { "j", "y" }, { "k", "t" }, { "l", "r" },
    { "m",    "w"       }, { "n", "i" }, { "o", "a" }, { "p", "s" },
    { "q",    "d"       }, { "r", "f" }, { "s", "g" }, { "t", "h" },
    { "u",    "j"       }, { "v", "z" }, { "w", "x" }, { "x", "n" },
    { "y",    "l"       }, { "z", "k" },
    { "",     ""        }
  };

  spell[ 0 ] = '\0';

  for ( pName = skill_table[ sn ].name; *pName != '\0'; pName += length ) {
    for ( iSyl = 0; ( length = strlen( syl_table[ iSyl ].old ) ) != 0; iSyl++ ) {
      if ( !str_prefix( syl_table[ iSyl ].old, pName ) ) {
        if ( !strcmp( syl_table[ iSyl ].new, " " ) ) {
          spaces++;
        }

        strcat( spell, syl_table[ iSyl ].new );
        break;
      }
    }

    if ( length == 0 ) {
      length = 1;
    }
  }

  sprintf( room, "$n utters the word%s, '%s'.", spaces == 0 ? "" : "s", spell );
  sprintf( caster, "You utter the word%s, '%s'.\n\r", spaces == 0 ? "" : "s", spell );

  act( AT_GREY, room, ch, NULL, NULL, TO_ROOM );
  send_to_char( AT_GREY, caster, ch );

  return;
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA * victim ) {
  int save;
  int base = 20;
  int savebase;

  if ( IS_NPC( victim ) ) {
    base += 30;
  }

  savebase = 0 - victim->saving_throw / 2;

  if ( victim->race == RACE_DWARF ) {
    savebase += 25 * savebase / 100;
  }

  if ( !IS_NPC( victim ) ) {
    savebase /= 6;
  } else {
    savebase /= 2;
  }

  save = base + ( victim->level - level ) + savebase;
  save = URANGE( 5, save, ( victim->race == RACE_DWARF ) ? 95 : 90 );
  return number_percent() < save;
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char * target_name;

void do_acspell( CHAR_DATA * ch, OBJ_DATA * pObj, char * argument ) {
  void      * vo;
  OBJ_DATA  * obj = NULL;
  CHAR_DATA * victim;
  char        arg1[ MAX_INPUT_LENGTH ];
  char        arg2[ MAX_INPUT_LENGTH ];
  int         sn;

  target_name = one_argument( argument, arg1 );
  one_argument( target_name, arg2 );

  if ( IS_NPC( ch ) ) {
    if ( IS_SET( ch->affected_by, AFF_CHARM ) ) {
      return;
    }
  }

  if ( ( sn = skill_lookup( arg1 ) ) < 0 ) {
    send_to_char( AT_BLUE, "You can't do that.\n\r", ch );
    return;
  }

  /*
   * Locate targets.
   */
  victim = NULL;
  obj    = NULL;
  vo     = NULL;

  switch ( skill_table[ sn ].target ) {
    default:
      bug( "Do_cast: bad target for sn %d.", sn );
      return;

    case TAR_GROUP_OFFENSIVE:
    case TAR_GROUP_DEFENSIVE:
    case TAR_GROUP_ALL:
    case TAR_GROUP_OBJ:
    case TAR_GROUP_IGNORE:
      group_cast( sn, URANGE( 1, ch->level, LEVEL_HERO ), ch, arg2 );
      return;

    case TAR_IGNORE:
      break;

    case TAR_CHAR_OFFENSIVE:

      if ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) ) {
        send_to_char( AT_BLUE, "You failed.\n\r", ch );
        return;
      }

      if ( arg2[ 0 ] == '\0' ) {
        if ( !( victim = ch->fighting ) ) {
          send_to_char( AT_BLUE, "Cast the spell on whom?\n\r", ch );
          return;
        }
      } else {
        if ( !( victim = get_char_room( ch, arg2 ) ) ) {
          send_to_char( AT_BLUE, "They aren't here.\n\r", ch );
          return;
        }
      }

      if ( IS_AFFECTED( victim, AFF_PEACE ) ) {
        send_to_char( AT_WHITE, "A wave of peace overcomes you.\n\r", ch );
        return;
      }

      if ( IS_AFFECTED( ch, AFF_MUTE ) ) {
        send_to_char( AT_WHITE, "You have been silenced.\n\r", ch );
        return;
      }

      if ( IS_AFFECTED( ch, AFF_PEACE ) ) {
        affect_strip( ch, skill_lookup( "aura of peace" ) );
        REMOVE_BIT( ch->affected_by, AFF_PEACE );
      }

      if ( is_safe( ch, victim ) ) {
        send_to_char( AT_BLUE, "You failed.\n\r", ch );
        return;
      }

      vo = (void *) victim;
      break;

    case TAR_CHAR_DEFENSIVE:

      if ( arg2[ 0 ] == '\0' ) {
        victim = ch;
      } else {
        if ( !( victim = get_char_room( ch, arg2 ) ) ) {
          send_to_char( AT_BLUE, "They aren't here.\n\r", ch );
          return;
        }
      }

      vo = (void *) victim;
      break;

    case TAR_CHAR_SELF:

      if ( arg2[ 0 ] != '\0' && !is_name( ch, arg2, ch->name ) ) {
        send_to_char( AT_BLUE, "You cannot cast this spell on another.\n\r", ch );
        return;
      }

      vo = (void *) ch;
      break;

    case TAR_OBJ_INV:

      if ( arg2[ 0 ] == '\0' ) {
        send_to_char( AT_BLUE, "What should the spell be cast upon?\n\r", ch );
        return;
      }

      if ( !( obj = get_obj_carry( ch, arg2 ) ) ) {
        send_to_char( AT_BLUE, "You are not carrying that.\n\r", ch );
        return;
      }

      vo = (void *) obj;
      break;
  }

  WAIT_STATE( ch, skill_table[ sn ].beats );

  if ( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) ) {
    send_to_char( AT_BLUE, "You failed.\n\r", ch );
    return;
  }

  if ( !IS_NPC( ch ) ) {
    update_skpell( ch, sn );
  }

  ( *skill_table[ sn ].spell_fun )( sn, URANGE( 1, ch->level, LEVEL_HERO ),
                                    ch, vo );

  if ( vo ) {
    oprog_invoke_trigger( pObj, ch, vo );

    if ( skill_table[ sn ].target == TAR_OBJ_INV ) {
      oprog_cast_sn_trigger( obj, ch, sn, vo );
    }

    rprog_cast_sn_trigger( ch->in_room, ch, sn, vo );
  }

  if ( skill_table[ sn ].target == TAR_CHAR_OFFENSIVE
       && victim->master != ch && victim != ch && IS_AWAKE( victim ) ) {
    CHAR_DATA * vch;

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room ) {
      if ( vch->deleted ) {
        continue;
      }

      if ( victim == vch && !victim->fighting ) {
        multi_hit( victim, ch, TYPE_UNDEFINED );
        break;
      }
    }
  }

  return;
}

void do_cast( CHAR_DATA * ch, char * argument ) {
  void      * vo;
  OBJ_DATA  * obj;
  CHAR_DATA * victim;
  char        arg1[ MAX_INPUT_LENGTH ];
  char        arg2[ MAX_INPUT_LENGTH ];
  int         mana;
  int         sn;
  bool        IS_DIVINE;

  sn = skill_lookup( arg1 );

  IS_DIVINE = FALSE;

  target_name = one_argument( argument, arg1 );

  if ( arg1[ 0 ] != '\0' ) {
    if ( !str_prefix( arg1, "divine" ) && ch->level >= LEVEL_IMMORTAL ) {
      IS_DIVINE   = TRUE;
      target_name = one_argument( target_name, arg1 );
    }
  }

  one_argument( target_name, arg2 );

  if ( arg1[ 0 ] == '\0' ) {
    send_to_char( AT_BLUE, "Cast which what where?\n\r", ch );
    return;
  }

  if ( IS_NPC( ch ) ) {
    if ( IS_SET( ch->affected_by, AFF_CHARM ) ) {
      return;
    }
  }

  if ( !IS_NPC( ch ) ) {
    if ( sn < 0 || !can_use_skpell( ch, sn ) ) {
      send_to_char( AT_BLUE, "You don't know that spell.\n\r", ch );
      return;
    }
  } else {
    if ( sn < 0 ) {
      return;
    }
  }

  if ( IS_AFFECTED( ch, AFF_MUTE ) ) {
    send_to_char( AT_WHITE, "You have been silenced.\n\r", ch );
    return;
  }

  if ( ch->position < skill_table[ sn ].minimum_position ) {
    send_to_char( AT_BLUE, "You can't concentrate enough.\n\r", ch );
    return;
  }

  if ( IS_STUNNED( ch, STUN_MAGIC ) ) {
    send_to_char( AT_LBLUE, "You're too stunned to cast spells.\n\r", ch );
    return;
  }

  if ( !IS_NPC( ch ) && ( !( ch->level > LEVEL_MORTAL ) ) ) {
    mana = MANA_COST( ch, sn );

    if ( ch->race == RACE_ELF ) {
      mana -= mana / 4;
    }
  } else {
    mana = 0;
  }

  if ( skill_table[ sn ].spell_fun == spell_null ) {
    send_to_char( AT_BLUE, "That's not a spell.\n\r", ch );
    return;
  }

  /*
   * Locate targets.
   */
  victim = NULL;
  obj    = NULL;
  vo     = NULL;

  switch ( skill_table[ sn ].target ) {
    default:
      bug( "Do_cast: bad target for sn %d.", sn );
      return;

    case TAR_GROUP_OFFENSIVE:
    case TAR_GROUP_DEFENSIVE:
    case TAR_GROUP_ALL:
    case TAR_GROUP_OBJ:
    case TAR_GROUP_IGNORE:
      group_cast( sn, URANGE( 1, ch->level, LEVEL_HERO ), ch, arg2 );
      return;

    case TAR_IGNORE:
      break;

    case TAR_CHAR_OFFENSIVE:

      if ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) ) {
        send_to_char( AT_BLUE, "You failed.\n\r", ch );
        return;
      }

      if ( arg2[ 0 ] == '\0' ) {
        if ( !( victim = ch->fighting ) ) {
          send_to_char( AT_BLUE, "Cast the spell on whom?\n\r", ch );
          return;
        }
      } else {
        if ( !( victim = get_char_room( ch, arg2 ) ) ) {
          send_to_char( AT_BLUE, "They aren't here.\n\r", ch );
          return;
        }
      }

      if ( IS_AFFECTED( victim, AFF_PEACE ) ) {
        send_to_char( AT_WHITE, "A wave of peace overcomes you.\n\r", ch );
        return;
      }

      if ( IS_AFFECTED( ch, AFF_PEACE ) ) {
        affect_strip( ch, skill_lookup( "aura of peace" ) );
        REMOVE_BIT( ch->affected_by, AFF_PEACE );
      }

      if ( is_safe( ch, victim ) ) {
        send_to_char( AT_BLUE, "You failed.\n\r", ch );
        return;
      }

      vo = (void *) victim;
      break;

    case TAR_CHAR_DEFENSIVE:

      if ( arg2[ 0 ] == '\0' ) {
        victim = ch;
      } else {
        if ( !( victim = get_char_room( ch, arg2 ) ) ) {
          send_to_char( AT_BLUE, "They aren't here.\n\r", ch );
          return;
        }
      }

      vo = (void *) victim;
      break;

    case TAR_CHAR_SELF:

      if ( arg2[ 0 ] != '\0' && !is_name( ch, arg2, ch->name ) ) {
        send_to_char( AT_BLUE, "You cannot cast this spell on another.\n\r", ch );
        return;
      }

      vo = (void *) ch;
      break;

    case TAR_OBJ_INV:

      if ( arg2[ 0 ] == '\0' ) {
        send_to_char( AT_BLUE, "What should the spell be cast upon?\n\r", ch );
        return;
      }

      if ( !( obj = get_obj_here( ch, arg2 ) ) ) {
        send_to_char( AT_BLUE, "You can't find that.\n\r", ch );
        return;
      }

      vo = (void *) obj;
      break;
  }

  if ( !IS_NPC( ch ) && ch->mana < mana ) {
    send_to_char( AT_BLUE, "You don't have enough mana.\n\r", ch );
    return;
  }

  say_spell( ch, sn );

  if ( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) ) {
    send_to_char( AT_BLUE, "You failed.\n\r", ch );
    return;
  }

  if ( ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) ) && ( skill_table[ sn ].target == TAR_CHAR_OFFENSIVE ) ) {
    send_to_char( AT_BLUE, "You failed.\n\r", ch );
    return;
  }

  WAIT_STATE( ch, skill_table[ sn ].beats );

  if ( !IS_NPC( ch ) ) {
    if ( number_percent() > ch->pcdata->learned[ sn ] ) {
      send_to_char( AT_BLUE, "You lost your concentration.\n\r", ch );

      ch->mana -= mana / 2;
    } else {
      ch->mana -= mana;

      if ( ( IS_AFFECTED2( ch, AFF_CONFUSED ) ) && number_percent() < 10 ) {
        act( AT_YELLOW, "$n looks around confused at what's going on.", ch, NULL, NULL, TO_ROOM );
        send_to_char( AT_YELLOW, "You become confused and botch the spell.\n\r", ch );
        return;
      }

      update_skpell( ch, sn );
      ( *skill_table[ sn ].spell_fun )( sn, IS_DIVINE ? URANGE( 1, ch->level, LEVEL_HERO ) * 3 : URANGE( 1, ch->level, LEVEL_HERO ), ch, vo );
    }
  }

  if ( IS_NPC( ch ) ) {
    ( *skill_table[ sn ].spell_fun )( sn, IS_DIVINE ? URANGE( 1, ch->level, LEVEL_HERO ) * 3 : URANGE( 1, ch->level, LEVEL_HERO ), ch, vo );
  }

  if ( vo ) {
    if ( skill_table[ sn ].target == TAR_OBJ_INV ) {
      oprog_cast_sn_trigger( obj, ch, sn, vo );
    }

    rprog_cast_sn_trigger( ch->in_room, ch, sn, vo );
  }

  if ( skill_table[ sn ].target == TAR_CHAR_OFFENSIVE && victim->master != ch && victim != ch && IS_AWAKE( victim ) ) {
    CHAR_DATA * vch;

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room ) {
      if ( vch->deleted ) {
        continue;
      }

      if ( victim == vch && !victim->fighting ) {
        multi_hit( victim, ch, TYPE_UNDEFINED );
        break;
      }
    }
  }

  return;
}

/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj ) {
  void * vo;

  if ( sn <= 0 ) {
    return;
  }

  if ( !is_sn( sn ) || skill_table[ sn ].spell_fun == 0 ) {
    bug( "Obj_cast_spell: bad sn %d.", sn );
    return;
  }

  if ( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) ) {
    send_to_char( AT_BLUE, "The magic of the item fizzles.\n\r", ch );
    return;
  }

  switch ( skill_table[ sn ].target ) {
    default:
      bug( "Obj_cast_spell: bad target for sn %d.", sn );
      return;

    case TAR_GROUP_OFFENSIVE:
    case TAR_GROUP_DEFENSIVE:
    case TAR_GROUP_ALL:
    case TAR_GROUP_OBJ:
    case TAR_GROUP_IGNORE:
      group_cast( sn, URANGE( 1, level, LEVEL_HERO ), ch,
                  victim ? (void *)victim : (void *)obj );
      return;

    case TAR_IGNORE:
      vo = NULL;
      break;

    case TAR_CHAR_OFFENSIVE:

      if ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) ) {
        send_to_char( AT_BLUE, "The magic of the item fizzles.\n\r", ch );
        return;
      }

      if ( !victim ) {
        victim = ch->fighting;
      }

      if ( !victim || ( !IS_NPC( victim ) && ch != victim ) ) {
        send_to_char( AT_BLUE, "You're not fighting anyone.\n\r", ch );
        return;
      }

      if ( ( ( ch->clan == 0 ) || ( ch->clan == 0 ) ) && ( !IS_NPC( victim ) ) ) {
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

      if ( ( ( ch->level - 9 > victim->level ) || ( ch->level + 9 < victim->level ) ) && ( !IS_NPC( victim ) ) ) {
        send_to_char( AT_WHITE, "That is not in the pkill range... valid range is +/- 8 levels.\n\r", ch );
        return;
      }

      vo = (void *) victim;
      break;

    case TAR_CHAR_DEFENSIVE:

      if ( !victim ) {
        victim = ch;
      }

      vo = (void *) victim;
      break;

    case TAR_CHAR_SELF:
      vo = (void *) ch;
      break;

    case TAR_OBJ_INV:

      if ( !obj ) {
        send_to_char( AT_BLUE, "You can't find that.\n\r", ch );
        return;
      }

      vo = (void *) obj;
      break;
  }

  /*    target_name = "";*/
  ( *skill_table[ sn ].spell_fun )( sn, level, ch, vo );

  if ( vo ) {
    if ( skill_table[ sn ].target == TAR_OBJ_INV ) {
      oprog_cast_sn_trigger( obj, ch, sn, vo );
    }

    rprog_cast_sn_trigger( ch->in_room, ch, sn, vo );
  }

  if ( skill_table[ sn ].target == TAR_CHAR_OFFENSIVE
       && victim->master != ch && ch != victim ) {
    CHAR_DATA * vch;

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room ) {
      if ( vch->deleted ) {
        continue;
      }

      if ( victim == vch && !victim->fighting ) {
        multi_hit( victim, ch, TYPE_UNDEFINED );
        break;
      }
    }
  }

  return;
}

void spell_animate( int sn, int level, CHAR_DATA * ch, void * vo ) {
  OBJ_DATA * obj = (OBJ_DATA *) vo;

  if ( obj->item_type != ITEM_CORPSE_NPC ) {
    send_to_char( AT_BLUE, "You cannot animate that.\n\r", ch );
    return;
  }

  if ( obj->deleted ) {
    return;
  }

  magic_mob( ch, obj, obj->ac_vnum );
  extract_obj( obj );

  return;
}

void spell_armor( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = 24;
  af.location  = APPLY_AC;
  af.modifier  = -25;
  af.bitvector = 0;
  affect_to_char( victim, &af );

  if ( ch != victim ) {
    send_to_char( AT_BLUE, "Ok.\n\r", ch );
  }

  send_to_char( AT_BLUE, "You feel someone protecting you.\n\r", victim );
  return;
}

void spell_blindness( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_BLIND ) || saves_spell( level, victim ) ) {
    send_to_char( AT_BLUE, "You have failed.\n\r", ch );
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = 5;
  af.location  = APPLY_HITROLL;
  af.modifier  = -10;
  af.bitvector = AFF_BLIND;
  affect_to_char( victim, &af );

  act( AT_WHITE, "$N is blinded!", ch, NULL, victim, TO_CHAR );
  send_to_char( AT_WHITE, "You are blinded!\n\r", victim );
  act( AT_WHITE, "$N is blinded!", ch, NULL, victim, TO_NOTVICT );
  return;
}

void spell_charm_person( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( victim == ch ) {
    send_to_char( AT_BLUE, "You like yourself even better!\n\r", ch );
    return;
  }

  if ( !IS_NPC( victim ) ) {
    return;
  }

  if (   IS_AFFECTED( victim, AFF_CHARM )
         || IS_AFFECTED( ch, AFF_CHARM )
         || level < victim->level
         || saves_spell( level, victim ) ) {
    return;
  }

  if ( IS_SIMM( victim, IMM_CHARM ) ) {
    return;
  }

  if ( victim->master ) {
    stop_follower( victim );
  }

  add_follower( victim, ch );
  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy( level / 6 );
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char( victim, &af );

  send_to_char( AT_BLUE, "Ok.\n\r", ch );
  act( AT_BLUE, "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
  return;
}

void spell_cure_blindness( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;

  if ( !is_affected( victim, gsn_blindness ) ) {
    return;
  }

  affect_strip( victim, gsn_blindness );

  if ( ch != victim ) {
    send_to_char( AT_BLUE, "Ok.\n\r", ch );
  }

  send_to_char( AT_WHITE, "Your vision returns!\n\r", victim );
  return;
}

void spell_cure_poison( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;

  if ( !is_affected( victim, gsn_poison ) ) {
    return;
  }

  affect_strip( victim, gsn_poison );

  send_to_char( AT_GREEN, "Ok.\n\r", ch );
  send_to_char( AT_GREEN, "A warm feeling runs through your body.\n\r", victim );
  act( AT_GREEN, "$N looks better.", ch, NULL, victim, TO_NOTVICT );

  return;
}

void spell_truesight( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED2( victim, AFF_TRUESIGHT ) ) {
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = level / 8;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_TRUESIGHT;
  affect_to_char2( victim, &af );

  if ( ch != victim ) {
    send_to_char( AT_BLUE, "Ok.\n\r", ch );
  }

  send_to_char( AT_BLUE, "Your eyes tingle.\n\r", victim );
  return;
}

void spell_fly( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_FLYING ) ) {
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = level + 3;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_FLYING;
  affect_to_char( victim, &af );

  send_to_char( AT_BLUE, "Your feet rise off the ground.\n\r", victim );
  act( AT_BLUE, "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_giant_strength( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_STR;
  af.modifier  = 1 + ( level >= 18 ) + ( level >= 25 );
  af.bitvector = 0;
  affect_to_char( victim, &af );

  if ( ch != victim ) {
    send_to_char( AT_BLUE, "Ok.\n\r", ch );
  }

  send_to_char( AT_BLUE, "You feel stronger.\n\r", victim );
  return;
}

void spell_heal( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  int         heal;

  heal = UMIN( ( victim->hit + ( ch->level * 5 ) ), MAX_HIT( victim ) );

  // if ( ch->race == RACE_ANGEL )
  //  heal = heal * 2;
  if ( heal > MAX_HIT( victim ) ) {
    heal = MAX_HIT( victim );
  }

  victim->hit = heal;
  update_pos( victim );

  if ( ch != victim ) {
    send_to_char( AT_BLUE, "Ok.\n\r", ch );
  }

  send_to_char( AT_BLUE, "A warm feeling fills your body.\n\r", victim );
  return;
}

void spell_infravision( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_INFRARED ) ) {
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = 2 * level;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_INFRARED;
  affect_to_char( victim, &af );

  send_to_char( AT_RED, "Your eyes glow.\n\r", victim );
  act( AT_RED, "$n's eyes glow.\n\r", ch, NULL, NULL, TO_ROOM );
  return;
}

void spell_incinerate( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( saves_spell( level, victim ) ) {
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_FLAMING;
  affect_join( victim, &af );

  if ( ch != victim ) {
    send_to_char( AT_RED, "Ok.\n\r", ch );
  }

  send_to_char( AT_RED, "Your body bursts into flames!\n\r", victim );
  return;
}

void spell_invis( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_INVISIBLE ) ) {
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = 24;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_INVISIBLE;
  affect_to_char( victim, &af );

  send_to_char( AT_GREY, "You fade out of existence.\n\r", victim );
  act( AT_GREY, "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_locate_object( int sn, int level, CHAR_DATA * ch, void * vo ) {
  OBJ_DATA * obj;
  OBJ_DATA * in_obj;
  char       buf[ MAX_INPUT_LENGTH ];
  bool       found;

  found = FALSE;

  for ( obj = object_list; obj; obj = obj->next ) {
    if ( !can_see_obj( ch, obj ) || !is_name( ch, target_name, obj->name ) ) {
      continue;
    }

    if ( IS_SET( obj->extra_flags, ITEM_NO_LOCATE ) && ( get_trust( ch ) < L_APP ) ) {
      continue;
    }

    found = TRUE;

    for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj ) {
      ;
    }

    if ( in_obj->carried_by ) {
      sprintf( buf, "%s carried by %s.\n\r", obj->short_descr, visible_name( in_obj->carried_by, ch, TRUE ) );
    } else if ( in_obj->stored_by ) {
      sprintf( buf, "%s in storage.\n\r", obj->short_descr );
    } else {
      sprintf( buf, "%s in %s.\n\r", obj->short_descr, !in_obj->in_room ? "somewhere" : in_obj->in_room->name );
    }

    buf[ 0 ] = UPPER( buf[ 0 ] );
    send_to_char( AT_BLUE, buf, ch );
  }

  if ( !found ) {
    send_to_char( AT_WHITE, "Nothing like that in hell, earth, or heaven.\n\r", ch );
  }

  return;
}

void spell_null( int sn, int level, CHAR_DATA * ch, void * vo ) {
  send_to_char( AT_WHITE, "That's not a spell!\n\r", ch );
  return;
}

void spell_pass_door( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_PASS_DOOR ) ) {
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy( level / 4 );
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_PASS_DOOR;
  affect_to_char( victim, &af );

  send_to_char( AT_GREY, "You turn translucent.\n\r", victim );
  act( AT_GREY, "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_poison( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( saves_spell( level, victim ) ) {
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_STR;
  af.modifier  = -2;
  af.bitvector = AFF_POISON;
  affect_join( victim, &af );

  if ( ch != victim ) {
    send_to_char( AT_GREEN, "Ok.\n\r", ch );
  }

  send_to_char( AT_GREEN, "You feel very sick.\n\r", victim );
  return;
}

void spell_confusion( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  char        buf[ MAX_STRING_LENGTH ];

  if ( IS_AFFECTED2( victim, AFF_CONFUSED ) ) {
    return;
  }

  if ( saves_spell( level, victim ) ) {
    send_to_char( AT_BLUE, "You failed.\n\r", ch );
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy( level / 10 );
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_CONFUSED;
  affect_to_char2( victim, &af );

  sprintf( buf, "You feel disorientated.\n\r" );
  send_to_char( AT_WHITE, buf, victim );
  act( AT_WHITE, "$n stares around blankly.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_scry( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_SCRY ) ) {
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy( level / 4 );
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_SCRY;
  affect_to_char( victim, &af );

  send_to_char( AT_BLUE, "Your vision improves.\n\r", victim );
  return;
}

void spell_sleep( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_SLEEP )
       || level < victim->level
       || ( saves_spell( level + IS_SRES( victim, RES_MAGIC ) ? -5 : 0, victim )
            && !( get_trust( ch ) > LEVEL_IMMORTAL ) ) ) {
    send_to_char( AT_BLUE, "You failed.\n\r", ch );
    return;
  }

  if ( IS_SIMM( victim, IMM_MAGIC ) ) {
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = 4 + level;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_SLEEP;
  affect_join( victim, &af );

  if ( IS_AWAKE( victim ) ) {
    send_to_char( AT_BLUE, "You feel very sleepy ..... zzzzzz.\n\r", victim );

    if ( victim->position == POS_FIGHTING ) {
      stop_fighting( victim, TRUE );
    }

    do_sleep( victim, "" );
  }

  return;
}

void spell_spell_bind( int sn, int level, CHAR_DATA * ch, void * vo ) {
  bool       Charged = 0;
  OBJ_DATA * obj     = (OBJ_DATA *) vo;

  if ( obj->item_type == ITEM_WAND
       || obj->item_type == ITEM_STAFF
       || obj->item_type == ITEM_LENSE ) {
    if ( obj->value[ 2 ] < obj->value[ 1 ] ) {
      obj->value[ 2 ] = obj->value[ 1 ];
      Charged++;
    }
  } else if ( obj->ac_type == 5 && obj->ac_spell ) {
    if ( obj->ac_charge[ 0 ] < obj->ac_charge[ 1 ] && obj->ac_charge[ 1 ] != -1 ) {
      obj->ac_charge[ 0 ]++;
      Charged++;
    }
  } else {
    send_to_char( AT_BLUE, "You cannot bind magic to that item.\n\r", ch );
    return;
  }

  if ( !Charged ) {
    send_to_char( AT_BLUE, "That item is at full charge.\n\r", ch );
    return;
  }

  act( AT_BLUE, "You slowly pass your hand over $p, it vibrates slowly.", ch, obj, NULL, TO_CHAR );
  act( AT_BLUE, "$n slowly passes $s hand over $p, it vibrates slowly.", ch, obj, NULL, TO_ROOM );
  return;
}

void spell_summon( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim;

  if ( !( victim = get_char_world( ch, target_name ) )
       || victim == ch
       || !victim->in_room
       || IS_SET( victim->in_room->room_flags, ROOM_SAFE )
       || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
       || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
       || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
       || victim->level >= level + 3
       || victim->fighting
       || ( IS_NPC( victim ) && saves_spell( level, victim ) )
       || IS_SET( victim->in_room->area->area_flags, AREA_PROTOTYPE ) ) {
    send_to_char( AT_BLUE, "You failed.\n\r", ch );
    return;
  }

  act( AT_BLUE, "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
  char_from_room( victim );
  char_to_room( victim, ch->in_room );
  act( AT_BLUE, "$n has summoned you!", ch, NULL, victim, TO_VICT );
  act( AT_BLUE, "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
  send_to_char( AT_BLUE, "You feel a wave of nausea come over you.\n\r", ch );
  ch->position = POS_STUNNED;
  update_pos( ch );
  STUN_CHAR( ch, 3, STUN_COMMAND );
  do_look( victim, "auto" );
  return;
}

void spell_word_of_recall( int sn, int level, CHAR_DATA * ch, void * vo ) {
  do_recall( (CHAR_DATA *) vo, "" );
  return;
}

void perm_spell( CHAR_DATA * victim, int sn ) {
  AFFECT_DATA * af;

  if ( is_affected( victim, sn ) ) {
    for ( af = victim->affected; af != NULL; af = af->next ) {
      if ( af->type == sn ) {
        af->duration = -1;
      }
    }
  }

  return;
}

int spell_duration( CHAR_DATA * victim, int sn ) {
  AFFECT_DATA * af;

  if ( is_affected( victim, sn ) ) {
    for ( af = victim->affected; af != NULL; af = af->next ) {
      if ( af->type == sn ) {
        return af->duration;
      }
    }
  }

  return -2;
}

/* RT save for dispels */
/* modified for envy -XOR */
bool saves_dispel( int dis_level, int spell_level, int duration ) {
  int save;

  if ( duration == -1 ) {
    spell_level += 5;/* very hard to dispel permanent effects */
  }

  save = 50 + ( spell_level - dis_level ) * 5;
  save = URANGE( 5, save, 95 );
  return number_percent() < save;
}

/* co-routine for dispel magic */
bool check_dispel( int dis_level, CHAR_DATA * victim, int sn ) {
  AFFECT_DATA * af;

  if ( is_affected( victim, sn ) ) {
    for ( af = victim->affected; af != NULL; af = af->next ) {
      if ( af->type == sn ) {
        if ( !saves_spell( dis_level, victim ) ) {
          /*	if(!saves_dispel(dis_level,victim->level,af->duration))*/
          affect_strip( victim, sn );

          if ( skill_table[ sn ].msg_off ) {
            send_to_char( C_DEFAULT, skill_table[ sn ].msg_off, victim );
            send_to_char( C_DEFAULT, "\n\r", victim );

            if ( skill_table[ sn ].room_msg_off ) {
              act( C_DEFAULT, skill_table[ sn ].room_msg_off,
                   victim, NULL, NULL, TO_ROOM );
            }
          }

          return TRUE;
        } else {
          af->level--;
        }
      }
    }
  }

  return FALSE;
}

/* Mobs built with spells only have the flag.
 * These function dispels those spells
 *  -Decklarean
 */

void check_dispel_aff( CHAR_DATA * victim, bool * found, int level, const char * spell, long vector ) {
  int sn;
  sn = skill_lookup( spell );

  if ( IS_AFFECTED( victim, vector )
       && !saves_spell( level, victim )
       /*   && !saves_dispel(level, victim->level,1) */
       && !is_affected( victim, sn ) ) {
    *found = TRUE;
    REMOVE_BIT( victim->affected_by, vector );

    if ( skill_table[ sn ].msg_off ) {
      act( C_DEFAULT, skill_table[ sn ].msg_off,
           victim, NULL, NULL, TO_CHAR );

      if ( skill_table[ sn ].room_msg_off ) {
        act( C_DEFAULT, skill_table[ sn ].room_msg_off,
             victim, NULL, NULL, TO_ROOM );
      }
    }

    if ( vector == AFF_FLYING ) {
      check_nofloor( victim );
    }
  }
}

void check_dispel_aff2( CHAR_DATA * victim, bool * found, int level, const char * spell, long vector ) {
  int sn;
  sn = skill_lookup( spell );

  if ( IS_AFFECTED2( victim, vector )
       && !saves_dispel( level, victim->level, -1 )
       && !is_affected( victim, sn ) ) {
    *found = TRUE;
    REMOVE_BIT( victim->affected_by2, vector );

    if ( skill_table[ sn ].msg_off ) {
      act( C_DEFAULT, skill_table[ sn ].msg_off,
           victim, NULL, NULL, TO_CHAR );

      if ( skill_table[ sn ].room_msg_off ) {
        act( C_DEFAULT, skill_table[ sn ].room_msg_off,
             victim, NULL, NULL, TO_ROOM );
      }
    }

    if ( vector == AFF_FLYING ) {
      check_nofloor( victim );
    }
  }
}

bool dispel_flag_only_spells( int level, CHAR_DATA * victim ) {
  bool found;
  found = FALSE;

  check_dispel_aff( victim, &found, level, "blindness", AFF_BLIND );
  check_dispel_aff( victim, &found, level, "charm person", AFF_CHARM );
  check_dispel_aff( victim, &found, level, "flaming", AFF_FLAMING );
  check_dispel_aff( victim, &found, level, "fly", AFF_FLYING );
  check_dispel_aff( victim, &found, level, "infravision", AFF_INFRARED );
  check_dispel_aff( victim, &found, level, "invis", AFF_INVISIBLE );
  check_dispel_aff( victim, &found, level, "pass door", AFF_PASS_DOOR );
  check_dispel_aff( victim, &found, level, "sleep", AFF_SLEEP );
  check_dispel_aff2( victim, &found, level, "true sight", AFF_TRUESIGHT );

  return found;
}

/* New dispel magic by Decklarean
 * The old way was just to stupid. :>
 * This will dispel all magic spells.
 */

void spell_dispel_magic( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA   * victim = (CHAR_DATA *) vo;
  AFFECT_DATA * paf;
  bool          found;

  if ( saves_spell( level, victim ) ) {
    send_to_char( AT_RED, "You feel a brief tingling sensation.\n\r", victim );
    send_to_char( AT_RED, "The spell failed.\n\r", ch );
    return;
  }

  found = FALSE;

  /* Check dispel of spells that mobs where built with */
  if ( IS_NPC( victim ) ) {
    found = dispel_flag_only_spells( level, victim );
  }

  /* Check dispel of spells cast */
  for ( paf = victim->affected; paf; paf = paf->next ) {
    if ( paf->deleted ) {
      continue;
    }

    if ( skill_table[ paf->type ].spell_fun != spell_null
         && skill_table[ paf->type ].dispelable == TRUE ) {
      if ( check_dispel( level, victim, paf->type ) ) {
        found = TRUE;
      }
    }
  }

  for ( paf = victim->affected2; paf; paf = paf->next ) {
    if ( paf->deleted ) {
      continue;
    }

    if ( skill_table[ paf->type ].spell_fun != spell_null
         && skill_table[ paf->type ].dispelable == TRUE ) {
      if ( check_dispel( level, victim, paf->type ) ) {
        found = TRUE;
      }
    }
  }

  if ( found ) {
    send_to_char( AT_RED, "You feel a brief tingling sensation.\n\r", victim );
    send_to_char( AT_YELLOW,
                  "Unraveled magical energy ripple away at your succes.\n\r", ch );
  } else {
    send_to_char( AT_RED, "The spell failed.\n\r", ch );
  }

}

/* Adds + dam to spells for having spellcraft skill */
int sc_dam( CHAR_DATA * ch, int dam ) {
  double mod;

  if ( ch->level < 50 ) {
    mod = 82.6;   /* x1.15 */
  } else if ( ch->level < 60 ) {
    mod = 73.07;  /* x1.3  */
  } else if ( ch->level < 70 ) {
    mod = 65.51;  /* x1.45 */
  } else if ( ch->level < 80 ) {
    mod = 55.88;  /* x1.7  */
  } else if ( ch->level < 90 ) {
    mod = 51.35;  /* x1.85 */
  } else if ( ch->level < 95 ) {
    mod = 47.5;   /* x2    */
  } else {
    mod = 38;     /* x2.5  */
  }

  if ( !IS_NPC( ch ) && ch->pcdata->learned[ gsn_spellcraft ] > 0 ) {
    dam += dam * ch->pcdata->learned[ gsn_spellcraft ] / mod;
    update_skpell( ch, gsn_spellcraft );
  }

  return dam;
}

void spell_silence( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_MUTE ) || saves_spell( level, victim ) ) {
    send_to_char( AT_BLUE, "You have failed.\n\r", ch );
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = 2;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_MUTE;
  affect_to_char( victim, &af );

  act( AT_WHITE, "$N is silenced!", ch, NULL, victim, TO_CHAR );
  send_to_char( AT_WHITE, "You are silenced!\n\r", victim );
  act( AT_WHITE, "$N is silenced!", ch, NULL, victim, TO_NOTVICT );
  return;
}

void spell_hallucinate( int sn, int level, CHAR_DATA * ch, void * vo ) {
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED2( victim, AFF_HALLUCINATING ) ) {
    return;
  }

  if ( IS_AFFECTED( victim, AFF_BLIND ) || saves_spell( level, victim ) ) {
    send_to_char( AT_BLUE, "You have failed.\n\r", ch );
    return;
  }

  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy( level / 6 );
  af.location  = APPLY_INT;
  af.modifier  = -4;
  af.bitvector = AFF_HALLUCINATING;
  affect_to_char2( victim, &af );

  act( AT_WHITE, "&.Thou&.sand&.s &.of &.danci&.ng &.ligh&.ts &.surr&.ound &.you&.!&w", victim, NULL, victim, TO_VICT );
  act( AT_GREY, "&W$n's &.body &.is &.surr&.ounded &.by d&.anci&.ng l&.ights.", victim, NULL, NULL, TO_ROOM );
  return;
}

