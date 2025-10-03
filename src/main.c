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

static void check_gl_attribute(SDL_GLAttr attr, const char *attr_name, int value) {
    if (SDL_GL_SetAttribute(attr, value) < 0) {
        fprintf(stderr, "Failed to set %s: %s\n", attr_name, SDL_GetError());
    } else {
        int set_value;
        SDL_GL_GetAttribute(attr, &set_value);
        printf("Set %s to %d\n", attr_name, set_value);
    }
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

    // Reset GL attributes to ensure a clean state
    SDL_GL_ResetAttributes();

    // GL attributes for 3.3 Core
    check_gl_attribute(SDL_GL_CONTEXT_PROFILE_MASK, "SDL_GL_CONTEXT_PROFILE_MASK", SDL_GL_CONTEXT_PROFILE_CORE);
    check_gl_attribute(SDL_GL_CONTEXT_MAJOR_VERSION, "SDL_GL_CONTEXT_MAJOR_VERSION", 3);
    check_gl_attribute(SDL_GL_CONTEXT_MINOR_VERSION, "SDL_GL_CONTEXT_MINOR_VERSION", 3);
    check_gl_attribute(SDL_GL_DOUBLEBUFFER, "SDL_GL_DOUBLEBUFFER", 1);
    check_gl_attribute(SDL_GL_DEPTH_SIZE, "SDL_GL_DEPTH_SIZE", 24);
    check_gl_attribute(SDL_GL_RED_SIZE, "SDL_GL_RED_SIZE", 8);
    check_gl_attribute(SDL_GL_GREEN_SIZE, "SDL_GL_GREEN_SIZE", 8);
    check_gl_attribute(SDL_GL_BLUE_SIZE, "SDL_GL_BLUE_SIZE", 8);
    check_gl_attribute(SDL_GL_ALPHA_SIZE, "SDL_GL_ALPHA_SIZE", 8);

    SDL_Window *window = SDL_CreateWindow("Lua+SDL3+GL", 800, 600, SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    lua_State *L = luaL_newstate();
    if (!L) {
        SDL_DestroyWindow(window);
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
    lua_pushcfunction(L, luaopen_module_imgui);
    lua_setfield(L, -2, "module_imgui");
    lua_pushcfunction(L, luaopen_module_enet);
    lua_setfield(L, -2, "module_enet");

    lua_pop(L, 2); // Pop package.preload and package tables

    // Set sdl_window global
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

    // Run Lua script
    if (luaL_dofile(L, lua_script) != LUA_OK) {
        fprintf(stderr, "Error running '%s': %s\n", lua_script, lua_tostring(L, -1));
        lua_close(L);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Cleanup
    lua_close(L);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}