#include "plugin.h"

char plugin_dir[0x400];
unsigned num;

static void load_plugin(hashmap_t* map, char* name) {
  char plugin_file[0x400];
  snprintf(plugin_file, 0x400, "%s/%s", plugin_dir, name);

  void *handle = dlopen(plugin_file, RTLD_LOCAL | RTLD_LAZY);
  if(!handle) {
    LOG("Loading plugin %s failed: %s", plugin_file, dlerror());
    return;
  }
  
  void (*init_ptr)(hashmap_t*) = (void (*)(hashmap_t*))dlsym(handle, "plugin_init");
  if(!init_ptr) {
    LOG("Couldn't load init_plugin for %s: %s", plugin_file, dlerror());
    return;
  }

  /* allow the plugin to set itself up */
  init_ptr(map);

  LOG(">> Loaded plugin %s", name);
  num++;
}

void init_plugins(hashmap_t* map) {
  /* only worry about native plugins for now */
  snprintf(plugin_dir, 0x400, "%s/.inari/native", getenv("HOME"));
  
  LOG("Loading plugins from %s... ", plugin_dir);
  num = 0;

  DIR *dp;
  struct dirent *ep;
  
  dp = opendir(plugin_dir);
  if (dp != NULL) {
    while((ep = readdir(dp))) {
      if(!strcmp(ep->d_name, ".") || !strcmp(ep->d_name, "..")) {
        continue;
      }
      load_plugin(map, ep->d_name);
    }
    closedir(dp);
  } else {
    perror("Couldn't open plugin directory");
  }

  LOG("Loaded %d native plugins.", num);

}
