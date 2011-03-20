#ifndef _SHELL_H_
#define _SHELL_H_

#include "inari.h"
#include "command.h"
#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void plugin_init(hashmap_t* map);

void cmd_exec(message_t msg);

#endif /* _SHELL_H_ */
