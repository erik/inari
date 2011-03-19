#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "inari.h"
#include "hashmap.h"

#define COMMAND_PREFIX '@'

enum action {
  ACT_PRIVMSG,
  ACT_JOIN,
  ACT_PART
};

typedef struct message {
  irc_server_t* irc;
  enum action action;
  char* nick;
  char* chan;
  char* cmd;
  char* args;
} message_t;

/* intialize all the required commands */
void command_init();

/* pass the message along to the proper commands */
void command_handle_msg(irc_server_t* irc, char* msg);

/* commands */
void cmd_say_hi(message_t msg);

/* admin commands */
void cmd_join_chan(message_t msg);
void cmd_part_chan(message_t msg);
void cmd_add_admin(message_t msg);

/* errors */
void cmd_nofunc(message_t msg);
void cmd_notadmin(message_t msg);


#endif /* _COMMAND_H_ */
