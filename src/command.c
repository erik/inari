#include "command.h"

hashmap_t* cmd_map;

#define SET_CMD(str, fcn) hashmap_insert(cmd_map, str, (void*)fcn)

void command_init() {
  cmd_map = hashmap_new();

  SET_CMD("sayhi", cmd_say_hi);

  SET_CMD("join", cmd_join_chan);
  SET_CMD("part", cmd_part_chan);
  SET_CMD("quit", cmd_quit);
  SET_CMD("setadmin", cmd_add_admin);
}

void command_deinit() {
  hashmap_destroy(cmd_map);
}

/* TODO: this is incredibly messy, and needs to be cleaned up */
void command_handle_msg(irc_server_t* irc, char* msg) {
  /* wait for authentication to be completed to handle messages */
  if(irc->status == AUTH) {
    return;
  }

  message_t message;

  /* skip over leading ':' */
  msg += 1;

  size_t len = 0;

  len = strcspn(msg, ":!");
  msg[len] = '\0';
  char* nick = msg;
  msg += len + 1;

  char* nonnick_chrs = "*1234567890 ";

  if(len <= 1 || strcspn(nick, nonnick_chrs) != len) {
    return;
  }

  /* find the action */
  msg += strcspn(msg, " ") + 1;
  len = strcspn(msg, " ");
  msg[len] = '\0';

  char* chan = NULL;
  char* action = msg;
  char* cmd = NULL;
  enum action act;

  msg += len + 1;

  if(!strcmp(action, "PRIVMSG")) {
    act = ACT_PRIVMSG;

    /* find the channel */
    len = strcspn(msg, " ");
    msg[len] = '\0';
    chan = msg;
    msg += len + 1;

    /* skip over ':', see if there is a command, or just text */
    msg += 1;
    if(msg[0] == COMMAND_PREFIX) {
      msg += 1;
      len = strcspn(msg, " ");
      msg[len] = '\0';
      cmd = msg;
      msg += len + 1;
    }

  } else if(!strcmp(action, "JOIN")) {
    act = ACT_JOIN;
  } else if(!strcmp(action, "PART")) {
    act = ACT_PART;
  } else {
    LOG("Unhandled action: %s", action);
    return;
  }

  char* args = msg;

  message.irc = irc;
  message.action = act;
  message.nick = nick;
  message.chan = chan;
  message.cmd = cmd;
  message.args = args;

  if(cmd) {
    void (*fcn)(message_t) = (void (*)(message_t))hashmap_get(cmd_map, cmd);
    if(fcn) {
      fcn(message);
    } else {
      cmd_nofunc(message);
    }
    return;
  }
}

/* shorthand for commands that require admin privs */
#define REQUIRES_AUTH                     \
  if(!irc_is_admin(*msg.irc, msg.nick)) { \
    cmd_notadmin(msg);                    \
    return;                               \
  }

void cmd_nofunc(message_t msg) {
  irc_privmsg(*msg.irc, msg.chan, "I don't know that command");
}

void cmd_notadmin(message_t msg) {
  irc_privmsg(*msg.irc, msg.chan, "You must be an admin to do that!");
}

void cmd_say_hi(message_t msg) {
  irc_privmsgf(*msg.irc, msg.chan, "%s: Hai there! :D", msg.nick);
}

void cmd_join_chan(message_t msg) {
  REQUIRES_AUTH;
 
  irc_join(*msg.irc, msg.args);  
}

void cmd_part_chan(message_t msg) {
  REQUIRES_AUTH;
  
  irc_part(*msg.irc, msg.args);
}

void cmd_add_admin(message_t msg) {
  REQUIRES_AUTH;
  
  if(!msg.args) {
    irc_privmsg(*msg.irc, msg.chan, "You need to specify a nick to make an admin");
    return;
  }

  irc_add_admin(msg.irc, msg.args);
  irc_privmsgf(*msg.irc, msg.chan, "%s is now an admin", msg.args);
}

void cmd_quit(message_t msg) {
  REQUIRES_AUTH;

  irc_send(*msg.irc, "QUIT :Ack, I am slain!");

  msg.irc->status = CLOSED;
}
