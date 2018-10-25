/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  In order to use any part of this Merc Diku Mud, you must comply with   *
*  both the original Diku license in 'license.doc' as well the Merc       *
*  license in 'license.txt'.  In particular, you may not remove either of *
*  these copyright notices.                                               *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/***************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com   *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this  *
*  code is allowed provided you add a credit line to the effect of:        *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest    *
*  of the standard diku/rom credits. If you use this or a modified version *
*  of this code, let me know via email: moongate@moongate.ams.com. Further *
*  updates will be posted to the rom mailing list. If you'd like to get    *
*  the latest version of quest.c, please send a request to the above add-  *
*  ress. Quest Code v2.00.                                                 *
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "quest.h"

ROOM_INDEX_DATA * room;
int               cnt =  0;

const struct quest_data quest_table[ ] = {
//{ vnum, qp,  }
  { 2426, 100, },
  { 2427, 200, },
  { 2428, 300, },
  { 2429, 100, },
  { 2430, 200, },
  { 2431, 300, },
  { 2432, 100, },
  { 2433, 200, },
  { 2434, 300, },
  { 2435, 100, },
  { 2436, 200, },
  { 2437, 300, },
  { 2438, 100, },
  { 2439, 200, },
  { 2440, 300, },
  { 2441, 150, },
  { 2442, 250, },
  { 2443, 350, },
  { 2444, 150, },
  { 2445, 250, },
  { 2446, 350, },
  { 2447, 150, },
  { 2448, 250, },
  { 2449, 350, },
  { 2450, 150, },
  { 2452, 350, },
  { 2453, 150, },
  { 2454, 250, },
  { 2455, 350, },
  { -1,   -1, }
};

const struct quest_item_type quest_item_table[ ] = {
//  name,          short,                         long
  { "picture",     "&Wan ancient picture",        "&WAn ancient picture of the &CRoyal Family&W lies here..."               },
  { "sword",       "&Wa sacred &Ysword",          "&WA hereditary &Ysword&W belonging to the &CRoyal Family&W lies here..." },
  { "gem",         "&Wa magical &pgem",           "&WA mysterious &pgem&W glittering faintly lies here..."                  },
  { "locket",      "&Wa &Rheart-shaped&W locket", "&WJekka's tiny &Rheart-shaped&W locket lies here..."                     },
  { "harp",        "&Wa &Cmagical&W harp",        "&WA &Cmagical&W harp playing a soft melody sits here..."                 },
  { "crest royal", "&Wthe &PRoyal Crest",         "&WThe &PRoyal Crest&W of &zHis Majesty&W sits here..."                   }
};

/* The main quest function */
void do_quest( CHAR_DATA * ch, char * argument ) {
  CHAR_DATA        * questman;
  OBJ_DATA         * obj = NULL;
  OBJ_DATA         * obj_next;
  OBJ_INDEX_DATA   * rewardObj;
  char               buf[ MAX_STRING_LENGTH ];
  char               result[ MAX_STRING_LENGTH * 2 ];
  char               arg1[ MAX_INPUT_LENGTH ];
  char               arg2[ MAX_INPUT_LENGTH ];
  char               arg3[ MAX_INPUT_LENGTH ];
  int                amt = 0;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if ( arg3[ 0 ] != '\0' ) {
    amt = is_number( arg3 ) ? atoi( arg3 ) : 1;
  } else {
    amt = 1;
  }

  if ( !strcmp( arg1, "info" ) ) {
    if ( CHECK_BIT( ch->act, PLR_QUESTOR ) ) {
      if ( ( !ch->questmob && !ch->questobj ) && ch->questgiver->short_descr != NULL ) {
        send_to_char( AT_YELLOW, "&YYour quest is ALMOST complete!\n\r", ch );
      } else if ( ch->questobj ) {
        sprintf( buf, "You are on a quest to find %s!\n\r", ch->questobj->short_descr );
        send_to_char( AT_WHITE, buf, ch );
        return;
      } else if ( ch->questmob ) {
        sprintf( buf, "You are on a quest to kill %s!\n\r", ch->questmob->short_descr );
        send_to_char( AT_WHITE, buf, ch );
        return;
      }
    } else {
      send_to_char( AT_WHITE, "You aren't currently on a quest.\n\r", ch );
    }

    return;
  }

  if ( !strcmp( arg1, "area" ) ) {
    if ( CHECK_BIT( ch->act, PLR_QUESTOR ) ) {
      sprintf( buf, "AREA: %s\n\r", room->area->name );
      send_to_char( AT_WHITE, buf, ch );
      return;
    } else {
      send_to_char( AT_WHITE, "You aren't currently on a quest.\n\r", ch );
    }

    return;
  }

  if ( !strcmp( arg1, "points" ) ) {
    sprintf( buf, "You have %d quest points.\n\r", ch->questpoints );
    send_to_char( AT_WHITE, buf, ch );
    return;
  } else if ( !strcmp( arg1, "time" ) ) {
    if ( !CHECK_BIT( ch->act, PLR_QUESTOR ) ) {
      send_to_char( AT_WHITE, "You aren't currently on a quest.\n\r", ch );

      if ( ch->nextquest > 1 ) {
        sprintf( buf, "There are %d minutes remaining until you can go on another quest.\n\r", ch->nextquest );
        send_to_char( AT_WHITE, buf, ch );
      } else if ( ch->nextquest == 1 ) {
        sprintf( buf, "There is less than a minute remaining until you can go on another quest.\n\r" );
        send_to_char( AT_WHITE, buf, ch );
      }
    } else if ( ch->countdown > 0 ) {
      sprintf( buf, "Time left for current quest: %d\n\r", ch->countdown );
      send_to_char( AT_WHITE, buf, ch );
    }

    return;
  }

  /* Checks for a Questmaster in the room */

  for ( questman = ch->in_room->people; questman; questman = questman->next_in_room ) {
    if ( IS_NPC( questman ) && CHECK_BIT( questman->act, ACT_QUESTMASTER ) ) {
      break;
    }
  }

  if ( !questman ) {
    send_to_char( AT_WHITE, "You can't do that here.\n\r", ch );
    return;
  }

  if ( questman->fighting != NULL ) {
    send_to_char( AT_WHITE, "Wait until the fighting stops.\n\r", ch );
    return;
  }

  /* Can have multiple questmasters, ch must report back to the one who gave quest */

  ch->questgiver = questman;

  /* List displaying items one can buy with quest points */

  if ( !strcmp( arg1, "list" ) ) {
    act( AT_WHITE, "$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM );
    act( AT_WHITE, "You ask $N for a list of quest items.", ch, NULL, questman, TO_CHAR );
    if ( IS_IMMORTAL(ch) ) {
      sprintf( result, "&Y[ &RVnum   &Y] &C[ &R%5s %10s &C] [ &R%44s &C]\n\r", "Lvl", "Points", "Item" );
    } else {
      sprintf( result, "&C[ &R%5s %10s &C] [ &R%44s &C]\n\r", "Lvl", "Points", "Item" );
    }

    for ( cnt = 0; quest_table[ cnt ].vnum != -1; cnt++ ) {
      if ( (rewardObj = get_obj_index( quest_table[ cnt ].vnum) ) ) {
        if ( IS_IMMORTAL(ch) ) {
          sprintf( buf, "&Y[ &R%-6d &Y] &C[ &R%5d &P%10d &C] &C[ &R%*s &C]\n\r",
                  rewardObj->vnum,
                  rewardObj->level,
                  quest_table[ cnt ].qp,
                  (int)( 44 + strlen( rewardObj->short_descr ) - strlen_wo_col( rewardObj->short_descr ) ),
                  capitalize(rewardObj->short_descr) );
        } else {
          sprintf( buf, "&C[ &R%5d &P%10d &C] &C[ &R%*s &C]\n\r",
                  rewardObj->level,
                  quest_table[ cnt ].qp,
                  (int)( 44 + strlen( rewardObj->short_descr ) - strlen_wo_col( rewardObj->short_descr ) ),
                  capitalize(rewardObj->short_descr) );
        }

        strcat( result, buf );
      }
    }

    send_to_char( AT_WHITE, result, ch );
    return;
  } else if ( !strcmp( arg1, "buy" ) ) {
    if ( arg2[ 0 ] == '\0' ) {
      send_to_char( AT_WHITE, "To buy an item, type 'QUEST BUY <item> <amount>'.\n\rAmount is optional.\n\r", ch );
      return;
    }

    for ( cnt = 0; quest_table[ cnt ].vnum != -1; cnt++ ) {
      if ( (rewardObj = get_obj_index( quest_table[ cnt ].vnum) ) ) {
        if ( is_name( ch, arg2, rewardObj->name ) ) {
          if ( ch->questpoints >= ( quest_table[ cnt ].qp * amt ) && amt > -1 ) {
            if ( rewardObj->level <= ch->level ) {
              ch->questpoints -= quest_table[ cnt ].qp * amt;

              while ( amt > 0 ) {
                obj = create_object( rewardObj, rewardObj->level );
                act( AT_WHITE, "$N gives $p to $n.", ch, obj, questman, TO_ROOM );
                act( AT_WHITE, "$N gives you $p.", ch, obj, questman, TO_CHAR );
                obj_to_char( obj, ch );
                amt--;
              }
            } else {
              sprintf( buf, "Sorry, %s, but you are too inexperienced to use that item.\n\r", ch->name );
              do_say( questman, buf );
              return;
            }
          } else {
            sprintf( buf, "Sorry, %s, but you don't have enough quest points for that.\n\r", ch->name );
            do_say( questman, buf );
            return;
          }

          break;
        }
      }
    }

    if ( obj == NULL ) {
      sprintf( buf, "I don't have that item, %s.\n\r", ch->name );
      do_say( questman, buf );
    }

    return;
  } else if ( !strcmp( arg1, "request" ) ) {
    act( AT_WHITE, "$n asks $N for a quest.", ch, NULL, questman, TO_ROOM );
    act( AT_WHITE, "You ask $N for a quest.", ch, NULL, questman, TO_CHAR );

    if ( CHECK_BIT( ch->act, PLR_QUESTOR ) ) {
      sprintf( buf, "But you're already on a quest!" );
      do_say( questman, buf );
      return;
    }

    if ( ch->nextquest > 0 ) {
      sprintf( buf, "You're very brave, %s, but let someone else have a chance.", ch->name );
      do_say( questman, buf );
      sprintf( buf, "Come back later." );
      do_say( questman, buf );
      return;
    }

    sprintf( buf, "Thank you, brave %s!", ch->name );
    do_say( questman, buf );

    generate_quest( ch, questman );

    if ( ch->questmob || ch->questobj ) {
      if ( ch->questmob ) {
        ch->countdown = number_range( 10, 30 );   /* time to complete quest */

      }

      /* Allow longer chance for an object quest */

      if ( ch->questobj ) {
        ch->countdown = number_range( 20, 45 );
      }

      SET_BIT( ch->act, PLR_QUESTOR );
      sprintf( buf, "You have %d minutes to complete this quest.", ch->countdown );
      do_say( questman, buf );
      sprintf( buf, "May the gods go with you!" );
      do_say( questman, buf );
    }

    return;
  } else if ( !strcmp( arg1, "complete" ) ) {
    act( AT_WHITE, "$n informs $N $e has completed $S quest.", ch, NULL, questman, TO_ROOM );
    act( AT_WHITE, "You inform $N you have completed $S quest.", ch, NULL, questman, TO_CHAR );

    /* Check if ch returned to correct QuestMaster */
    if ( ch->questgiver != questman ) {
      sprintf( buf, "I never sent you on a quest! Perhaps you're thinking of someone else." );
      do_say( questman, buf );
      return;
    }

    if ( CHECK_BIT( ch->act, PLR_QUESTOR ) ) {
      if ( ( !ch->questmob && !ch->questobj ) && ch->countdown > 0 ) {
        int pointreward, pracreward;

        pointreward = number_range( 10, 30 );

        sprintf( buf, "Congratulations on completing your quest!" );
        do_say( questman, buf );

        sprintf( buf, "As a reward, I am giving you %d quest points!", pointreward );
        do_say( questman, buf );

        /* 5% chance of getting between 1 and 6 practices :> */
        if ( chance( 5 ) ) {
          pracreward = number_range( 1, 6 );
          sprintf( buf, "You gain %d practices!\n\r", pracreward );
          send_to_char( AT_WHITE, buf, ch );
          ch->practice += pracreward;
        }

        REMOVE_BIT( ch->act, PLR_QUESTOR );
        ch->questgiver = NULL;
        ch->countdown  = 0;
        ch->questmob   = NULL;
        ch->questobj   = NULL;
        ch->nextquest  = 30; /* 30 */
        /*	ch->gold += reward;*/
        ch->questpoints += pointreward;

        return;
      }
      /* Look to see if ch has quest obj in inventory */
      else if ( ch->questobj && ch->countdown > 0 ) {
        bool obj_found = FALSE;

        for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
          obj_next = obj->next_content;

          if ( obj == ch->questobj ) {
            obj_found = TRUE;
            break;
          }
        }

        /* If ch returned without quest obj... */
        if ( !obj_found ) {
          sprintf( buf, "You haven't completed the quest yet, but there is still time!" );
          do_say( questman, buf );
          return;
        }

        {
          int pointreward, pracreward;

          /* Ch receives between 10 and 30 qp for completing quest :> */
          pointreward = number_range( 10, 30 );

          /* Player doesn't keep quest object */
          act( AT_WHITE, "You hand $p to $N.", ch, obj, questman, TO_CHAR );
          act( AT_WHITE, "$n hands $p to $N.", ch, obj, questman, TO_ROOM );

          sprintf( buf, "Congratulations on completing your quest!" );
          do_say( questman, buf );
          sprintf( buf, "As a reward, I am giving you %d quest points!", pointreward );
          do_say( questman, buf );

          /* 5% chance to get pracs.. */
          if ( chance( 5 ) ) {
            pracreward = number_range( 1, 6 );
            sprintf( buf, "You gain %d practices!\n\r", pracreward );
            send_to_char( AT_WHITE, buf, ch );
            ch->practice += pracreward;
          }

          REMOVE_BIT( ch->act, PLR_QUESTOR );
          ch->questgiver   = NULL;
          ch->countdown    = 0;
          ch->questmob     = NULL;
          ch->questobj     = NULL;
          ch->nextquest    = 30; /* 30 min till ch can quest again */
          ch->questpoints += pointreward;
          extract_obj( obj );
          return;
        }
      }
      /* Quest not complete, but still time left */
      else if ( ( ch->questmob || ch->questobj )
                && ch->countdown > 0 ) {
        sprintf( buf, "You haven't completed the quest yet, but there is still time!" );
        do_say( questman, buf );
        return;
      }
    }

    if ( ch->nextquest > 0 ) {
      sprintf( buf, "But you didn't complete your quest in time!" );
    } else {
      sprintf( buf, "You have to REQUEST a quest first, %s.", ch->name );
    }

    do_say( questman, buf );
    return;
  }

  if ( arg1[ 0 ] == '\0' ) {
    if ( CHECK_BIT( ch->act, PLR_QUESTOR ) ) {
      if ( ch->questobj ) {
        sprintf( buf, "You are on a quest to find %s!\n\r", ch->questobj->short_descr );
        send_to_char( AT_WHITE, buf, ch );
      } else if ( ch->questmob ) {
        sprintf( buf, "You are on a quest to kill %s!\n\r", ch->questmob->short_descr );
        send_to_char( AT_WHITE, buf, ch );
      }

      sprintf( buf, "You have %d quest points.\n\r", ch->questpoints );
      send_to_char( AT_WHITE, buf, ch );

      if ( ch->countdown > 0 ) {
        sprintf( buf, "Time left for current quest: %d\n\r", ch->countdown );
        send_to_char( AT_WHITE, buf, ch );
      }
    }

    if ( ch->nextquest > 1 ) {
      sprintf( buf, "There are %d minutes remaining until you can go on another quest.\n\r", ch->nextquest );
      send_to_char( AT_WHITE, buf, ch );
    } else if ( ch->nextquest == 1 ) {
      sprintf( buf, "There is less than a minute remaining until you can go on another quest.\n\r" );
      send_to_char( AT_WHITE, buf, ch );
    }

    send_to_char( AT_RED, "QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY AREA.\n\r", ch );
    send_to_char( AT_WHITE, "For more information, type 'HELP AUTOQUEST'.\n\r", ch );
    return;
  }
}

