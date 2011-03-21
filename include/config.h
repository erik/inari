#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "inari.h"

#include <stdio.h>

typedef struct server_config {
  char* server_name;
  char* server_url;
  unsigned port;
  char* nick;
  char** channels;
  unsigned num_channels;
  char** admins;
  unsigned num_admins;
} server_config_t;

typedef struct config {
  unsigned status; /* 0 => succeeded, 1 => error occured */
  struct server_config** configs;
  unsigned num_configs;
} config_t;

/* automatically sets up and connects to a server based on config data */
irc_server_t* config_create_irc(server_config_t* config);

/* load a configuration from the given file name */
config_t* config_load(char* filename);

/* frees all memory associated with config */
void config_destroy(config_t* config);

#endif /* _CONFIG_H_ */
