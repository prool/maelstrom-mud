#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <wsclient/wsclient.h>

int gossip_onclose( wsclient *c );
int gossip_onerror( wsclient *c, wsclient_error *err );
int gossip_onmessage( wsclient *c, wsclient_message *msg );
int gossip_onopen( wsclient *c );

void gossip_connect() {
  // initialize new wsclient * using specified URI
  wsclient *client = libwsclient_new("wss://gossip.haus/socket");

  if(!client) {
    fprintf(stderr, "Unable to initialize new WS client.\n");
    exit(1);
  }

  // set callback functions for this client
  libwsclient_onopen(client, &onopen);
  libwsclient_onmessage(client, &onmessage);
  libwsclient_onerror(client, &onerror);
  libwsclient_onclose(client, &onclose);

  // starts run thread.
  libwsclient_run(client);
}

void gossip_broadcast(char * channel, char * player, char * message) {

}

int gossip_onclose(wsclient *c) {
  fprintf(stderr, "onclose called: %d\n", c->sockfd);
  return 0;
}

int gossip_onerror(wsclient *c, wsclient_error *err) {
  fprintf(stderr, "onerror: (%d): %s\n", err->code, err->str);

  if(err->extra_code) {
    errno = err->extra_code;
    perror("recv");
  }

  return 0;
}

int gossip_onmessage(wsclient *c, wsclient_message *msg) {
  fprintf(stderr, "onmessage: (%llu): %s\n", msg->payload_len, msg->payload);
  return 0;
}

int gossip_onopen(wsclient *c) {
  fprintf(stderr, "onopen called: %d\n", c->sockfd);
  libwsclient_send(c, "{ \"event\": \"authenticate\", \"payload\": { \"client_id\": \"0585ca62-492d-4ed5-8304-84ff51c7351b\", \"client_secret\": \"82610f34-e73c-49ab-a478-7440859fc95a\", \"supports\": [\"channels\"], \"channels\": [\"gossip\"], \"version\": \"1.0.0\", \"user_agent\": \"Maelstrom\" } }");
  return 0;
}
