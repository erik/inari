#include "inari.h"
#include "command.h"

int main(void) {
  command_init();

  irc_server_t irc = connect_to_server("irc.ninthbit.net", 6667, "inari");

  if(irc.status == CLOSED) {
    LOG("ERROR: Connect failed, aborting");
    exit(EXIT_FAILURE);
  }

  irc_authenticate(irc, NULL);
  
  while(irc.status == AUTH) {
    irc_handle(&irc);
  }

  irc_join(irc, "#tempchan");

  do {
    irc_handle(&irc);
  } while (irc.status == CONN);

  return 0;
}
