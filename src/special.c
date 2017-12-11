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

/* call some stuff from fight.c */
extern void set_fighting( CHAR_DATA * ch, CHAR_DATA * victim );
extern void trip( CHAR_DATA * ch, CHAR_DATA * victim );

/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN( spec_executioner );
DECLARE_SPEC_FUN( spec_fido );
DECLARE_SPEC_FUN( spec_guard );
DECLARE_SPEC_FUN( spec_janitor );
DECLARE_SPEC_FUN( spec_mayor );
DECLARE_SPEC_FUN( spec_poison );
DECLARE_SPEC_FUN( spec_repairman );
DECLARE_SPEC_FUN( spec_thief );

/*
 * Special Functions Table.	OLC
 */
const struct  spec_type spec_table  [] = {
  /*
   * Special function commands.
   */
  { "spec_executioner",      spec_executioner        },
  { "spec_fido",             spec_fido               },
  { "spec_guard",            spec_guard              },
  { "spec_janitor",          spec_janitor            },
  { "spec_mayor",            spec_mayor              },
  { "spec_poison",           spec_poison             },
  { "spec_repairman",        spec_repairman          },
  { "spec_thief",            spec_thief              },
  // fin.
  { "",                      0                       }
};

/*****************************************************************************
   Name:		spec_string
   Purpose:	Given a function, return the appropriate name.
   Called by:	<???>
 ****************************************************************************/
char * spec_string( SPEC_FUN * fun ) { /* OLC */
  int cmd;

  for ( cmd = 0; *spec_table[ cmd ].spec_fun; cmd++ ) { /* OLC 1.1b */
    if ( fun == spec_table[ cmd ].spec_fun ) {
      return spec_table[ cmd ].spec_name;
    }
  }

  return 0;
}

/*****************************************************************************
   Name:		spec_lookup
   Purpose:	Given a name, return the appropriate spec fun.
   Called by:	do_mset(act_wiz.c) load_specials,reset_area(db.c)
 ****************************************************************************/
SPEC_FUN * spec_lookup( const char * name ) { /* OLC */
  int cmd;

  for ( cmd = 0; *spec_table[ cmd ].spec_name; cmd++ ) { /* OLC 1.1b */
    if ( !str_cmp( name, spec_table[ cmd ].spec_name ) ) {
      return spec_table[ cmd ].spec_fun;
    }
  }

  return 0;
}

bool spec_executioner( CHAR_DATA * ch ) {
  CHAR_DATA * guard;
  CHAR_DATA * victim;
  char      * crime;
  char        buf[ MAX_STRING_LENGTH ];

  if ( !IS_AWAKE( ch ) || ch->fighting ) {
    return FALSE;
  }

  crime = "";

  for ( victim = ch->in_room->people; victim; victim = victim->next_in_room ) {
    if ( victim->deleted ) {
      continue;
    }

    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_KILLER ) ) {
      crime = "KILLER";
      break;
    }

    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_THIEF ) ) {
      crime = "THIEF";
      break;
    }
  }

  if ( !victim ) {
    return FALSE;
  }

  sprintf( buf, "%s is a %s!  JUSTICE WILL PREVAIL! I SENTENCE %s TO DEATH!!!",
           victim->name, crime, victim->name );
  do_yell( ch, buf );
  /*    if ( crime != "THIEF" )
      { */
  multi_hit( ch, victim, TYPE_UNDEFINED );

  guard = create_mobile( get_mob_index( MOB_VNUM_CITYGUARD ) );
  char_to_room( guard, ch->in_room );
  guard->fighting     = ch->fighting;
  guard->position     = POS_FIGHTING;
  guard->summon_timer = 15;

  guard = create_mobile( get_mob_index( MOB_VNUM_CITYGUARD ) );
  char_to_room( guard, ch->in_room );
  guard->fighting     = ch->fighting;
  guard->position     = POS_FIGHTING;
  guard->summon_timer = 15;

  /*         char_to_room( create_mobile( get_mob_index( MOB_VNUM_CITYGUARD) ),
         ch->in_room );
         char_to_room( create_mobile( get_mob_index( MOB_VNUM_CITYGUARD ) ),
         ch->in_room ); */
  /*  }*/
  return TRUE;
}

