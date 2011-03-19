#include "command.h"

hashmap_t* cmd_map;

#define SET_CMD(str, fcn) hashmap_insert(cmd_map, str, (void*)fcn)

void command_init() {
  cmd_map = hashmap_new();

  SET_CMD("sayhi", cmd_say_hi);
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

void cmd_nofunc(message_t msg) {
  irc_privmsg(*msg.irc, msg.chan, "I don't know that command");
}

void cmd_say_hi(message_t msg) {
  irc_privmsg(*msg.irc, msg.chan, "Hello there! :D");
}
