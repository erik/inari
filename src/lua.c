#include "luaplugin.h"
#include "plugin.h"

extern char lua_dir[0x400]; /* plugin.c */
unsigned num_lua;

#define ARG_CHECK {                                             \
    void *udata = luaL_checkudata(L, 1, "inari.irc");          \
    luaL_argcheck(L, udata != NULL, 1, "`irc' expected");        \
  }


int lua_privmsg(lua_State* L) {
  ARG_CHECK;
  message_t *msg = (message_t *)lua_touserdata(L, 1);
  char* chan = (char*)luaL_checkstring(L, 2);
  char* text = (char*)luaL_checkstring(L, 3);

  if(msg && chan && text) {
    irc_privmsg(*msg->irc, chan, text);
  }

  return 0;
}

int lua_send(lua_State* L) {
  ARG_CHECK;
  message_t *msg = (message_t *)lua_touserdata(L, 1);
  char* text = (char*)luaL_checkstring(L, 2);

  if(msg && text) {
    irc_send(*msg->irc, text);
  }

  return 0;
}

int lua_is_admin(lua_State* L) {
  ARG_CHECK;
  message_t *msg = (message_t *)lua_touserdata(L, 1);
  char* nick = (char*)luaL_checkstring(L, 2);

  if(msg && nick) {
    lua_pushboolean(L, irc_is_admin(*msg->irc, nick));
  }

  return 1;  
}

static const struct luaL_reg irclib [] = {
  {"privmsg", lua_privmsg},
  {"send", lua_send},
  {"admin", lua_is_admin},
  {NULL, NULL}
};

static void init_state(lua_State* L) {
  luaL_newmetatable(L, "inari.irc");

  /* use metatables to allow irc:privmsg(...) instead of irc_lib.privmsg(irc, ...) */
  lua_pushstring(L, "__index");
  lua_pushvalue(L, -2);
  lua_settable(L, -3);

  luaL_openlib(L, NULL, irclib, 0);
  luaL_openlib(L, "irc_lib", irclib, 0);
}

void load_lua_plugin(hashmap_t* map, char* name) {
  char plugin_file[0x400];
  snprintf(plugin_file, 0x400, "%s/%s", lua_dir, name);

  lua_State *L = lua_open();
  luaL_openlibs(L);

  if(luaL_loadfile(L, plugin_file) || lua_pcall(L, 0, 0, 0)) {
    LOG("Loading %s failed: %s", plugin_file,
        lua_tostring(L, -1));
    return;
  }

  lua_getglobal(L, "PluginName");
  if(!lua_isstring(L, -1)) {
    LOG("Can't load lua plugin %s: 'PluginName' doesn't exist", plugin_file);
    return;
  }
  char* plugin_name = (char*)lua_tostring(L, -1);

  /* allow whatever setup that needs to happen to take place */
  lua_getglobal(L, "plugin_init");
  if(!lua_isfunction(L, -1)) {
    LOG("Can't load lua plugin %s: function 'plugin_init' doesn't exist", plugin_file);
    return;
  } else {
    lua_pcall(L, 0, 0, 0);
  }

  /* don't actually do anything with this, just make sure that it's present */
  lua_getglobal(L, "plugin");
  if(!lua_isfunction(L, -1)) {
    LOG("Can't load lua plugin %s: function 'plugin' doesn't exist", plugin_file);
    return;
  }
  lua_pop(L, 1);

  init_state(L);

  command_handle_t* plugin = malloc(sizeof(command_handle_t));

  plugin->fcn = NULL;
  plugin->type = CMD_LUA;
  plugin->name = plugin_name;
  plugin->ptr = (void*)L;

  hashmap_insert(map, plugin_name, (void*)plugin);

  LOG(">> Loaded plugin %s", name);

  num_lua++;
}

unsigned lua_init(hashmap_t* map) {
  DIR *dp;
  struct dirent *ep;
  
  LOG("Loading Lua plugins from %s...", lua_dir);

  dp = opendir(lua_dir);
  if (dp != NULL) {
    while((ep = readdir(dp))) {
      if(!strcmp(ep->d_name, ".") || !strcmp(ep->d_name, "..")) {
        continue;
      }
      load_lua_plugin(map, ep->d_name);
    }
    closedir(dp);
  } else {
    perror("Couldn't open plugin directory");
  }

  return num_lua;
}

void lua_handle_message(command_handle_t* cmd, message_t msg) {
  lua_State* L = (lua_State*)cmd->ptr;

  int top = lua_gettop(L);
  lua_getglobal(L, "AdminOnly");
  
  /* if the top of the stack has changed (AdminOnly exists) */
  if(top - lua_gettop(L)) {
    if(lua_toboolean(L, -1)) {
      REQUIRES_AUTH;
    }
  }

  lua_getglobal(L, "plugin");

  message_t *msg_ptr = (message_t*)lua_newuserdata(L, sizeof(message_t));
  *msg_ptr = msg;

  /* set the metatable of msg_ptr */
  luaL_getmetatable(L, "inari.irc");

  /* setting values of the metatable, irc.chan, irc.nick, ... */
  lua_pushstring(L, "nick");
  lua_pushstring(L, msg.nick);
  lua_settable(L, -3);

  lua_pushstring(L, "chan");
  lua_pushstring(L, msg.chan);
  lua_settable(L, -3);


  lua_pushstring(L, "cmd");
  lua_pushstring(L, msg.cmd);
  lua_settable(L, -3);


  lua_pushstring(L, "args");
  lua_pushstring(L, msg.args? msg.args : "");
  lua_settable(L, -3);

  lua_setmetatable(L, -2);

  if(lua_pcall(L, 1, 0, 0)) {
    LOG("Error calling Lua plugin %s: %s", cmd->name, lua_tostring(L, -1));
  }
}
