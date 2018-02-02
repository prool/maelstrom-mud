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

#include "merc.h"
#include <string.h>

/*
 * Globals
 */
SOCIAL_DATA * social_first;
SOCIAL_DATA * social_last;


void do_socials( CHAR_DATA * ch, char * argument ) {
  char buf[ MAX_STRING_LENGTH ];
  char buf1[ MAX_STRING_LENGTH ];
  int           col;
  SOCIAL_DATA * pSocial;

  buf1[ 0 ] = '\0';
  col       = 0;

  for ( pSocial = social_first; pSocial; pSocial = pSocial->next ) {
    sprintf( buf, "%-12s", pSocial->name );
    strcat( buf1, buf );

    if ( ++col % 6 == 0 ) {
      strcat( buf1, "\n\r" );
    }
  }

  if ( col % 6 != 0 ) {
    strcat( buf1, "\n\r" );
  }

  send_to_char( C_DEFAULT, buf1, ch );
  return;
}

void social_sort( SOCIAL_DATA * pSocial ) {
  SOCIAL_DATA * fSocial;

  // initialize list with first node and return (nothing left to do)
  if ( !social_first ) {
    social_first = pSocial;
    social_last  = pSocial;

    return;
  }

  // if the new node's name comes alphabetically before the old one, then
  // place it at the beginning of the list and return (cause we done)
  if ( strcmp( pSocial->name, social_first->name ) < 0 ) {
    pSocial->next = social_first;
    social_first  = pSocial;

    return;
  }

  // traverse the list and find the appropriate place to insert the new node
  for ( fSocial = social_first; fSocial; fSocial = fSocial->next ) {
    // if the new node's name comes alphabetically before the next one, then
    // slot it in between the current node and the next one
    if ( fSocial->next && strcmp( pSocial->name, fSocial->next->name ) < 0 ) {
      pSocial->next = fSocial->next;
      fSocial->next = pSocial;

      return;
    }
  }

  // end of the line, tack the node on the end of the list
  social_last->next = pSocial;
  social_last       = pSocial;
  pSocial->next     = NULL;

  return;
}

void load_socials( void ) {
  SOCIAL_DATA * pSocial;
  json_t      * obj;
  json_error_t  error;

  obj = json_load_file( SOCIAL_FILE, 0, &error );

  if ( !obj ) {
    return;
  }

  void * iter = json_object_iter( obj );

  while ( iter ) {
    pSocial       = new_social_index();
    pSocial->name = str_dup(json_object_iter_key( iter ));

    json_unpack( json_object_iter_value( iter ),
                 "{s:{s:s, s:s}, s:{s:s, s:s, s:s}, s:{s:s, s:s}}",
                 "no_arg",
                 "char", &pSocial->char_no_arg,
                 "others", &pSocial->others_no_arg,
                 "found",
                 "char", &pSocial->char_found,
                 "others", &pSocial->others_found,
                 "vict", &pSocial->vict_found,
                 "auto",
                 "char", &pSocial->char_auto,
                 "others", &pSocial->others_auto
                 );

    social_sort( pSocial );

    iter = json_object_iter_next( obj, iter );
  }

  return;
}

void save_social() {
  SOCIAL_DATA * pSocial;
  json_t      * obj = json_object();

  for ( pSocial = social_first; pSocial; pSocial = pSocial->next ) {
    json_object_set( obj, pSocial->name, json_pack(
                       "{s:{s:s, s:s}, s:{s:s, s:s, s:s}, s:{s:s, s:s}}",
                       "no_arg",
                       "char", pSocial->char_no_arg,
                       "others", pSocial->others_no_arg,
                       "found",
                       "char", pSocial->char_found,
                       "others", pSocial->others_found,
                       "vict", pSocial->vict_found,
                       "auto",
                       "char", pSocial->char_auto,
                       "others", pSocial->others_auto
                       ) );
  }

  if ( json_dump_file( obj, SOCIAL_FILE, JSON_SORT_KEYS | JSON_INDENT( 2 ) ) == -1 ) {
    bug( "Save_social: fopen", 0 );
    perror( SOCIAL_FILE );
  }

  return;
}

void free_social_index( SOCIAL_DATA * pSocial ) {
  free_string( pSocial->name );
  free_string( pSocial->char_no_arg );
  free_string( pSocial->others_no_arg );
  free_string( pSocial->char_found );
  free_string( pSocial->others_found );
  free_string( pSocial->vict_found );
  free_string( pSocial->char_auto );
  free_string( pSocial->others_auto );
  top_social--;

  free_mem( pSocial, sizeof( *pSocial ) );

  return;
}

SOCIAL_DATA * new_social_index( void ) {
  SOCIAL_DATA * pSocial;

  pSocial =  alloc_perm( sizeof( *pSocial ) );

  pSocial->next          =       NULL;
  pSocial->name          =       &str_empty[ 0 ];
  pSocial->char_no_arg   =       &str_empty[ 0 ];
  pSocial->others_no_arg =       &str_empty[ 0 ];
  pSocial->char_found    =       &str_empty[ 0 ];
  pSocial->others_found  =       &str_empty[ 0 ];
  pSocial->vict_found    =       &str_empty[ 0 ];
  pSocial->char_auto     =       &str_empty[ 0 ];
  pSocial->others_auto   =       &str_empty[ 0 ];
  top_social++;

  return pSocial;
}
