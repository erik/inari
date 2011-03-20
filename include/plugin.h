#ifndef _PLUGINS_H_
#define _PLUGINS_H_

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include "inari.h"
#include "hashmap.h"

void init_plugins(hashmap_t* map);

#endif /* _PLUGINS_H_ */
