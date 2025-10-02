#include "module_sdl.h"
#include <SDL3/SDL.h>
#include <lauxlib.h>

// Helper: Push event constants table
static void push_event_constants(lua_State *L) {
    lua_newtable(L);
    lua_pushinteger(L, SDL_EVENT_QUIT);
    lua_setfield(L, -2, "SDL_EVENT_QUIT");
    lua_pushinteger(L, SDL_EVENT_WINDOW_RESIZED);
    lua_setfield(L, -2, "SDL_EVENT_WINDOW_RESIZED");
    lua_pushinteger(L, SDL_EVENT_KEY_DOWN);
    lua_setfield(L, -2, "SDL_EVENT_KEY_DOWN");
    lua_pushinteger(L, SDL_EVENT_MOUSE_BUTTON_DOWN);
    lua_setfield(L, -2, "SDL_EVENT_MOUSE_BUTTON_DOWN");
    lua_setfield(L, -2, "constants");
}

// Lua: module_sdl.poll_events() -> table of events
static int sdl_poll_events(lua_State *L) {
    lua_newtable(L);
    int idx = 1;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        lua_pushinteger(L, idx++);
        lua_newtable(L);
        
        lua_pushinteger(L, (lua_Integer)event.type);
        lua_setfield(L, -2, "type");
        
        if (event.type == SDL_EVENT_KEY_DOWN) {
            lua_pushinteger(L, (lua_Integer)event.key.key);
            lua_setfield(L, -2, "key");
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            lua_pushinteger(L, (lua_Integer)event.button.button);  // Fixed: event.button
            lua_setfield(L, -2, "button");
            lua_pushinteger(L, (lua_Integer)event.button.x);  // Fixed
            lua_setfield(L, -2, "x");
            lua_pushinteger(L, (lua_Integer)event.button.y);  // Fixed
            lua_setfield(L, -2, "y");
        }
        
        lua_settable(L, -3);
    }
    return 1;
}

// Lua: module_sdl.quit()
static int sdl_quit(lua_State *L) {
    SDL_Quit();
    return 0;
}

// Lua: module_sdl.init_gl() -> bool, err_msg (proxies to module_gl)
static int sdl_init_gl(lua_State *L) {
    // Require module_gl if not loaded
    lua_getglobal(L, "require");
    lua_pushstring(L, "module_gl");
    lua_call(L, 1, 1);
    lua_getfield(L, -1, "init");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);  // Pop init and module_gl
        lua_pushboolean(L, 0);
        lua_pushstring(L, "module_gl not available");
        return 2;
    }
    lua_pop(L, 1);  // Pop init
    lua_pushvalue(L, -1);  // Duplicate module_gl table
    lua_pushstring(L, "init");
    lua_gettable(L, -2);
    lua_call(L, 0, 2);  // Call module_gl.init() -> bool, err?
    return 2;  // Return its results
}

static const struct luaL_Reg sdl_lib[] = {
    {"poll_events", sdl_poll_events},
    {"quit", sdl_quit},
    {"init_gl", sdl_init_gl},
    {NULL, NULL}
};

int luaopen_module_sdl(lua_State *L) {
    luaL_newlib(L, sdl_lib);
    push_event_constants(L);
    return 1;
}