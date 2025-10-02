#include "module_sdl.h"
#include "module_lua.h"
#include "module_gl.h"
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

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // GL attributes for 3.3 Core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_Window *window = SDL_CreateWindow("Lua+SDL3+GL", 800, 600, SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    lua_State *L = luaL_newstate();
    if (!L) return 1;
    lua_atpanic(L, lua_panic);
    luaL_openlibs(L);

    // Load custom modules and set as globals (e.g., "sdl", "lua_util", "gl")
    luaL_requiref(L, "module_sdl", luaopen_module_sdl, 1);
    lua_setglobal(L, "sdl");  // FIXED: Set as global "sdl"
    
    luaL_requiref(L, "module_lua", luaopen_module_lua, 1);
    lua_setglobal(L, "lua_util");  // FIXED: Set as "lua_util" (matches module_lua)
    
    luaL_requiref(L, "module_gl", luaopen_module_gl, 1);
    lua_setglobal(L, "gl");  // FIXED: Set as global "gl"

    lua_pushlightuserdata(L, window);
    lua_setglobal(L, "sdl_window");

    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    if (!SDL_SetPointerProperty(props, "lua_state", L)) {
        fprintf(stderr, "Failed to set window property: %s\n", SDL_GetError());
        lua_close(L);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Run Lua script (it handles the main loop; we wait here)
    if (luaL_dofile(L, lua_script) != LUA_OK) {
        fprintf(stderr, "Error running '%s': %s\n", lua_script, lua_tostring(L, -1));
        lua_close(L);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Cleanup (reached only if Lua script returns/exits)
    lua_close(L);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}