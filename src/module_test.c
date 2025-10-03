// module_test.c
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdlib.h>

// Module state
static int initialized = 0;
static double stored_number = 0.0;
static char *stored_string = NULL;
static int stored_int = 0;
static int stored_bool = 0;

// Lua: init()
static int l_init(lua_State *L) {
    initialized = 1;
    // Reset stored values
    stored_number = 0.0;
    if (stored_string) {
        free(stored_string);
        stored_string = NULL;
    }
    stored_int = 0;
    stored_bool = 0;
    lua_pushboolean(L, 1); // Return true
    return 1;
}

// Lua: set_number(number)
static int l_set_number(lua_State *L) {
    if (!initialized) {
        luaL_error(L, "Module not initialized. Call init() first.");
        return 0;
    }
    luaL_checktype(L, 1, LUA_TNUMBER);
    stored_number = lua_tonumber(L, 1);
    lua_pushboolean(L, 1); // Return true
    return 1;
}

// Lua: get_number()
static int l_get_number(lua_State *L) {
    if (!initialized) {
        luaL_error(L, "Module not initialized. Call init() first.");
        return 0;
    }
    lua_pushnumber(L, stored_number);
    return 1;
}

// Lua: set_string(string)
static int l_set_string(lua_State *L) {
    if (!initialized) {
        luaL_error(L, "Module not initialized. Call init() first.");
        return 0;
    }
    luaL_checktype(L, 1, LUA_TSTRING);
    const char *str = lua_tostring(L, 1);
    if (stored_string) {
        free(stored_string);
    }
    stored_string = strdup(str);
    if (!stored_string) {
        luaL_error(L, "Memory allocation failed for string.");
        return 0;
    }
    lua_pushboolean(L, 1); // Return true
    return 1;
}

// Lua: get_string()
static int l_get_string(lua_State *L) {
    if (!initialized) {
        luaL_error(L, "Module not initialized. Call init() first.");
        return 0;
    }
    if (stored_string) {
        lua_pushstring(L, stored_string);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// Lua: set_int(number)
static int l_set_int(lua_State *L) {
    if (!initialized) {
        luaL_error(L, "Module not initialized. Call init() first.");
        return 0;
    }
    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checkinteger(L, 1); // Ensure it's an integer
    stored_int = (int)lua_tointeger(L, 1);
    lua_pushboolean(L, 1); // Return true
    return 1;
}

// Lua: get_int()
static int l_get_int(lua_State *L) {
    if (!initialized) {
        luaL_error(L, "Module not initialized. Call init() first.");
        return 0;
    }
    lua_pushinteger(L, stored_int);
    return 1;
}

// Lua: set_bool(boolean)
static int l_set_bool(lua_State *L) {
    if (!initialized) {
        luaL_error(L, "Module not initialized. Call init() first.");
        return 0;
    }
    luaL_checktype(L, 1, LUA_TBOOLEAN);
    stored_bool = lua_toboolean(L, 1);
    lua_pushboolean(L, 1); // Return true
    return 1;
}

// Lua: get_bool()
static int l_get_bool(lua_State *L) {
    if (!initialized) {
        luaL_error(L, "Module not initialized. Call init() first.");
        return 0;
    }
    lua_pushboolean(L, stored_bool);
    return 1;
}

// Lua: call_foo()
static int l_call_foo(lua_State *L) {
    if (!initialized) {
        luaL_error(L, "Module not initialized. Call init() first.");
        return 0;
    }
    printf("bar\n"); // Print "bar" to console
    lua_pushboolean(L, 1); // Return true
    return 1;
}

// Module function table
static const struct luaL_Reg module_test[] = {
    {"init", l_init},
    {"set_number", l_set_number},
    {"get_number", l_get_number},
    {"set_string", l_set_string},
    {"get_string", l_get_string},
    {"set_int", l_set_int},
    {"get_int", l_get_int},
    {"set_bool", l_set_bool},
    {"get_bool", l_get_bool},
    {"call_foo", l_call_foo},
    {NULL, NULL} // Sentinel
};

// Module entry point
int luaopen_module_test(lua_State *L) {
    luaL_newlib(L, module_test);
    return 1;
}