void generate_quest( CHAR_DATA * ch, CHAR_DATA * questman ) {
  CHAR_DATA * vsearch, * vsearch_next;
  CHAR_DATA * victim = NULL;
  /*  ROOM_INDEX_DATA *room;*/
  OBJ_INDEX_DATA * pObjIndex;
  OBJ_DATA * questitem;
  char       buf[ MAX_STRING_LENGTH ];
  long       mcounter;
  int        level_diff;

  /*  Randomly selects a mob from the world mob list. If you don't
     want a mob to be selected, make sure it is immune to summon.
     Or, you could add a new mob flag called ACT_NOQUEST. The mob
     is selected for both mob and obj quests, even tho in the obj
     quest the mob is not used. This is done to assure the level
     of difficulty for the area isn't too great for the player. */

  mcounter = 0;

  for ( vsearch = char_list; vsearch; vsearch = vsearch_next ) {
    vsearch_next = vsearch->next;

    if ( vsearch->deleted ) {
      continue;
    }

    level_diff = ch->level - vsearch->level;

    if ( ( level_diff <= 3 && level_diff > -5 )
         && ( IS_NPC( vsearch ) )
         && ( !CHECK_BIT( vsearch->in_room->area->area_flags, AREA_PROTOTYPE ) )
         && ( !CHECK_BIT( vsearch->in_room->area->area_flags, AREA_NO_QUEST ) )
         && ( vsearch->pIndexData->pShop == NULL )
         && ( vsearch->pIndexData->vnum != 1351 )
         && ( vsearch->pIndexData->vnum != 1350 )
         && ( ch->level <= vsearch->in_room->area->ulevel )
         && ( ch->level > vsearch->in_room->area->llevel )
         && ( !CHECK_BIT( vsearch->act, ACT_TRAIN ) )
         && ( !CHECK_BIT( vsearch->act, ACT_PRACTICE ) )
         && ( !CHECK_BIT( vsearch->in_room->room_flags, ROOM_SAFE ) )
         && ( !CHECK_BIT( vsearch->in_room->room_flags, ROOM_NO_OFFENSIVE ) ) ) {
      if ( number_range( 0, mcounter ) == 0 ) {
        victim = vsearch;
        mcounter++;
      }
    }
  }

  if ( !victim ) {
    sprintf( buf, "I'm sorry, but I don't have any quests for you at this time." );
    do_say( questman, buf );
    sprintf( buf, "Try again later." );
    do_say( questman, buf );
    ch->nextquest = 10;
    return;
  }

  if ( ( room = find_location( ch, victim->name ) ) == NULL ) {
    sprintf( buf, "I'm sorry, but I don't have any quests for you at this time." );
    do_say( questman, buf );
    sprintf( buf, "Try again later." );
    do_say( questman, buf );
    ch->nextquest = 10;
    return;
  }

  // 50% chance it will send the player on a 'recover item' quest.
  if ( chance( 50 ) ) {
    pObjIndex = get_obj_index( OBJ_VNUM_DUMMY );
    questitem = create_object( pObjIndex, ch->level );

    // pick a random item from the table
    struct quest_item_type item = quest_item_table[ RANDOM( quest_item_table ) ];

    questitem->name         = str_dup( item.name );

    sprintf( buf, "%s &R[QUEST]&X", item.short_desc);

    questitem->short_descr  = str_dup( buf );

    sprintf( buf, "%s &R[QUEST]&X", item.long_desc);

    questitem->description  = str_dup( buf );

    questitem->item_type    = ITEM_TRASH;
    questitem->wear_flags  ^= ITEM_TAKE;
    questitem->extra_flags ^= ITEM_GLOW;
    questitem->extra_flags ^= ITEM_NO_LOCATE;

    ch->questobj = questitem;

    if ( chance ( 50 ) ) {
      obj_to_room( questitem, room );
      sprintf( buf, "Vile pilferers have stolen %s from the royal treasury!", questitem->short_descr );
    } else {
      obj_to_char( questitem, victim );
      sprintf( buf, "An enemy of mine, %s, has stolen %s from the royal treasury!", victim->short_descr, questitem->short_descr );
    }

    do_say(questman, buf);

    do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");
    sprintf(buf, "Look in the general area of %s...", room->area->name);
    do_say(questman, buf);
    return;
  } else {
    switch ( number_range( 0, 1 ) ) {
      case 0:
        sprintf( buf, "An enemy of mine, %s, is making vile threats against the Oracle!", victim->short_descr );
        do_say( questman, buf );
        sprintf( buf, "This threat must be eliminated!" );
        do_say( questman, buf );
        break;

      case 1:
        sprintf( buf, "The realm's most heinous criminal, %s, has escaped from the dungeon!", victim->short_descr );
        do_say( questman, buf );
        sprintf( buf, "Since the escape, %s has murdered %d civilians!", victim->short_descr, number_range( 2, 20 ) );
        do_say( questman, buf );
        do_say( questman, "The penalty for this crime is death, and you are to deliver the sentence!" );
        break;
    }

    if ( room->name != NULL ) {
      sprintf(buf, "Seek %s out somewhere in the vicinity of %s!", victim->short_descr, room->area->name);
      do_say(questman, buf);
    }

    ch->questmob = victim;
  }

  return;
}

