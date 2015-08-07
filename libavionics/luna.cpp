#include "luna.h"

#include <vector>
#include <algorithm>
#include <string.h>
#include <sys/types.h>
#ifndef WINDOWS
#include <dirent.h>
#else
#include <windows.h>
#endif
#include <cstdlib>

#include "interpolator.h"

using namespace xa;

/// Stores interpolators object, which is part of the lua-cpp interpolation interface
static std::vector<Interpolator<double>*> Interpolators;

/// bitwise and
static int luaBitAnd(lua_State *L) 
{
    int n = lua_gettop(L);
    int res = 0;
    if (2 == n) 
        res = lua_tointeger(L, 1) & lua_tointeger(L, 2);
    lua_pushinteger(L, res);
    return 1;
}

/// bitwise or
static int luaBitOr(lua_State *L) 
{
    int n = lua_gettop(L);
    int res = 0;
    if (2 == n) 
        res = lua_tointeger(L, 1) | lua_tointeger(L, 2);
    lua_pushinteger(L, res);
    return 1;
}

/// bitwise xor
static int luaBitXor(lua_State *L) 
{
    int n = lua_gettop(L);
    int res = 0;
    if (2 == n) 
        res = lua_tointeger(L, 1) ^ lua_tointeger(L, 2);
    lua_pushinteger(L, res);
    return 1;
}


#ifdef WINDOWS


/// enumerate files in directory
static int luaListFiles(lua_State *L) 
{
    const char *name = lua_tostring(L, 1);
    if (! name)
        return 0;

    std::string mask = name;
    mask = mask + "\\*";

    lua_newtable(L);

    WIN32_FIND_DATA de;
    HANDLE dir = FindFirstFile(mask.c_str(), &de);

    if (dir == INVALID_HANDLE_VALUE) 
        return 1;

    int i = 1;
    do {
        lua_pushnumber(L, i);
        lua_newtable(L);
        lua_pushstring(L, "name");
        lua_pushstring(L, de.cFileName);
        lua_settable(L, -3);
        lua_pushstring(L, "type");
        lua_pushstring(L, de.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 
                "dir" : "file");
        lua_settable(L, -3);
        lua_settable(L, -3);
        i++;
    } while (FindNextFile(dir, &de));
    FindClose(dir);

    return 1;
}

#else


/// enumerate files in directory
static int luaListFiles(lua_State *L) 
{
    const char *name = lua_tostring(L, 1);
    if (! name)
        return 0;

    lua_newtable(L);

    DIR *dir = opendir(name);
    if (! dir) {
        return 1;
    }

    int i = 1;
    for (struct dirent *de = readdir(dir); de; de = readdir(dir)) {
        if ((DT_DIR == de->d_type) || (DT_LNK == de->d_type) || 
                (DT_REG == de->d_type))
        {
            lua_pushnumber(L, i);
            lua_newtable(L);
            lua_pushstring(L, "name");
            lua_pushstring(L, de->d_name);
            lua_settable(L, -3);
            lua_pushstring(L, "type");
            lua_pushstring(L, DT_DIR == de->d_type ? "dir" : "file");
            lua_settable(L, -3);
            lua_settable(L, -3);
            i++;
        }
    }
    closedir(dir);

    return 1;
}
#endif



static std::vector<double> loadLuaTable(lua_State *L, const std::size_t& stack_shift = 0)
{
    std::vector<double> values;

    /* table is in the stack at index -1 */
    lua_pushnil(L);  /* first key, table at -2 */
	while (lua_next(L, -2 - stack_shift) != 0) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        int index = lua_tonumber(L, -2);
        double value = lua_tonumber(L, -1);
        if (0 < index) {
            index = index - 1; // lua arrays are 1-based
            if (values.size() <= (unsigned)index)
                values.resize(index + 1, 0);
            values[index] = value;
        }
        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
    }

    return values;
}

static bool configureInterpolator(Interpolator<double>* inInterpolator, const std::vector<double>& inValues,
	const std::vector<std::size_t>& inDelimiters, const std::vector<std::vector<double> >& inFunction) {

	inInterpolator->setGrid(inValues);
	inInterpolator->setGridDelimiters(inDelimiters);

	for (std::size_t i = 0; i < inFunction.size(); i++) {
		inInterpolator->addFunction(inFunction[i]);
	}

	if (inInterpolator->validate()) {
		inInterpolator->calculateGradients();
		Interpolators.push_back(inInterpolator);
		return true;
	} else {
		return false;
	}
}

