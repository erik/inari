#include "run.h"

config_t* load_configurations() {
  char buf[256];
  snprintf(buf, 256, "%s/.inari/inari.cfg", getenv("HOME"));
  
  config_t* conf = config_load(buf);
  if(conf->num_configs <= 0) {
    LOG("No configurations found, aborting");
    exit(EXIT_FAILURE);
  }

  return conf;
}

int run_inari() {
  config_t* conf = load_configurations();
  unsigned num_configs = conf->num_configs;

  irc_server_t** connections = malloc(sizeof(irc_server_t*) * num_configs);

  unsigned i;
  for(i = 0; i < num_configs; ++i) {
    connections[i] = config_create_irc(conf->configs[i]);
  }

  command_init();

  int max = -1;
  int open_connections = num_configs;
  while(open_connections > 0) {
    fd_set sockets;
    FD_ZERO(&sockets);
    /* this is dumb, but select requires it */
    for(i = 0; i < num_configs; ++i) {
      if(!connections[i]) {
      } else {
        max = connections[i]->socketfd > max ? connections[i]->socketfd : max;
        FD_SET(connections[i]->socketfd, &sockets);
      }
    }    

    int num = select(max + 1, &sockets, NULL, NULL, NULL);
    if(num == 0) {
      break;
    } else if(num < 0) {
      perror("select");
      break;
    }
    
    for(i = 0; i < num_configs; ++i) {
      if(!connections[i]) {
        /* ignore NULL ptrs */
      } else {
        if(FD_ISSET(connections[i]->socketfd, &sockets)) {
          irc_handle(connections[i]);
        }
      }
    }

    /* clean up closed connections */
    for(i = 0; i < num_configs; ++i) {
      if(connections[i] == NULL) {
        continue;
      }
      
      if(connections[i]->socketfd == 0 || connections[i]->status == CLOSED) {
        open_connections--;
        irc_destroy(connections[i]);
        connections[i] = NULL;
      }
    }
  }
  
  free(connections);
  command_deinit();
  config_destroy(conf);

  return 0;
}