/* Called from update_handler() by pulse_area */

void quest_update( void ) {
  CHAR_DATA * ch, * ch_next;

  /* Added != NULL to the for loop.. hope it removes that crash... */
  for ( ch = char_list; ch != NULL; ch = ch_next ) {
    ch_next = ch->next;

    if ( IS_NPC( ch ) ) {
      continue;
    }

    /* i'll be amazed if the following change works, but i think it might */
    if ( ch->nextquest > 0 && ch->countdown <= 0 ) { /* added the countdown part -- Kj */

      if ( --ch->nextquest == 0 ) {
        send_to_char( AT_WHITE, "You may now quest again.\n\r", ch );
        return;
      }
    } else if ( CHECK_BIT( ch->act, PLR_QUESTOR ) ) {
      if ( ch->questmob
           && ch->questmob->deleted
           && ch->countdown > 1 ) {
        char buf[ MAX_INPUT_LENGTH ];
        sprintf( buf,
                 "%s tell you 'News has it that %s has been eliminated.'\n\r",
                 ch->questgiver->short_descr, ch->questmob->short_descr );
        send_to_char( AT_WHITE, buf, ch );
        sprintf( buf,
                 "%s tells you 'Thank you for attempting to kill %s.'\n\r",
                 ch->questgiver->short_descr, ch->questmob->short_descr );
        send_to_char( AT_WHITE, buf, ch );
        ch->reply = ch->questgiver;
        REMOVE_BIT( ch->act, PLR_QUESTOR );
        ch->questgiver = NULL;
        ch->questmob   = NULL;
        ch->countdown  = 0;
        ch->nextquest  = 15; /* 15 min until ch can quest again */
        return;
      }

      if ( --ch->countdown <= 0 ) {
        char buf[ MAX_STRING_LENGTH ];

        ch->nextquest = 30; /* 30 min until ch can quest again */
        sprintf( buf, "You have run out of time for your quest!\n\rYou may quest again in %d minutes.\n\r", ch->nextquest );
        send_to_char( AT_WHITE, buf, ch );
        REMOVE_BIT( ch->act, PLR_QUESTOR );

        if ( ch->questobj ) {
          ch->questobj->timer = 1; /* have obj_update
                                    * extract it.
                                    * -- Hannibal */
        }

        ch->questgiver = NULL;
        ch->countdown  = 0;
        ch->questmob   = NULL;
        ch->questobj   = NULL;
      }

      if ( ch->countdown > 0 && ch->countdown < 6 ) {
        send_to_char( AT_WHITE, "Better hurry, you're almost out of time for your quest!\n\r", ch );
        return;
      }
    }
  }

  return;
}

bool check_for_worn( CHAR_DATA * ch, int vnum ) {
  OBJ_DATA * obj;

  for ( obj = ch->carrying; obj; obj = obj->next_content ) {
    if ( ( obj->pIndexData->vnum == vnum )
         && obj->wear_loc != WEAR_NONE ) {
      return TRUE;
    }
  }

  return FALSE;
}
