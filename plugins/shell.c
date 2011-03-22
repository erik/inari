#include "shell.h"

void plugin_init(command_handle_t* handle) {
  handle->name = "sh";
  handle->fcn = cmd_exec;
}

void cmd_exec(message_t msg) {
  REQUIRES_AUTH;

  if(!msg.args) {
    irc_privmsgf(*msg.irc, msg.chan, "%s: you need to give me a something to execute", msg.nick);
    return;
  }

  FILE *f;
  char line[256];
  /* limit number of lines allowed to be printed */
  unsigned lines = 0;
  
  f = (FILE*)popen(msg.args, "r");

  if (!f) {
    irc_privmsgf(*msg.irc, msg.chan, "%s: error: %s", msg.nick, strerror(errno));
    exit(1);
  }

   while(fgets(line, 256, f)) {
     if(lines++ >= 4) {
       irc_privmsgf(*msg.irc, msg.chan, "%s: (output too long, aborting)", msg.nick);
       break;
     }
     /* get rid of trailing newline */
     line[strlen(line) - 1] = '\0';

     irc_privmsgf(*msg.irc, msg.chan, "%s: %s", msg.nick, line);
   }

   pclose(f);
}
