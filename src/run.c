#include "run.h"

void* run_server(void* sconfig) {
  server_config_t* conf = (server_config_t*)sconfig;
  irc_server_t* irc = config_create_irc(conf);

  while(irc->socketfd && irc->status == CONN) {
    irc_handle(irc);
  }

  irc_destroy(irc);
  irc = NULL;

  return NULL;  
}

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

  pthread_t* threads = malloc(sizeof(pthread_t) * num_configs);

  command_init();

  unsigned i;
  for(i = 0; i < num_configs; ++i) {
    pthread_create(&threads[i], NULL, run_server, (void*)conf->configs[i]);
  }

  for(i = 0; i < num_configs; ++i) {
    pthread_join(threads[i], NULL);
  }

  free(threads);
  command_deinit();
  config_destroy(conf);

  return 0;
}
