#ifndef __LUACHK_H__
#define __LUACHK_H__

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

int  lua_safe_pcall(lua_State *, int, int, int);
void lua_safe_register(lua_State *L, const char *name, lua_CFunction f);

#if defined(PROTECT_LUA)
#	define LUA_PCALL     lua_safe_pcall
#	define LUA_REGISTER  lua_safe_register
#else
#	define LUA_PCALL     lua_pcall
#	define LUA_REGISTER  lua_register
#endif // PROTECT_LUA
#endif //__LUACHK_H__
