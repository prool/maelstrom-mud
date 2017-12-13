/*****************************************************************************
* Room affect stuff.                                                        *
* Put skills that use room affects in here.                                 *
* -- Hannibal                                                               *
*****************************************************************************/
#define unix 1
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"

/* Globals */
ROOM_AFFECT_DATA * raffect_free;
POWERED_DATA     * pd_free;

/*
 * Slap a rAffect on a room and update ch->powered.
 */
void raffect_to_room( ROOM_INDEX_DATA * room, CHAR_DATA * ch, ROOM_AFFECT_DATA * raf ) {
  ROOM_AFFECT_DATA * raf_new;
  POWERED_DATA     * pd;

  if ( !raffect_free ) {
    raf_new = alloc_perm( sizeof( *raf_new ) );
  } else {
    raf_new      = raffect_free;
    raffect_free = raffect_free->next;
  }

  if ( !pd_free ) {
    pd = alloc_perm( sizeof( *pd ) );
  } else {
    pd      = pd_free;
    pd_free = pd_free->next;
  }

  *raf_new      = *raf;
  raf_new->next = room->rAffect;
  room->rAffect = raf_new;
  pd->room      = room;
  pd->raf       = raf_new;
  pd->next      = ch->powered;
  pd->type      = raf_new->type;
  pd->cost      = MANA_COST( ch, pd->type );

  if ( ch->race == RACE_ELF ) {
    pd->cost *= .75;
  }

  ch->powered = pd;
  obj_to_room( raf->material, room );

  if ( raf_new->location != ROOM_NONE ) {
    TOGGLE_BIT( room->room_flags, raf->location );
  }

  return;

}

/*
 * Remove and delete a rAffect and update ch->powered.
 */
void raffect_remove( ROOM_INDEX_DATA * room, CHAR_DATA * ch, ROOM_AFFECT_DATA * raf ) {
  ROOM_AFFECT_DATA * rAf, * prAf;
  POWERED_DATA     * pd, * ppd;
  char               buf[ MAX_STRING_LENGTH ];

  if ( !room->rAffect ) {
    bug( "rAffect_remove: no affect.", 0 );
    return;
  }

  if ( raf->location != ROOM_NONE ) {
    TOGGLE_BIT( room->room_flags, raf->location );
  }

  sprintf( buf, "%s", skill_table[ raf->type ].room_msg_off );

  if ( ch->in_room == room ) {
    act( AT_CYAN, buf, ch, NULL, NULL, TO_ROOM );
  } else if ( room->people ) {
    act( AT_CYAN, buf, room->people, NULL, NULL, TO_ROOM );
  }

  ppd = NULL;

  for ( pd = ch->powered; pd; ppd = pd, pd = pd->next ) {
    if ( pd->room == room && pd->raf == raf ) {
      if ( !ppd ) {
        ch->powered = pd->next;
      } else {
        ppd->next = pd->next;
      }

      if ( ch->mana < pd->cost ) {
        sprintf( buf, "%s %s.\n\r", skill_table[ raf->type ].msg_off,
                 raf->room->name );
        send_to_char( AT_CYAN, buf, ch );
      }

      break;

    }
  }

  prAf = NULL;

  for ( rAf = room->rAffect; rAf; prAf = rAf, rAf = rAf->next ) {
    if ( raf == rAf ) {
      if ( !prAf ) {
        room->rAffect = rAf->next;
      } else {
        prAf->next = rAf->next;
      }

      break;
    }
  }

  extract_obj( raf->material );
  free_mem( raf, sizeof( *raf ) );
  free_mem( pd, sizeof( *pd ) );

  return;

}

/*
 * Check if room is rAffected by something.
 */
bool is_raffected( ROOM_INDEX_DATA * room, int sn ) {
  ROOM_AFFECT_DATA * raf;

  for ( raf = room->rAffect; raf; raf = raf->next ) {
    if ( raf->type == sn ) {
      return TRUE;
    }
  }

  return FALSE;

}

/*
 * To see what you're powering ( imms see room's rAffects as well ).
 */