bool spec_fido( CHAR_DATA * ch ) {
  OBJ_DATA * obj;
  OBJ_DATA * obj_next;
  OBJ_DATA * corpse;
  OBJ_DATA * corpse_next;

  if ( !IS_AWAKE( ch ) ) {
    return FALSE;
  }

  for ( corpse = ch->in_room->contents; corpse; corpse = corpse_next ) {
    corpse_next = corpse->next_content;

    if ( corpse->deleted ) {
      continue;
    }

    if ( corpse->item_type != ITEM_CORPSE_NPC ) {
      continue;
    }

    act( C_DEFAULT, "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );

    for ( obj = corpse->contains; obj; obj = obj_next ) {
      obj_next = obj->next_content;

      if ( obj->deleted ) {
        continue;
      }

      obj_from_obj( obj );
      obj_to_room( obj, ch->in_room );
    }

    extract_obj( corpse );
    return TRUE;
  }

  return FALSE;
}

bool spec_guard( CHAR_DATA * ch ) {
  CHAR_DATA * victim;
  CHAR_DATA * ech;
  char      * crime;
  char        crimename[ MAX_STRING_LENGTH ];
  char        buf[ MAX_STRING_LENGTH ];
  int         max_evil, max_good;

  if ( !IS_AWAKE( ch ) || ch->fighting ) {
    return FALSE;
  }

  max_evil = 300;
  max_good = -300;
  ech      = NULL;
  crime    = "";

  for ( victim = ch->in_room->people; victim; victim = victim->next_in_room ) {
    if ( victim->deleted ) {
      continue;
    }

    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_KILLER ) ) {
      crime = "KILLER";
      break;
    }

    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_THIEF ) ) {
      crime = "THIEF";
      break;
    }

    if ( victim->fighting
         && ( IS_GOOD( ch ) || IS_NEUTRAL( ch ) )
         && victim->fighting != ch
         && victim->alignment < max_evil ) {
      max_evil = victim->alignment;
      ech      = victim;
    }

    if ( victim->fighting
         && IS_EVIL( ch )
         && victim->fighting != ch
         && victim->alignment > max_good ) {
      max_good = victim->alignment;
      ech      = victim;
    }
  }

  if ( victim ) {
    strcpy( crimename, "" );

    if ( !str_cmp( "KILLER", crime ) ) {
      strcpy( crimename, "KILLERS" );
    }

    if ( !str_cmp( "THIEF", crime ) ) {
      strcpy( crimename, "THIEVES" );
    }

    sprintf( buf, "%s is a %s!  JUSTICE WILL PREVAIL!! DEATH TO %s!!!",
             victim->name, crime, crimename );
    do_yell( ch, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
  }

  if ( ech ) {
    if ( IS_EVIL( ch ) ) {
      sprintf( buf, "$n screams 'EVIL MUST PREVAIL!!'" );
    } else {
      sprintf( buf, "$n screams 'PROTECT THE INNOCENT!!'" );
    }

    act( C_DEFAULT, buf, ch, NULL, NULL, TO_ROOM );
    multi_hit( ch, ech, TYPE_UNDEFINED );
    return TRUE;
  }

  return FALSE;
}

bool spec_janitor( CHAR_DATA * ch ) {
  OBJ_DATA * trash;
  OBJ_DATA * trash_next;

  if ( !IS_AWAKE( ch ) ) {
    return FALSE;
  }

  for ( trash = ch->in_room->contents; trash; trash = trash_next ) {
    trash_next = trash->next_content;

    if ( trash->deleted ) {
      continue;
    }

    if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) ) {
      continue;
    }

    act( C_DEFAULT, "$n picks up some unsightly garbage.", ch, NULL, NULL, TO_ROOM );
    act( C_DEFAULT, "You pick up $p.", ch, trash, NULL, TO_CHAR );
    obj_from_room( trash );
    obj_to_char( trash, ch );
    return TRUE;
  }

  return FALSE;
}

