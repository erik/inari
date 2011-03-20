#include "inari.h"
#include "command.h"

int main(void) {
  command_init();

  irc_server_t irc = connect_to_server("irc.ninthbit.net", 6667, "inari");
  irc_add_admin(&irc, "boredomist");
  irc_add_admin(&irc, "someone_else");

  if(irc.status == CLOSED) {
    LOG("ERROR: Connect failed, aborting");
    exit(EXIT_FAILURE);
  }

  irc_authenticate(irc, NULL);
  
  while(irc.status == AUTH) {
    irc_handle(&irc);
  }

  irc_join(irc, "#tempchan");

  while(irc.status == CONN) {
    irc_handle(&irc);
  }

  irc_destroy(&irc);
  command_deinit();

  return 0;
}
