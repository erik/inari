#include "inari.h"
#include "plugin.h"
#include "command.h"

void cmd_plugin_hello(message_t);

void plugin_init(command_handle_t* handle) {
  handle->name = "plugin";
  handle->fcn = cmd_plugin_hello;
}

void cmd_plugin_hello(message_t msg) {
  irc_privmsgf(*msg.irc, msg.chan, "%s: Hello from an external plugin!", msg.nick);
}
