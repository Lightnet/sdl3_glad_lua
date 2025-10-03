#include "module_sdl.h"
#include <SDL3/SDL.h>
#include <cimgui.h>  // For ImGui_ImplSDL3_ProcessEvent
#include <cimgui_impl.h>
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
        // Process event for ImGui input
        // ImGui_ImplSDL3_ProcessEvent(&event);

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
        } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            lua_pushinteger(L, (lua_Integer)event.window.data1);
            lua_setfield(L, -2, "width");
            lua_pushinteger(L, (lua_Integer)event.window.data2);
            lua_setfield(L, -2, "height");
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

static const struct luaL_Reg sdl_lib[] = {
    {"poll_events", sdl_poll_events},
    {"quit", sdl_quit},
    {NULL, NULL}
};

int luaopen_module_sdl(lua_State *L) {
    luaL_newlib(L, sdl_lib);
    push_event_constants(L);
    return 1;
}