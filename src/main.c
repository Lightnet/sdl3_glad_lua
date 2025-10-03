// main entry
#include "module_sdl.h"
#include "module_lua.h"
#include "module_gl.h"
#include "module_imgui.h"
#include "module_enet.h"
#include <SDL3/SDL.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static int lua_panic(lua_State *L) {
    fprintf(stderr, "Lua panic: %s\n", lua_tostring(L, -1));
    return 0;
}

int main(int argc, char **argv) {
    const char *lua_script = "main.lua";
    if (argc > 1) lua_script = argv[1];

    struct stat st;
    if (stat(lua_script, &st) != 0) {
        fprintf(stderr, "Error: Lua script '%s' not found.\n", lua_script);
        return 1;
    }

    // Initialize SDL with minimal subsystems (can be extended in Lua)
    if (SDL_Init(0) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    lua_State *L = luaL_newstate();
    if (!L) {
        SDL_Quit();
        return 1;
    }
    lua_atpanic(L, lua_panic);
    luaL_openlibs(L);

    // Preload custom modules into package.preload
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, luaopen_module_sdl);
    lua_setfield(L, -2, "module_sdl");
    lua_pushcfunction(L, luaopen_module_lua);
    lua_setfield(L, -2, "lua_util");
    lua_pushcfunction(L, luaopen_module_gl);
    lua_setfield(L, -2, "module_gl");
    // lua_pushcfunction(L, luaopen_module_imgui);
    // lua_setfield(L, -2, "module_imgui");
    lua_pushcfunction(L, luaopen_module_enet);
    lua_setfield(L, -2, "module_enet");

    lua_pop(L, 2); // Pop package.preload and package tables

    // Run Lua script
    if (luaL_dofile(L, lua_script) != LUA_OK) {
        fprintf(stderr, "Error running '%s': %s\n", lua_script, lua_tostring(L, -1));
        lua_close(L);
        SDL_Quit();
        return 1;
    }

    // Cleanup
    lua_close(L);
    SDL_Quit();
    return 0;
}