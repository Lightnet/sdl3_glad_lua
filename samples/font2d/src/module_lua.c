#include "module_lua.h"
#include <lauxlib.h>
#include <sys/stat.h>

// Lua: module_lua.path_exists(path) -> bool
static int lua_path_exists(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    struct stat st;
    lua_pushboolean(L, stat(path, &st) == 0);
    return 1;
}

// Lua: module_lua.log(msg)
static int lua_log(lua_State *L) {
    const char *msg = luaL_checkstring(L, 1);
    printf("[Lua] %s\n", msg);
    return 0;
}

static const struct luaL_Reg lua_util_lib[] = {
    {"path_exists", lua_path_exists},
    {"log", lua_log},
    {NULL, NULL}
};

int luaopen_module_lua(lua_State *L) {
    luaL_newlib(L, lua_util_lib);
    return 1;
}