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
  char cmd[0x400];
  
  strcpy(cmd, "2>&1 ");
  strcat(cmd, msg.args);
  
  /* limit number of lines allowed to be printed */
  unsigned lines = 0;
  
  f = (FILE*)popen(cmd, "r");

  if (!f) {
    irc_privmsgf(*msg.irc, msg.chan, "%s: error: %s", msg.nick, strerror(errno));
    exit(1);
  }

   while(fgets(line, 256, f)) {
     if(lines++ >= 4) {
       irc_privmsgf(*msg.irc, msg.chan, "%s: (output too long, aborting)", msg.nick);
       break;
     }

     if(line && strlen(line)) {
       irc_privmsgf(*msg.irc, msg.chan, "%s: %s", msg.nick, line);
     }
   }
   
   if(f)
     pclose(f);
}
