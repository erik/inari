#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "inari.h"
#include "hashmap.h"

#define COMMAND_PREFIX '@'

/* shorthand for commands that require admin privs */
#define REQUIRES_AUTH                     \
  if(!irc_is_admin(*msg.irc, msg.nick)) { \
    cmd_notadmin(msg);                    \
    return;                               \
  }

enum action {
  ACT_PRIVMSG,
  ACT_JOIN,
  ACT_PART
};

enum command_type {
  CMD_BUILTIN,
  CMD_NATIVE,
  CMD_LUA
};

typedef struct message {
  irc_server_t* irc;
  enum action action;
  char* nick;
  char* chan;
  char* cmd;
  char* args;
} message_t;

typedef struct command_handle {
  void (*fcn)(message_t);
  enum command_type type;
  char* name;
  void* ptr;
} command_handle_t;

/* intialize all the required commands */
void command_init();

/* do whatever is required to the commands on program shutdown */
void command_deinit();

/* pass the message along to the proper commands */
void command_handle_msg(irc_server_t* irc, char* msg);

/* commands */
void cmd_say_hi(message_t msg);

/* admin commands */
void cmd_join_chan(message_t msg);
void cmd_part_chan(message_t msg);
void cmd_add_admin(message_t msg);
void cmd_quit(message_t msg);

/* errors */
void cmd_nofunc(message_t msg);
void cmd_notadmin(message_t msg);


#endif /* _COMMAND_H_ */
