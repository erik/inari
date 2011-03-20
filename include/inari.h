#ifndef _INARI_H_
#define _INARI_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define LOG_EVENTS

#ifdef LOG_EVENTS
/* prints date stamp as well as format specified */
#define LOG(...) { time_t __t; struct tm* __tm; time(&__t); __tm = localtime(&__t);  \
    char buf[80];strftime(buf, 80, "[%b %d %X] ", __tm); fprintf(stderr, "%s", buf); \
    fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#else
  #define LOG(...)
#endif

#define MESSAGE_SIZE 0x1000

enum server_status {
  AUTH,   /* authenticating with server */
  CONN,   /* connected to server */
  CLOSED, /* socket is closed */
};

typedef struct irc_server {
  int socketfd;
  enum server_status status;
  unsigned num_admins;
  char** admins;
  char* nick;
} irc_server_t;

/* if returned server has a status of CLOSED, creation failed for some reason */
irc_server_t connect_to_server(char* server, int port, char* nick);

/* closes out all connections, frees any applicable memory */
void irc_destroy(irc_server_t *irc);

/* adds a nick to the list of admin level users */
void irc_add_admin(irc_server_t* irc, char* nick);

/* returns whether or not a nick is an admin
 * 1 admin, 0 if not
 */
int irc_is_admin(irc_server_t irc, char* nick);

/* fetch and handle input */
void irc_handle(irc_server_t* irc);

/* react to a message or action */
void irc_handle_msg(irc_server_t* irc, char* msg);

/* authenticate with the server, pass can be NULL if no password authentication needed*/
void irc_authenticate(irc_server_t irc, char* pass);

/* send a message to an irc server, returns bytes written */
int irc_send(irc_server_t irc, char* msg);

/* similar to printf, allows for easier sending of more complex messages 
 * also returns bytes written
 */
int irc_sendf(irc_server_t irc, char* fmt, ...);

/* join chan */
void irc_join(irc_server_t irc, char* chan);

/* part chan */
void irc_part(irc_server_t irc, char* chan);

/* send a privmsg to chan (or nick) with message */
int irc_privmsg(irc_server_t irc, char* chan, char* msg);

/* like irc_sendf, but specifically for channel messages */
int irc_privmsgf(irc_server_t irc, char* chan, char* msg, ...);

#endif /* _INARI_H_ */
