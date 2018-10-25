/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/*
 * Local functions
 */
bool is_note_to( CHAR_DATA * ch, NOTE_DATA * pnote );
void note_attach( CHAR_DATA * ch );
void note_remove( CHAR_DATA * ch, NOTE_DATA * pnote );
void talk_channel( CHAR_DATA * ch, char * argument, int channel, const char * verb );
void note_delete( NOTE_DATA * pnote );


void note_delete( NOTE_DATA * pnote ) {
  NOTE_DATA * prev;

  if ( pnote == note_list ) {
    note_list = pnote->next;
  } else {
    for ( prev = note_list; prev; prev = prev->next ) {
      if ( prev->next == pnote ) {
        break;
      }
    }

    if ( !prev ) {
      bug( "Note_delete: no note.", 0 );
      return;
    }

    prev->next = pnote->next;
  }

  free_string( pnote->text );
  free_string( pnote->subject );
  free_string( pnote->to_list );
  free_string( pnote->date );
  free_string( pnote->sender );

  /*  pnote->next = note_free;
     note_free = pnote;*/
  free_mem( pnote, sizeof( *pnote ) );
}

void note_cleanup( void ) {
  NOTE_DATA * pnote;
  NOTE_DATA * pnote_next;
  FILE      * fp;

  for ( pnote = note_list; pnote && pnote->date_stamp + 604800 < current_time; pnote = pnote_next ) {
    pnote_next = pnote->next;

    if ( pnote->protected ) {
      continue;
    }

    note_delete( pnote );
  }

  fclose( fpReserve );

  if ( !( fp = fopen( NOTE_FILE, "w" ) ) ) {
    perror( NOTE_FILE );
  } else {
    for ( pnote = note_list; pnote; pnote = pnote->next ) {
      fprintf( fp, "Sender  %s~\n", pnote->sender );
      fprintf( fp, "Date    %s~\n", pnote->date );
      fprintf( fp, "Stamp   %ld\n", pnote->date_stamp );
      fprintf( fp, "To      %s~\n", pnote->to_list );
      fprintf( fp, "Subject %s~\n", pnote->subject );
      fprintf( fp, "Protect %d\n",  pnote->protected );
      fprintf( fp, "Board   %d\n",  pnote->on_board );
      fprintf( fp, "Text\n%s~\n\n", pnote->text );
    }

    fclose( fp );
  }

  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

bool is_note_to( CHAR_DATA * ch, NOTE_DATA * pnote ) {
  if ( !str_cmp( ch->name, pnote->sender ) ) {
    return TRUE;
  }

  if ( is_name( NULL, "all", pnote->to_list ) ) {
    return TRUE;
  }

  if ( ( get_trust( ch ) >= LEVEL_IMMORTAL )
       && ( (   is_name( NULL, "immortal", pnote->to_list )
                || is_name( NULL, "immortals", pnote->to_list )
                || is_name( NULL, "imm",       pnote->to_list )
                || is_name( NULL, "immort",    pnote->to_list ) ) ) ) {

    return TRUE;
  }

  if ( ( get_trust( ch ) > L_CON /* || IS_CODER( ch )*/ ) &&
       is_name( NULL, "council", pnote->to_list ) ) {
    return TRUE;
  }

  if ( is_name( NULL, "IMP", pnote->to_list ) && ch->level == L_IMP ) {
    return TRUE;
  }

  if ( is_name( NULL, ch->name, pnote->to_list ) ) {
    return TRUE;
  }

  return FALSE;
}

void note_attach( CHAR_DATA * ch ) {
  NOTE_DATA * pnote;

  if ( ch->pnote ) {
    return;
  }

  if ( !note_free ) {
    pnote = alloc_perm( sizeof( *ch->pnote ) );
  } else {
    pnote     = note_free;
    note_free = note_free->next;
  }

  pnote->next      = NULL;
  pnote->sender    = str_dup( ch->name );
  pnote->date      = str_dup( "" );
  pnote->to_list   = str_dup( "" );
  pnote->subject   = str_dup( "" );
  pnote->text      = str_dup( "" );
  pnote->protected = FALSE;
  pnote->on_board  = 0;
  ch->pnote        = pnote;
  return;
}

void note_remove( CHAR_DATA * ch, NOTE_DATA * pnote ) {
  FILE      * fp;
  NOTE_DATA * prev;
  char      * to_list;
  char        to_new[ MAX_INPUT_LENGTH ];
  char        to_one[ MAX_INPUT_LENGTH ];

  /*
   * Build a new to_list.
   * Strip out this recipient.
   */
  to_new[ 0 ] = '\0';
  to_list     = pnote->to_list;

  while ( *to_list != '\0' ) {
    to_list = one_argument( to_list, to_one );

    if ( to_one[ 0 ] != '\0' && str_cmp( ch->name, to_one ) ) {
      strcat( to_new, " " );
      strcat( to_new, to_one );
    }
  }

  /*
   * Just a simple recipient removal?
   */
  if ( str_cmp( ch->name, pnote->sender ) && to_new[ 0 ] != '\0' &&
       get_trust( ch ) < L_DEM ) {
    free_string( pnote->to_list );
    pnote->to_list = str_dup( to_new + 1 );
    return;
  }

  /*
   * Remove note from linked list.
   */
  if ( pnote == note_list ) {
    note_list = pnote->next;
  } else {
    for ( prev = note_list; prev; prev = prev->next ) {
      if ( prev->next == pnote ) {
        break;
      }
    }

    if ( !prev ) {
      bug( "Note_remove: pnote not found.", 0 );
      return;
    }

    prev->next = pnote->next;
  }

  free_string( pnote->text );
  free_string( pnote->subject );
  free_string( pnote->to_list );
  free_string( pnote->date );
  free_string( pnote->sender );

  /*    pnote->next	= note_free;
      note_free	= pnote;*/
  free_mem( pnote, sizeof( *pnote ) );

  /*
   * Rewrite entire list.
   */
  fclose( fpReserve );

  if ( !( fp = fopen( NOTE_FILE, "w" ) ) ) {
    perror( NOTE_FILE );
  } else {
    for ( pnote = note_list; pnote; pnote = pnote->next ) {
      fprintf( fp, "Sender  %s~\n", pnote->sender );
      fprintf( fp, "Date    %s~\n", pnote->date );
      fprintf( fp, "Stamp   %ld\n", pnote->date_stamp );
      fprintf( fp, "To      %s~\n", pnote->to_list );
      fprintf( fp, "Subject %s~\n", pnote->subject );
      fprintf( fp, "Protect %d\n",  pnote->protected );
      fprintf( fp, "Board   %d\n",  pnote->on_board );
      fprintf( fp, "Text\n%s~\n\n", pnote->text );
    }

    fclose( fp );
  }

  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

void do_note( CHAR_DATA * ch, char * argument ) {
  NOTE_DATA * pnote;
  CHAR_DATA * to_ch;
  char        buf[ MAX_STRING_LENGTH ];
  char        buf1[ MAX_STRING_LENGTH * 7 ];
  char        arg[ MAX_INPUT_LENGTH ];
  int         vnum;
  int         anum;

  /*    if ( IS_NPC( ch ) )
      return;*/

  argument = one_argument( argument, arg );
  smash_tilde( argument );

  if ( arg[ 0 ] == '\0' ) {
    if ( !IS_NPC( ch ) ) {
      do_note( ch, "read" );
    }

    return;
  }

  if ( !str_cmp( arg, "list" ) ) {
    char arg1[ MAX_STRING_LENGTH ];
    char arg2[ MAX_STRING_LENGTH ];
    int  fn = 0;
    int  ln = 0;

    if ( IS_NPC( ch ) ) {
      return;
    }

    vnum      = 0;
    buf1[ 0 ] = '\0';

    if ( argument[ 0 ] != '\0' ) {
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );
      fn       = is_number( arg1 ) ? atoi( arg1 ) : 0;
      ln       = is_number( arg2 ) ? atoi( arg2 ) : 0;

      if ( ( fn == 0 && ln == 0 ) || ( fn < 1 ) || ( ln < 0 )
           || ( ln < fn ) ) {
        send_to_char( AT_DGREEN, "Invalid note range.\n\r", ch );
        return;
      }
    }

    for ( pnote = note_list; pnote; pnote = pnote->next ) {
      if ( ( is_note_to( ch, pnote ) && vnum >= fn && vnum <= ln )
           || ( fn == 0 && ln == 0 && is_note_to( ch, pnote ) ) ) {
        sprintf( buf, "&G[%3d%s%s] %s: %s\n\r",
                 vnum,
                 ( pnote->date_stamp > ch->last_note
                   && str_cmp( pnote->sender, ch->name ) ) ? "N" : " ",
                 ( get_trust( ch ) > L_CON /* || IS_CODER(ch)*/ ) ?
                 pnote->protected ? "P" : " " : "",
                 pnote->sender, pnote->subject );
        strcat( buf1, buf );
      }

      if ( is_note_to( ch, pnote ) ) {
        vnum++;
      }

    }

    send_to_char( AT_GREEN, buf1, ch );
    return;
  }

  if ( !str_cmp( arg, "read" ) ) {
    bool fAll;

    if ( IS_NPC( ch ) ) {
      return;
    }
    /*	if ( !str_cmp( argument, "all" ) )
       {
       fAll = TRUE;
       anum = 0;
       }*/
    else if ( argument[ 0 ] == '\0' || !str_prefix( argument, "next" ) ) {
      /* read next unread note */
      vnum      = 0;
      buf1[ 0 ] = '\0';

      for ( pnote = note_list; pnote; pnote = pnote->next ) {
        if ( is_note_to( ch, pnote )
             && str_cmp( ch->name, pnote->sender )
             && ch->last_note < pnote->date_stamp ) {
          sprintf( buf, "[%3d] %s: %s\n\r&G%s\n\rTo: %s\n\r",
                   vnum,
                   pnote->sender,
                   pnote->subject,
                   pnote->date,
                   pnote->to_list );
          strcat( buf1, buf );
          strcat( buf1, pnote->text );
          ch->last_note = UMAX( ch->last_note, pnote->date_stamp );
          send_to_char( AT_GREEN, buf1, ch );
          return;
        } else if ( is_note_to( ch, pnote ) ) {
          vnum++;
        }
      }

      send_to_char( AT_DGREEN, "You have no unread notes.\n\r", ch );
      return;
    } else if ( is_number( argument ) ) {
      fAll = FALSE;
      anum = atoi( argument );
    } else {
      send_to_char( AT_DGREEN, "Note read which number?\n\r", ch );
      return;
    }

    vnum      = 0;
    buf1[ 0 ] = '\0';

    for ( pnote = note_list; pnote; pnote = pnote->next ) {
      if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) ) {
        sprintf( buf, "[%3d] %s: %s\n\r&G%s\n\rTo: %s\n\r",
                 vnum - 1,
                 pnote->sender,
                 pnote->subject,
                 pnote->date,
                 pnote->to_list );
        strcat( buf1, buf );
        strcat( buf1, pnote->text );

        if ( !fAll ) {
          send_to_char( AT_GREEN, buf1, ch );
        } else {
          strcat( buf1, "\n\r" );
        }

        ch->last_note = UMAX( ch->last_note, pnote->date_stamp );

        if ( !fAll ) {
          return;
        }
      }
    }

    if ( !fAll ) {
      send_to_char( AT_DGREEN, "No such note.\n\r", ch );
    } else {
      send_to_char( AT_GREEN, buf1, ch );
    }

    return;
  }

  if ( !str_cmp( arg, "+" ) ) {
    note_attach( ch );
    strcpy( buf, ch->pnote->text );

    if ( strlen( buf ) + strlen( argument ) >= MAX_STRING_LENGTH - 100 ) {
      send_to_char( AT_DGREEN, "Note too long.\n\r", ch );
      return;
    }

    strcat( buf, argument );
    strcat( buf, "\n\r" );
    free_string( ch->pnote->text );
    ch->pnote->text = str_dup( buf );
    send_to_char( AT_WHITE, "Ok.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "write" ) ) {
    if ( IS_NPC( ch ) ) {
      return;
    }

    note_attach( ch );
    string_append( ch, &ch->pnote->text );
    return;
  }

  if ( !str_cmp( arg, "subject" ) ) {
    note_attach( ch );
    free_string( ch->pnote->subject );
    ch->pnote->subject = str_dup( argument );
    send_to_char( AT_WHITE, "Ok.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "to" ) ) {
    note_attach( ch );
    free_string( ch->pnote->to_list );
    ch->pnote->to_list = str_dup( argument );
    send_to_char( AT_WHITE, "Ok.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "clear" ) ) {
    if ( ch->pnote ) {
      free_string( ch->pnote->text );
      free_string( ch->pnote->subject );
      free_string( ch->pnote->to_list );
      free_string( ch->pnote->date );
      free_string( ch->pnote->sender );

      /*	    ch->pnote->next	= note_free;
          note_free		= ch->pnote;*/
      free_mem( ch->pnote, sizeof( *ch->pnote ) );
      ch->pnote = NULL;
    }

    send_to_char( AT_WHITE, "Ok.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "show" ) ) {
    if ( IS_NPC( ch ) ) {
      return;
    }

    if ( !ch->pnote ) {
      send_to_char( AT_DGREEN, "You have no note in progress.\n\r", ch );
      return;
    }

    sprintf( buf, "%s: %s\n\r&GTo: %s\n\r",
             ch->pnote->sender,
             ch->pnote->subject,
             ch->pnote->to_list );
    send_to_char( AT_GREEN, buf, ch );
    send_to_char( AT_GREEN, ch->pnote->text, ch );
    return;
  }

  if ( !str_cmp( arg, "post" ) || !str_prefix( arg, "send" ) ) {
    FILE     * fp;
    char     * strtime;

    if ( !ch->pnote ) {
      send_to_char( AT_DGREEN, "You have no note in progress.\n\r", ch );
      return;
    }

    if ( !str_cmp( ch->pnote->to_list, "" ) ) {
      send_to_char( AT_DGREEN,
                    "You need to provide a recipient (name, all, or immortal).\n\r",
                    ch );
      return;
    }

    if ( !str_cmp( ch->pnote->subject, "" ) ) {
      send_to_char( AT_DGREEN, "You need to provide a subject.\n\r", ch );
      return;
    }

    ch->pnote->on_board = 0;

    if ( IS_NPC( ch ) && ch->pnote->on_board == 0 ) {
      return;
    }

    ch->pnote->next                  = NULL;
    strtime                          = ctime( &current_time );
    strtime[ strlen( strtime ) - 1 ] = '\0';
    free_string( ch->pnote->date );
    ch->pnote->date       = str_dup( strtime );
    ch->pnote->date_stamp = current_time;

    if ( !note_list ) {
      note_list = ch->pnote;
    } else {
      for ( pnote = note_list; pnote->next; pnote = pnote->next ) {
        ;
      }

      pnote->next = ch->pnote;
    }

    pnote     = ch->pnote;
    ch->pnote = NULL;

    fclose( fpReserve );

    if ( !( fp = fopen( NOTE_FILE, "a" ) ) ) {
      perror( NOTE_FILE );
    } else {
      fprintf( fp, "Sender  %s~\n", pnote->sender );
      fprintf( fp, "Date    %s~\n", pnote->date );
      fprintf( fp, "Stamp   %ld\n", pnote->date_stamp );
      fprintf( fp, "To      %s~\n", pnote->to_list );
      fprintf( fp, "Subject %s~\n", pnote->subject );
      fprintf( fp, "Protect %d\n",  pnote->protected );
      fprintf( fp, "Board   %d\n",  pnote->on_board );
      fprintf( fp, "Text\n%s~\n\n", pnote->text );
      fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );

    send_to_char( AT_WHITE, "Ok.\n\r", ch );

    for ( to_ch = char_list; to_ch; to_ch = to_ch->next ) {
      if ( !to_ch->in_room || to_ch->deleted ) {
        continue;
      }

      if ( is_note_to( to_ch, pnote ) && to_ch != ch ) {
        send_to_char( C_DEFAULT, "New note.\n\r", to_ch );
      }
    }

    return;
  }

  if ( !str_cmp( arg, "remove" ) ) {
    if ( IS_NPC( ch ) ) {
      return;
    }

    if ( !is_number( argument ) ) {
      send_to_char( AT_DGREEN, "Note remove which number?\n\r", ch );
      return;
    }

    anum = atoi( argument );
    vnum = 0;

    for ( pnote = note_list; pnote; pnote = pnote->next ) {
      if ( is_note_to( ch, pnote ) && vnum++ == anum ) {
        note_remove( ch, pnote );
        send_to_char( AT_WHITE, "Ok.\n\r", ch );
        return;
      }
    }

    send_to_char( AT_DGREEN, "No such note.\n\r", ch );
    return;
  }

  /*
   * "Permanent" note flag.
   * -- Altrag
   */
  if ( !str_cmp( arg, "protect" ) ) {
    if ( IS_NPC( ch ) ) {
      return;
    }

    if ( get_trust( ch ) < L_CON /* && !IS_CODER( ch )*/ ) {
      send_to_char( AT_DGREEN, "Huh?  Type 'help note' for usage.\n\r", ch );
      return;
    }

    if ( argument[ 0 ] == '\0' || !is_number( argument ) ) {
      send_to_char( AT_DGREEN, "Syntax:  note protect <#>\n\r", ch );
      return;
    }

    anum = atoi( argument );
    vnum = 0;

    for ( pnote = note_list; pnote; pnote = pnote->next ) {
      if ( is_note_to( ch, pnote ) && vnum++ == anum ) {
        if ( pnote->protected ) {
          pnote->protected = FALSE;
        } else {
          pnote->protected = TRUE;
        }

        note_cleanup();
        send_to_char( AT_WHITE, "Ok.\n\r", ch );
        return;
      }
    }

    send_to_char( AT_WHITE, "No such note.\n\r", ch );
    return;
  }

  send_to_char( AT_DGREEN, "Huh?  Type 'help note' for usage.\n\r", ch );
  return;
}