bool spec_mayor( CHAR_DATA * ch ) {
  static const char * path;
  static const char   open_path  [] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
  static const char close_path [] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";
  static int  pos;
  static bool move;

  if ( !move ) {
    if ( time_info.hour ==  6 ) {
      path = open_path;
      move = TRUE;
      pos  = 0;
    }

    if ( time_info.hour == 20 ) {
      path = close_path;
      move = TRUE;
      pos  = 0;
    }
  }

  if ( ch->fighting ) {
    return FALSE;
  }

  if ( !move || ch->position < POS_SLEEPING ) {
    return FALSE;
  }

  switch ( path[ pos ] ) {
    case '0':
    case '1':
    case '2':
    case '3':
      move_char( ch, path[ pos ] - '0', FALSE );
      break;

    case 'W':
      ch->position = POS_STANDING;
      act( C_DEFAULT, "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
      break;

    case 'S':
      ch->position = POS_SLEEPING;
      act( C_DEFAULT, "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
      break;

    case 'a':
      act( C_DEFAULT, "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
      break;

    case 'b':
      act( C_DEFAULT, "$n says 'What a view!  I must do something about that dump!'",
           ch, NULL, NULL, TO_ROOM );
      break;

    case 'c':
      act( C_DEFAULT, "$n says 'Vandals!  Youngsters have no respect for anything!'",
           ch, NULL, NULL, TO_ROOM );
      break;

    case 'd':
      act( C_DEFAULT, "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
      break;

    case 'e':
      act( C_DEFAULT, "$n says 'I hereby declare the city of Bethaven open!'",
           ch, NULL, NULL, TO_ROOM );
      break;

    case 'E':
      act( C_DEFAULT, "$n says 'I hereby declare the city of Bethaven closed!'",
           ch, NULL, NULL, TO_ROOM );
      break;

    case 'O':
      do_unlock( ch, "gate" );
      do_open( ch, "gate" );
      break;

    case 'C':
      do_close( ch, "gate" );
      do_lock( ch, "gate" );
      break;

    case '.':
      move = FALSE;
      break;
  }

  pos++;
  return FALSE;
}

bool spec_poison( CHAR_DATA * ch ) {
  CHAR_DATA * victim;

  if ( ch->position != POS_FIGHTING
       || !( victim = ch->fighting )
       || number_percent() > 2 * ch->level ) {
    return FALSE;
  }

  act( C_DEFAULT, "You bite $N!", ch, NULL, victim, TO_CHAR );
  act( C_DEFAULT, "$n bites you!", ch, NULL, victim, TO_VICT );
  act( C_DEFAULT, "$n bites $N!", ch, NULL, victim, TO_NOTVICT );
  spell_poison( gsn_poison, ch->level, ch, victim );
  return TRUE;
}

bool spec_thief( CHAR_DATA * ch ) {
  CHAR_DATA * victim;
  MONEY_DATA  amount;

  if ( ch->position == POS_FIGHTING ) {
    if ( ch->level < 40 ) {
      return FALSE;
    }

    victim = ch->fighting;

    switch ( number_range( 0, 2 ) ) {
      case 0:
        do_circle( ch, "" );
        return TRUE;
      case 1:
        do_gouge( ch, "" );
        return TRUE;
      case 2:
        trip( ch, victim );
        return TRUE;
    }
  }

  if ( ch->position != POS_STANDING ) {
    return FALSE;
  }

  for ( victim = ch->in_room->people; victim;
        victim = victim->next_in_room ) {
    if ( IS_NPC( victim )
         || victim->level >= LEVEL_IMMORTAL
         || number_bits( 3 ) != 0
         || !can_see( ch, victim ) ) { /* Thx Glop */
      continue;
    }

    if ( IS_AWAKE( victim ) && number_percent() >= ch->level ) {
      act( C_DEFAULT, "You discover $n's hands in your wallet!",
           ch, NULL, victim, TO_VICT );
      act( C_DEFAULT, "$N discovers $n's hands in $S wallet!",
           ch, NULL, victim, TO_NOTVICT );
      return TRUE;
    } else {
      amount.gold = amount.silver = amount.copper = 0;
      amount.gold = ( victim->money.gold   * number_range( 1, 20 ) / 100 );

      if ( chance( 50 ) ) {
        amount.silver = ( victim->money.silver * number_range( 1, 20 ) / 100 );
        amount.copper = ( victim->money.copper * number_range( 1, 20 ) / 100 );
      }

      ch->money.gold   += 7 * amount.gold   / 8;
      ch->money.silver += 7 * amount.silver / 8;
      ch->money.copper += 7 * amount.copper / 8;
      sub_money( &victim->money, &amount );
      return TRUE;
    }
  }

  return FALSE;
}

/*
 * spec_fun to repair bashed doors by Thelonius for EnvyMud.
 */
bool spec_repairman( CHAR_DATA * ch ) {
  EXIT_DATA       * pexit;
  EXIT_DATA       * pexit_rev;
  ROOM_INDEX_DATA * to_room;
  int               door;

  if ( !IS_AWAKE( ch ) ) {
    return FALSE;
  }

  door = dice( 1, MAX_DIR ) - 1;

  /*
   *  Could search through all doors randomly, but deathtraps would
   *  freeze the game!  And I'd prefer not to go through from 1 to 6...
   *  too boring.  Instead, just check one direction at a time.  There's
   *  a 51% chance they'll find the door within 4 tries anyway.
   *  -- Thelonius (Monk)
   */
  if ( !( pexit = ch->in_room->exit[ door ] ) ) {
    return FALSE;
  }

  if ( IS_SET( pexit->exit_info, EX_BASHED ) ) {
    REMOVE_BIT( pexit->exit_info, EX_BASHED );
    act( C_DEFAULT, "You repair the $d.", ch, NULL, pexit->keyword, TO_CHAR );
    act( C_DEFAULT, "$n repairs the $d.", ch, NULL, pexit->keyword, TO_ROOM );

    /* Don't forget the other side! */
    if ( ( to_room   = pexit->to_room               )
         && ( pexit_rev = to_room->exit[ direction_table[ door ].reverse ] )
         && pexit_rev->to_room == ch->in_room ) {
      CHAR_DATA * rch;

      REMOVE_BIT( pexit_rev->exit_info, EX_BASHED );

      for ( rch = to_room->people; rch; rch = rch->next_in_room ) {
        act( C_DEFAULT, "The $d is set back on its hinges.",
             rch, NULL, pexit_rev->keyword, TO_CHAR );
      }
    }

    return TRUE;
  }

  return FALSE;
}
