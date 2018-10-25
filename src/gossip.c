#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <wsclient/wsclient.h>

#include "merc.h"
#include "gossip.h"

/*
 * Globals
 */
wsclient *gossip_ws;

void gossip_connect() {
  // initialize new wsclient * using specified URI
  gossip_ws = libwsclient_new("wss://gossip.haus/socket");

  if(!gossip_ws) {
    log_string( "Unable to connect to Gossip" );
    wiznet( "Unable to connect to Gossip", NULL, NULL, WIZ_GENERAL, 0, 0 );
    return;
  }

  // set callback functions for this client
  libwsclient_onopen(gossip_ws, &gossip_onopen);
  libwsclient_onmessage(gossip_ws, &gossip_onmessage);
  libwsclient_onerror(gossip_ws, &gossip_onerror);
  libwsclient_onclose(gossip_ws, &gossip_onclose);

  // starts run thread.
  libwsclient_run(gossip_ws);
}

void gossip_send(char * player, char * message) {
  char * payload;
  json_t * obj;

  obj = json_pack("{s:s, s:{s:s, s:s, s:s}}",
                  "event", "channels/send",
                  "payload",
                  "channel", "gossip",
                  "name", player,
                  "message", message
                  );

  payload = json_dumps( obj, 0 );

  libwsclient_send(gossip_ws, payload);

  free( payload );
}

void gossip_broadcast(const char * channel, const char * name, const char * game, const char * message) {
  char              buf[ MAX_STRING_LENGTH ];
  DESCRIPTOR_DATA * d;

  if(!gossip_ws) {
    return;
  }

  if ( !strcmp("gossip", channel) ) {
    sprintf( buf, "&c%s of %s gossips '&W%s&c'", capitalize(name), game, message );
  } else {
    return;
  }

  for ( d = descriptor_list; d; d = d->next ) {
    CHAR_DATA * ch = ( d->original ? d->original : d->character );

    if ( d->connected == CON_PLAYING && !CHECK_BIT( ch->deaf, CHANNEL_GOSSIP ) ) {
      send_to_char( C_DEFAULT, buf, d->character );
      send_to_char( C_DEFAULT, "\n\r", d->character );
    }
  }
}

void gossip_heartbeat() {
  DESCRIPTOR_DATA * d;
  char * payload;
  json_t * obj;
  json_t * players = json_array();

  for ( d = descriptor_list; d; d = d->next ) {
    CHAR_DATA  * wch;
    wch = ( d->original ) ? d->original : d->character;

    if ( d->connected != CON_PLAYING ) {
      continue;
    }

    json_array_append_new( players, json_string( wch->name ) );
  }

  obj = json_pack("{s:s, s:{s:o}}", "event", "heartbeat", "payload", "players", players);

  payload = json_dumps( obj, 0 );

  libwsclient_send(gossip_ws, payload);

  free( payload );
}

int gossip_onclose(wsclient *c) {
  log_string("Disconnected from Gossip");
  wiznet( "Disconnected from Gossip", NULL, NULL, WIZ_GENERAL, 0, 0 );

  return 0;
}

int gossip_onerror(wsclient *c, wsclient_error *err) {
  char buf[ MAX_STRING_LENGTH ];

  sprintf(buf, "Gossip: %s", err->str);
  bug(buf, 0);

  if(err->extra_code) {
    errno = err->extra_code;
    perror("recv");
  }

  return 0;
}

int gossip_onmessage(wsclient *c, wsclient_message *msg) {
  char buf[ MAX_STRING_LENGTH ];
  const char * event;
  json_t     * message, * raw, * payload;
  json_error_t error;

  message = json_loads( msg->payload, 0, &error );

  if ( !message ) {
    sprintf( buf, "Gossip: JSON error on line %d: %s", error.line, error.text );
    log_string(buf);
    wiznet( buf, NULL, NULL, WIZ_GENERAL, 0, 0 );
    return 1;
  }

  raw = json_object_get( message, "event" );

  if ( !json_is_string( raw ) ) {
    log_string("Gossip: Unable to parse message JSON");
    return 1;
  }

  event = json_string_value( raw );
  payload = json_object_get( message, "payload" );

  if ( !strcmp(event, "heartbeat") ) {
    gossip_heartbeat();
  } else if ( !strcmp(event, "channels/broadcast") ) {
    const char * channel, * name, * game, * message;

    json_unpack(payload,
                "{s:s, s:s, s:s, s:s}",
                "channel", &channel,
                "name", &name,
                "game", &game,
                "message", &message
                );

    gossip_broadcast(channel, name, game, message);
  }

  return 0;
}

int gossip_onopen(wsclient *c) {
  char * payload;
  json_t * obj;

  log_string("Connected to Gossip");

  obj = json_pack("{s:s, s:{s:s, s:s, s:[s], s:[s], s:s, s:s}}",
                    "event", "authenticate",
                    "payload",
                    "client_id", GOSSIP_CLIENT_ID,
                    "client_secret", GOSSIP_CLIENT_SECRET,
                    "supports", "channels",
                    "channels", "gossip",
                    "version", "2.2.0",
                    "user_agent", "Maelstrom"
                    );

  payload = json_dumps( obj, 0 );

  libwsclient_send(c, payload);

  free( payload );

  return 0;
}
