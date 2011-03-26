#ifndef _RUN_H_
#define _RUN_H_

#include "inari.h"
#include "config.h"
#include "command.h"

#include <ctype.h>
#include <sys/time.h>
#include <fcntl.h>

#include <pthread.h>

/* connect and run in a new thread */
void* run_server(void* conf);

/* load up the configuration file */
config_t* load_configurations();

/* main loop of the bot */
int run_inari();

#endif /* _RUN_H_ */
