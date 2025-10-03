// module_test.c
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

// Module state
static int initialized = 0;

// Lua: init()
static int l_init(lua_State *L) {
    initialized = 1;
    lua_pushboolean(L, 1); // Return true
    return 1;
}

// Lua: get_number()
static int l_get_number(lua_State *L) {
    if (!initialized) {
        luaL_error(L, "Module not initialized. Call init() first.");
        return 0;
    }
    lua_pushnumber(L, 42); // Return a sample number
    return 1;
}

// Module function table
static const struct luaL_Reg module_test[] = {
    {"init", l_init},
    {"get_number", l_get_number},
    {NULL, NULL} // Sentinel
};

// Module entry point
int luaopen_module_test(lua_State *L) {
    luaL_newlib(L, module_test);
    return 1;
}