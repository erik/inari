#include "command.h"
#include "plugin.h"

hashmap_t* cmd_map;

#define SET_CMD(str, f) {                                               \
    command_handle_t* h = malloc(sizeof(command_handle_t));             \
    h->fcn = f; h->type = CMD_BUILTIN; h->name = str; h->ptr = NULL;    \
    hashmap_insert(cmd_map, str, (void*)h);                             \
  }

void command_init() {
  cmd_map = hashmap_new();

  init_plugins(cmd_map);

  SET_CMD("sayhi", cmd_say_hi);

  SET_CMD("join", cmd_join_chan);
  SET_CMD("part", cmd_part_chan);
  SET_CMD("quit", cmd_quit);
  SET_CMD("setadmin", cmd_add_admin);
}

static void free_cmd(hashnode_t* node) {
  if(!node) {
    return;
  }

  free_cmd(node->left);
  free_cmd(node->right);

  command_handle_t* h = (command_handle_t*)node->data;
  switch(h->type) {
  case CMD_BUILTIN:
    break;
  case CMD_NATIVE:
    dlclose(h->ptr);
    break;
  }
  free(h);
}

void command_deinit() {
  hashnode_t* node = cmd_map->root;

  free_cmd(node);
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
  
  /* strip leading whitespace */
  unsigned s = strspn(msg, " \r\n");
  msg += s;

  char* args = msg;

  if(args && msg[0] == '\0') {
    args = NULL;
  }

  message.irc = irc;
  message.action = act;
  message.nick = nick;
  message.chan = chan;
  message.cmd = cmd;
  message.args = args;

  if(cmd) {
    command_handle_t* handle = hashmap_get(cmd_map, cmd);
    if(handle) {
      switch(handle->type) {
      case CMD_BUILTIN:
      case CMD_NATIVE:
        handle->fcn(message);
        break;
      }
    } else {
      cmd_nofunc(message);
    }
    return;
  }
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
