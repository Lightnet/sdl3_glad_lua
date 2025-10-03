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
    // Add window flags for Lua
    lua_pushinteger(L, SDL_WINDOW_OPENGL);
    lua_setfield(L, -2, "SDL_WINDOW_OPENGL");
    // lua_pushinteger(L, SDL_WINDOW_HEADLESS); // does not exist
    // lua_setfield(L, -2, "SDL_WINDOW_HEADLESS");
    lua_pushinteger(L, SDL_WINDOW_RESIZABLE);
    lua_setfield(L, -2, "SDL_WINDOW_RESIZABLE");
    lua_setfield(L, -2, "constants");
}

// Lua: sdl.init_window(width, height, flags) -> bool, err_msg
static int sdl_init_window(lua_State *L) {
    int width = (int)luaL_checkinteger(L, 1);
    int height = (int)luaL_checkinteger(L, 2);
    Uint32 flags = (Uint32)luaL_checkinteger(L, 3);

    // If OpenGL flag is set, configure GL attributes
    if (flags & SDL_WINDOW_OPENGL) {
        SDL_GL_ResetAttributes();
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) < 0 ||
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) < 0 ||
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) < 0 ||
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) < 0 ||
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24) < 0 ||
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8) < 0 ||
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8) < 0 ||
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8) < 0 ||
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8) < 0) {
            lua_pushboolean(L, 0);
            lua_pushstring(L, SDL_GetError());
            return 2;
        }
        // Verify attributes
        int doublebuffer, depth_size;
        SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer);
        SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth_size);
        printf("SDL init_window GL Attributes: Doublebuffer %d, Depth Size %d\n", doublebuffer, depth_size);
    }

    SDL_Window *window = SDL_CreateWindow("Lua+SDL3+GL", width, height, flags);
    if (!window) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, SDL_GetError());
        return 2;
    }

    // Store window in global sdl_window
    lua_pushlightuserdata(L, window);
    lua_setglobal(L, "sdl_window");

    // Set Lua state as window property
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    if (!SDL_SetPointerProperty(props, "lua_state", L)) {
        SDL_DestroyWindow(window);
        lua_pushboolean(L, 0);
        lua_pushstring(L, SDL_GetError());
        return 2;
    }

    lua_pushboolean(L, 1);
    return 1;
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
    // Destroy window if it exists
    lua_getglobal(L, "sdl_window");
    SDL_Window *window = (SDL_Window *)lua_topointer(L, -1);
    if (window) {
        SDL_DestroyWindow(window);
    }
    lua_pop(L, 1);
    SDL_Quit();
    return 0;
}

static const struct luaL_Reg sdl_lib[] = {
    {"init_window", sdl_init_window},
    {"poll_events", sdl_poll_events},
    {"quit", sdl_quit},
    {NULL, NULL}
};

int luaopen_module_sdl(lua_State *L) {
    luaL_newlib(L, sdl_lib);
    push_event_constants(L);
    return 1;
}