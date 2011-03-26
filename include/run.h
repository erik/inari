#ifndef _RUN_H_
#define _RUN_H_

#include "inari.h"
#include "config.h"
#include "command.h"

#include <ctype.h>
#include <sys/time.h>
#include <fcntl.h>

config_t* load_configurations();

int run_inari();

#endif /* _RUN_H_ */
