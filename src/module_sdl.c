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

// Lua: sdl.poll_events() -> table of events
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
            lua_pushinteger(L, (lua_Integer)event.button.button);
            lua_setfield(L, -2, "button");
            lua_pushinteger(L, (lua_Integer)event.button.x);
            lua_setfield(L, -2, "x");
            lua_pushinteger(L, (lua_Integer)event.button.y);
            lua_setfield(L, -2, "y");
        }
        
        lua_settable(L, -3);
    }
    return 1;
}

// Lua: sdl.quit()
static int sdl_quit(lua_State *L) {
    SDL_Quit();
    return 0;
}

// Lua: sdl.init_gl() -> bool, err_msg (proxies to gl.init(), sets global "gl")
static int sdl_init_gl(lua_State *L) {
    // Require module_gl and set as global "gl"
    lua_getglobal(L, "require");
    lua_pushstring(L, "module_gl");
    lua_call(L, 1, 1);  // Returns module table
    lua_pushvalue(L, -1);  // Duplicate for global
    lua_setglobal(L, "gl");  // Set _G.gl = module_table
    lua_getfield(L, -1, "init");  // Get init func (stack: module_gl, init)
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);  // Pop init and module_gl
        lua_pushboolean(L, 0);
        lua_pushstring(L, "module_gl.init not available");
        return 2;
    }
    // FIXED: Direct call to init (no extra pops/duplicates/inserts)
    lua_call(L, 0, 2);  // Call init() -> bool, err (pops init, pushes results)
    return 2;  // Return bool, err
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