static int luaCreateInterpolator(lua_State *L)
{
	std::size_t dimensions = lua_objlen(L, 1);
	std::vector<double> gridValues;
	std::vector<std::size_t> gridDelimiters;
	for (std::size_t i = 1; i <= dimensions; i++) {
		lua_pushnumber(L, i); lua_gettable(L, 1);
		std::vector<double> gridPart = loadLuaTable(L);
		lua_pop(L, 1); 

		if (!gridPart.size()) {
			lua_pushnil(L);
			return 0;
		}

		gridValues.insert(gridValues.end(), gridPart.begin(), gridPart.end());

		gridDelimiters.push_back(gridPart.size());
	}

	std::size_t functions = lua_objlen(L, 2);
	std::vector<std::vector<double> > function;
	for (std::size_t i = 1; i <= functions; i++) {
		lua_pushnumber(L, i);
		lua_gettable(L, 2);
		std::vector<double> v = loadLuaTable(L);
		function.push_back(v);
		lua_pop(L, 1);
	}

	bool successConf = true;
	switch (gridDelimiters.size()) {
	case 1: {
		Interpolator1D<double>* interpolator1d = new Interpolator1D<double>;
		if (!configureInterpolator(interpolator1d, gridValues, gridDelimiters, function)) {
			successConf = false;
		} else {
			lua_pushnumber(L, interpolator1d->getID());
			return 1;
		}
	}
	break;
	case 2: {
		Interpolator2D<double>* interpolator2d = new Interpolator2D<double>;
		if (!configureInterpolator(interpolator2d, gridValues, gridDelimiters, function)) {
			successConf = false;
		} else {
			lua_pushnumber(L, interpolator2d->getID());
			return 1;
		}
	}
	break;
	case 3: {
		Interpolator3D<double>* interpolator3d = new Interpolator3D<double>;
		if (!configureInterpolator(interpolator3d, gridValues, gridDelimiters, function)) {
			successConf = false;
		} else {
			lua_pushnumber(L, interpolator3d->getID());
			return 1;
		}
	}
	break;
	case 4: {
		Interpolator4D<double>* interpolator4d = new Interpolator4D<double>;
		if (!configureInterpolator(interpolator4d, gridValues, gridDelimiters, function)) {
			successConf = false;
		} else {
			lua_pushnumber(L, interpolator4d->getID());
			return 1;
		}
	}
	break;
	case 5: {
		Interpolator5D<double>* interpolator5d = new Interpolator5D<double>;
		if (!configureInterpolator(interpolator5d, gridValues, gridDelimiters, function)) {
			successConf = false;
		} else {
			lua_pushnumber(L, interpolator5d->getID());
			return 1;
		}
	}
	break;
	default:
		lua_pushnil(L);
		return 0;
	}

	if (!successConf) {
		lua_pushnil(L);
		return 0;
	}
}

static int luaInterpolate(lua_State *L)
{
    if ((! lua_isnumber(L, 1)) || (! lua_istable(L, 2))) {
        lua_pushnil(L);
        return 1;
    }

	bool closedRange = false;
	std::size_t stack_shift = 1;
	int interpolatorID = lua_tonumber(L, 1);
	if (lua_gettop(L) == 3) {
		closedRange = (bool)lua_tonumber(L, 3);
		stack_shift = 2;
	} 

	lua_pushnumber(L, 1);
	lua_gettable(L, 2);
	std::vector<double> point = loadLuaTable(L, stack_shift);
	lua_pop(L, 1);

	std::vector<double> i_point;
	for (std::vector<Interpolator<double>*>::iterator it = Interpolators.begin(); it != Interpolators.end(); ++it) {
		if ((*it)->getID() == interpolatorID) {
			i_point = (*it)->interpolate(point, closedRange);

			lua_newtable(L);
			for (std::size_t i = 0; i < i_point.size(); i++) {
				lua_pushnumber(L, i + 1); 
				lua_pushnumber(L, i_point[i]); 
				lua_settable(L, -3);
			}

			return 1;
		}
	}

    lua_pushnil(L);
    return 1;
}

