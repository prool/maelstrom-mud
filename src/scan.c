#define unix 1
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"

int scan_room( CHAR_DATA * ch, const ROOM_INDEX_DATA * room, char * buf ) {
  CHAR_DATA * target       = room->people;
  int         number_found = 0;

	for ( target = room->people; target != NULL; target = target->next_in_room ) {
    if ( !IS_NPC( target ) && !( target->desc ) ) {
      continue;
    }

    // players shouldn't see themselves
    if( ch == target ) {
      continue;
    }

    if ( can_see( ch, target ) ) {
      strcat( buf, " - " );
      strcat( buf, visible_name( target, ch, FALSE ) );
      strcat( buf, "\n\r" );
      number_found++;
    }
  }

  return number_found;
}

void do_scan( CHAR_DATA * ch, char * argument ) {
  EXIT_DATA       * pexit;
  ROOM_INDEX_DATA * room;
  char              buf[ MAX_STRING_LENGTH ];
  int               dir;
  int               distance;

  // can't scan if you're blind
  if ( !check_blind( ch ) ) {
    return;
  }

  sprintf( buf, "Right here you see:\n\r" );
  scan_room( ch, ch->in_room, buf );
  send_to_char( AT_BLUE, buf, ch );

  for ( dir = 0; dir < MAX_DIR; dir++ ) {
    room = ch->in_room;

    for ( distance = 1; distance < 4; distance++ ) {
      pexit = room->exit[ dir ];

      if ( ( pexit == NULL ) || ( pexit->to_room == NULL ) || ( IS_SET( pexit->exit_info, EX_CLOSED ) ) ) {
        break;
      }

      sprintf( buf, "%d %s from here you see:\n\r", distance, direction_table[ dir ].name );

      if ( scan_room( ch, pexit->to_room, buf ) ) {
        send_to_char( AT_WHITE, buf, ch );
      }

      room = pexit->to_room;
    }
  }
}