void do_raffect( CHAR_DATA * ch, char * argument ) {
  POWERED_DATA     * pd;
  ROOM_AFFECT_DATA * raf;
  char               buf[ MAX_INPUT_LENGTH ];

  if ( IS_NPC( ch ) ) {
    return;
  }

  if ( !ch->powered ) {
    send_to_char( AT_CYAN, "You are not powering any room affects.\n\r", ch );
  }

  for ( pd = ch->powered; pd; pd = pd->next ) {
    sprintf( buf, "%s&w, &cMana Cost&w: &R%d&w, &cRoom&w: &W%s&w.\n\r",
             skill_table[ pd->type ].name,
             pd->cost,
             pd->room->name );
    buf[ 0 ] = UPPER( buf[ 0 ] );
    send_to_char( AT_WHITE, buf, ch );

  }

  if ( ch->level >= L_APP ) {
    send_to_char( AT_CYAN, "This room is affected by:\n\r", ch );

    for ( raf = ch->in_room->rAffect; raf; raf = raf->next ) {
      sprintf( buf, "%s &cpowered by&w: &W%s&w.\n\r",
               skill_table[ raf->type ].name,
               raf->powered_by->name );
      buf[ 0 ] = UPPER( buf[ 0 ] );
      send_to_char( AT_WHITE, buf, ch );
    }
  }

  return;

}

/*
 * Remove all rAffects ch is powering. (mostly for quitting)
 */
void raffect_remall( CHAR_DATA * ch ) {
  POWERED_DATA * pd, * pd_next;

  for ( pd = ch->powered; pd; pd = pd_next ) {
    if ( !pd ) {
      return;
    }

    pd_next = pd->next;
    raffect_remove( pd->room, ch, pd->raf );

  }

  return;

}

/*
 * Toggle a room's rAffect location. (for asave_area)
 */
void toggle_raffects( ROOM_INDEX_DATA * room ) {
  ROOM_AFFECT_DATA * raf;

  for ( raf = room->rAffect; raf; raf = raf->next ) {
    if ( !raf ) {
      break;
    }

    if ( raf->location != ROOM_NONE ) {
      TOGGLE_BIT( room->room_flags, raf->location );
    }

  }

  return;

}

/*
 * Locate or turn off a specific type of rAffect skill.
 */
void loc_off_raf( CHAR_DATA * ch, int gsn_raf, bool rOff ) {
  POWERED_DATA * pd;
  char           buf[ MAX_STRING_LENGTH ];
  bool           found = FALSE;

  if ( !rOff ) { /* Locate a rAffect of type gsn_raf. */
    for ( pd = ch->powered; pd; pd = pd->next ) {
      if ( pd->type == gsn_raf ) {
        sprintf( buf, "%s&w, &W%s&w; &cCost&w: &R%d&w.\n\r",
                 skill_table[ gsn_raf ].name, pd->room->name, pd->cost );
        send_to_char( AT_WHITE, buf, ch );
        found = TRUE;
      }
    }

    if ( !found ) {
      sprintf( buf, "You are not sustaining any %s&w.\n\r",
               skill_table[ gsn_raf ].name );
      send_to_char( AT_CYAN, buf, ch );
    }

    return;

  }

  /* Delete a rAffect of gsn_raf from room. */
  if ( !is_raffected( ch->in_room, gsn_raf ) ) {
    sprintf( buf, "There is no %s &cin this room.\n\r",
             skill_table[ gsn_raf ].name );
    send_to_char( AT_CYAN, buf, ch );

    return;

  }

  for ( pd = ch->powered; pd; pd = pd->next ) {
    if ( !pd ) {
      break;
    }

    if ( pd->type == gsn_raf ) {
      found = TRUE;

      if ( pd->room == ch->in_room ) {
        sprintf( buf, "%s\n\r", skill_table[ gsn_raf ].noun_damage );
        send_to_char( AT_WHITE, buf, ch );
        raffect_remove( ch->in_room, ch, pd->raf );
        return;
      }
    }
  }

  if ( !found ) {
    sprintf( buf, "You are not sustaining any %s&w.\n\r",
             skill_table[ gsn_raf ].name );
    return;
  }

  sprintf( buf, "You are not powering the %s &cin this room.\n\r",
           skill_table[ gsn_raf ].name );
  send_to_char( AT_CYAN, buf, ch );

  return;

}
