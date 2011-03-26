#include "config.h"

static server_config_t* load_default_config() {
  server_config_t* conf = malloc(sizeof(server_config_t));
  
  conf->server_name = NULL;
  conf->server_url = NULL;
  conf->port = 6667;
  conf->nick = "inari";
  conf->channels = NULL;
  conf->num_channels = 0;
  conf->admins = NULL;
  conf->num_admins = 0;
  conf->echo = 1;

  return conf;
}

#define ERR {                                   \
  config->status = 1;                           \
  fclose(fp);                                   \
  return config;                                \
  }

config_t* config_load(char* filename) {
  config_t *config = malloc(sizeof(config_t));

  config->configs = malloc(0);
  
  FILE* fp = fopen(filename, "r");
  if(!fp) {
    LOG("Failed to open config file %s", filename);
    ERR;
  }

  server_config_t* cur = NULL;
  char* ptr = NULL;

  char lbuf[256];
  char *line = NULL;
  unsigned line_num = 1;

  while(fgets(lbuf, 256, fp)) {
    line = (char*)&lbuf;
    unsigned s = strspn(line, " \n");
    line += s;
    if(line[0] == '\0') {
      line_num++;
      continue;
    }

    /* comment */
    if(line[0] == '#') {
      line_num++;
      continue;
    }

    /* server config */
    else if(line[0] == '[') {
      /* save the old server_config first */
      if(cur) {
        config->configs = realloc(config->configs, sizeof(server_config_t*) * (config->num_configs + 1));
        config->configs[config->num_configs - 1] = cur;
      }

      config->num_configs++;
      line += 1;
      unsigned len = strcspn(line, "]");
      line[len] = '\0';

      LOG("Loading configuration: '%s'", line);

      cur = load_default_config();
      
      ptr = malloc(len);
      strcpy(ptr, line);
      
      cur->server_name = ptr;

      line_num++;
      continue;
    }

    /* laziness macros */
#define IF(x) if(!strcmp(setting, x))
#define EIF(x) else if(!strcmp(setting, x))

    if(cur) {
      char* setting = NULL;
      unsigned len = strcspn(line, ":");

      line[len] = '\0';
      setting = line;
      line += len + 1;

      if(line[0] == '\0') {
        LOG("Unexpected EOL on line %d: '%s'", line_num, lbuf);
        ERR;
      }

      IF("server") {
        len = strcspn(line, " \n");
        line[len] = '\0';
        
        ptr = malloc(len);
        strcpy(ptr, line);

        cur->server_url = ptr;
      } EIF("port") {
        len = strcspn(line, " \n");
        line[len] = '\0';
        
        unsigned num =  strtol(line, NULL, 10);

        cur->port = num;
      } EIF("nick") {
        len = strcspn(line, " \n");
        line[len] = '\0';

        ptr = malloc(len);
        strcpy(ptr, line);

        cur->nick = ptr;
      } EIF("channels") {
        char * pch;
        pch = strtok(line, " \n");
        while (pch != NULL) {
          cur->num_channels++;
          cur->channels = realloc(cur->channels, sizeof(char*) * cur->num_channels);
          
          ptr = malloc(strlen(pch));
          strcpy(ptr, pch);
          cur->channels[cur->num_channels - 1] = ptr;
          
          pch = strtok(NULL, " \n");
        }
      } EIF("admins") {
        char * pch;
        pch = strtok(line, " \n");
        while (pch != NULL) {
          cur->num_admins++;
          cur->admins = realloc(cur->admins, sizeof(char*) * cur->num_admins);
          
          ptr = malloc(strlen(pch));
          strcpy(ptr, pch);
          cur->admins[cur->num_admins - 1] = ptr;

          pch = strtok(NULL, " \n");
        }
      } EIF("echo") {
        char echo = line[0];
        if(echo == 'y') {
          cur->echo = 1;
        } else if(echo == 'n') {
          cur->echo = 0;
        } else {
          LOG("Expected a value of [yn] on line %d, got: %s", line_num, line);
          ERR;
        }
        
      } else {
        LOG("Unrecognized setting on line %d: '%s'", line_num, setting);
        ERR;
      }
      
    } else {
      LOG("Syntax error on line %d: '%s'", line_num, line);
      ERR;
    }
    line_num++;
  }

  if(config->num_configs) {
    config->configs[config->num_configs - 1] = cur;
  }
  
  fclose(fp);
  return config;
 }

irc_server_t* config_create_irc(server_config_t* config) {
  irc_server_t *irc = malloc(sizeof(irc_server_t));

  *irc = connect_to_server(config->server_url, config->port, config->nick);

  irc->echo = config->echo;

  if(irc->status == CLOSED) {
    LOG("ERROR: Connect to %s failed!", config->server_name);
    free(irc);
    return NULL;
  }

  irc_authenticate(*irc, NULL);

  while(irc->status == AUTH) {
    irc_handle(irc);
  }

  unsigned i;
  for(i = 0; i < config->num_channels; ++i) {
    irc_join(*irc, config->channels[i]);
  }

  for(i = 0; i < config->num_admins; ++i) {
    irc_add_admin(irc, config->admins[i]);
  }

  return irc;
}

void config_destroy(config_t* config) {
  unsigned i;
  for(i = 0; i < config->num_configs; ++i) {
    server_config_t* c = config->configs[i];
    free(c->server_name);
    free(c->server_url);
    free(c->nick);

    unsigned j;
    for(j = 0; j < c->num_channels; ++j) {
      free(c->channels[j]);
    }
    free(c->channels);

    for(j = 0; j < c->num_admins; ++j) {
      free(c->admins[j]);
    }
    free(c->admins);

    free(c);
  }
  free(config->configs);
  free(config);
}
