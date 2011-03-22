#include "inari.h"
#include "command.h"
#include "config.h"

int main(void) {
  irc_server_t* irc;
  config_t* conf;

  char buf[256];
  snprintf(buf, 256, "%s/.inari/inari.cfg", getenv("HOME"));

  conf = config_load(buf);
  if(conf->num_configs <= 0) {
    LOG("No configurations found, aborting");
    exit(EXIT_FAILURE);
  }

  command_init();
  irc = config_create_irc(conf->configs[0]);
  
  config_destroy(conf);

   while(irc->status == CONN) {
    irc_handle(irc);
  }
  
  irc_destroy(irc);
  free(irc);
  command_deinit();

  return 0;
}
