#include "luachk.h"
#include "../xap/xpsdk.h"

#define ALLOC_LOCK   0x00A110C4
#define ALLOC_UNLOCK 0x00A110C5

static void lua_safe_lock() 
{
	XPLMSendMessageToPlugin(XPLM_PLUGIN_XPLANE, ALLOC_LOCK, NULL);
}

static void lua_safe_unlock() 
{
	XPLMSendMessageToPlugin(XPLM_PLUGIN_XPLANE, ALLOC_UNLOCK, NULL);
}

int
lua_safe_callback(lua_State *L)
{
	lua_safe_lock();
	lua_CFunction f = (lua_CFunction )lua_touserdata(L, lua_upvalueindex(1));
	int r =  f(L);
	lua_safe_unlock();
	return r;
}

void
lua_safe_register(lua_State *L, const char *name, lua_CFunction f) 
{
	lua_pushlightuserdata(L, (void *)f);
	lua_pushcclosure(L, &lua_safe_callback, 1);
	lua_setglobal(L, name);
}

int 
lua_safe_pcall(lua_State *L, int a, int b, int c) 
{
	lua_safe_unlock();
	int r = lua_pcall(L, a, b, c);
	lua_safe_lock();
	return r;
}
