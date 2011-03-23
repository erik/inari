#ifndef _LUAPLUGIN_H_
#define _LUAPLUGIN_H_

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "command.h"
#include "hashmap.h"

int lua_privmsg(lua_State* L);
int lua_send(lua_State* L);

/* loads a specified lua plugin */
void load_lua_plugin(hashmap_t* map, char* name);

/* initialize lua plugins */
unsigned lua_init(hashmap_t* cmd_map);

/* handle a command defined in a lua plugin. requires the
 * command_handle containing the plugin's lua_State
 */
void lua_handle_message(command_handle_t* cmd, message_t message);

#endif /* _LUA_H_ */