Luna::Luna(sasl_lua_creator_callback luaCreator,
                sasl_lua_destroyer_callback luaDestroyer)
{
    this->luaDestroyer = luaDestroyer;

    if (luaCreator)
        lua = luaCreator();
    else
        lua = luaL_newstate();

    if (lua)
    {
        luaL_openlibs(lua);
        LUA_REGISTER(lua, "bitand", luaBitAnd);
        LUA_REGISTER(lua, "bitor", luaBitOr);
        LUA_REGISTER(lua, "bitxor", luaBitXor);
        LUA_REGISTER(lua, "listFiles", luaListFiles);
        LUA_REGISTER(lua, "newCPPInterpolator", luaCreateInterpolator);
        LUA_REGISTER(lua, "interpolateCPP", luaInterpolate);

        lua_newtable(lua);
        lua_setfield(lua, LUA_REGISTRYINDEX, "xavionics");
    } else {
        std::abort();
    }
}

Luna::~Luna()
{
    for (std::vector<Interpolator<double>*>::iterator it = Interpolators.begin(); it != Interpolators.end(); ++it) {
		delete (*it);
	}
	Interpolators.clear();
	
	if (luaDestroyer)
        luaDestroyer(lua);
    else
        lua_close(lua);
}

bool Luna::runScript(const std::string &fileName)
{
    return luaL_dofile(lua, fileName.c_str());
}

float Luna::getFieldf(int tableIdx, const std::string &field, float dflt)
{
    float v = dflt;
    lua_getfield(lua, tableIdx, field.c_str());
    if (! lua_isnil(lua, -1))
        v = (float)lua_tonumber(lua, -1);
    lua_pop(lua, 1);
    return v;
}

double Luna::getFieldd(int tableIdx, const std::string &field, double dflt)
{
    lua_getfield(lua, tableIdx, field.c_str());
    double v = dflt;
    if (! lua_isnil(lua, -1))
        v = lua_tonumber(lua, -1);
    lua_pop(lua, 1);
    return v;
}

std::string Luna::getFields(int tableIdx, const std::string &field,
        const std::string &dflt)
{
    std::string s(dflt);
    lua_getfield(lua, tableIdx, field.c_str());
    if (! lua_isnil(lua, -1))
        s = lua_tostring(lua, -1);
    lua_pop(lua, 1);
    return s;
}

bool Luna::getFieldb(int tableIdx, const std::string &field, bool dflt)
{
    bool v = dflt;
    lua_getfield(lua, tableIdx, field.c_str());
    if (! lua_isnil(lua, -1))
        v = lua_toboolean(lua, -1);
    lua_pop(lua, 1);
    return v;
}


int Luna::getFieldi(int tableIdx, const std::string &field, int dflt)
{
    int v = dflt;
    lua_getfield(lua, tableIdx, field.c_str());
    if (! lua_isnil(lua, -1))
        v = (int)lua_tonumber(lua, -1);
    lua_pop(lua, 1);
    return v;
}


int Luna::addRef()
{
    lua_getfield(lua, LUA_REGISTRYINDEX, "xavionics");
    lua_pushvalue(lua, -2);
    int ref = luaL_ref(lua, -2);
    lua_pop(lua, 2);
    return ref;
}

void Luna::getRef(int ref)
{
    lua_getfield(lua, LUA_REGISTRYINDEX, "xavionics");
    lua_rawgeti(lua, -1, ref);
    lua_remove(lua, -2);
}

void Luna::unRef(int ref)
{
    lua_getfield(lua, LUA_REGISTRYINDEX, "xavionics");
    luaL_unref(lua, -1, ref);
}

void Luna::storeAvionics(Avionics *avionics)
{
    lua_pushlightuserdata(lua, avionics);
    lua_setfield(lua, LUA_REGISTRYINDEX, "avionics");
}

Avionics* xa::getAvionics(lua_State *lua)
{
    lua_getfield(lua, LUA_REGISTRYINDEX, "avionics");
    Avionics *v = (Avionics*)lua_touserdata(lua, -1);
    lua_pop(lua, 1);
    return v;
}

