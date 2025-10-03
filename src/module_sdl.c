#include "module_sdl.h"
#include <SDL3/SDL.h>
#include <cimgui.h>  // For ImGui_ImplSDL3_ProcessEvent
#include <cimgui_impl.h>
#include <lauxlib.h>

// Helper: Push event and subsystem constants table
static void push_event_constants(lua_State *L) {
    // Event constants
    lua_pushinteger(L, SDL_EVENT_QUIT);
    lua_setfield(L, -2, "SDL_EVENT_QUIT");
    lua_pushinteger(L, SDL_EVENT_WINDOW_RESIZED);
    lua_setfield(L, -2, "SDL_EVENT_WINDOW_RESIZED");
    lua_pushinteger(L, SDL_EVENT_KEY_DOWN);
    lua_setfield(L, -2, "SDL_EVENT_KEY_DOWN");
    lua_pushinteger(L, SDL_EVENT_MOUSE_BUTTON_DOWN);
    lua_setfield(L, -2, "SDL_EVENT_MOUSE_BUTTON_DOWN");
    // Window flags
    lua_pushinteger(L, SDL_WINDOW_OPENGL);
    lua_setfield(L, -2, "SDL_WINDOW_OPENGL");
    lua_pushinteger(L, SDL_WINDOW_RESIZABLE);
    lua_setfield(L, -2, "SDL_WINDOW_RESIZABLE");
    // Subsystem flags
    lua_pushinteger(L, SDL_INIT_VIDEO);
    lua_setfield(L, -2, "SDL_INIT_VIDEO");
    lua_pushinteger(L, SDL_INIT_EVENTS);
    lua_setfield(L, -2, "SDL_INIT_EVENTS");
    lua_pushinteger(L, 0);
    lua_setfield(L, -2, "SDL_INIT_NONE");
}

// Lua: sdl.init(subsystems) -> bool, err_msg
static int sdl_init(lua_State *L) {
    Uint32 subsystems = (Uint32)luaL_checkinteger(L, 1);
    if (SDL_Init(subsystems) < 0) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, SDL_GetError());
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

// Lua: sdl.init_window(width, height, flags) -> bool, err_msg
static int sdl_init_window(lua_State *L) {
    int width = (int)luaL_checkinteger(L, 1);
    int height = (int)luaL_checkinteger(L, 2);
    Uint32 flags = (Uint32)luaL_checkinteger(L, 3);

    // If OpenGL flag is set, configure GL attributes
    if (flags & SDL_WINDOW_OPENGL) {
        SDL_GL_ResetAttributes();
        // Set attributes in a specific order
        const struct { SDL_GLAttr attr; const char *name; int value; } attrs[] = {
            { SDL_GL_RED_SIZE, "SDL_GL_RED_SIZE", 8 },
            { SDL_GL_GREEN_SIZE, "SDL_GL_GREEN_SIZE", 8 },
            { SDL_GL_BLUE_SIZE, "SDL_GL_BLUE_SIZE", 8 },
            { SDL_GL_ALPHA_SIZE, "SDL_GL_ALPHA_SIZE", 8 },
            { SDL_GL_DOUBLEBUFFER, "SDL_GL_DOUBLEBUFFER", 1 },
            { SDL_GL_DEPTH_SIZE, "SDL_GL_DEPTH_SIZE", 24 },
            { SDL_GL_CONTEXT_PROFILE_MASK, "SDL_GL_CONTEXT_PROFILE_MASK", SDL_GL_CONTEXT_PROFILE_CORE },
            { SDL_GL_CONTEXT_MAJOR_VERSION, "SDL_GL_CONTEXT_MAJOR_VERSION", 3 },
            { SDL_GL_CONTEXT_MINOR_VERSION, "SDL_GL_CONTEXT_MINOR_VERSION", 3 },
        };
        for (int i = 0; i < sizeof(attrs) / sizeof(attrs[0]); i++) {
            if (SDL_GL_SetAttribute(attrs[i].attr, attrs[i].value) < 0) {
                // fprintf(stderr, "Failed to set %s: %s\n", attrs[i].name, SDL_GetError());
            } else {
                int set_value;
                if (SDL_GL_GetAttribute(attrs[i].attr, &set_value) < 0) {
                    // fprintf(stderr, "Failed to get %s: %s\n", attrs[i].name, SDL_GetError());
                } else {
                    printf("Set %s to %d\n", attrs[i].name, set_value);
                    if (set_value != attrs[i].value) {
                        // fprintf(stderr, "Warning: %s set to %d, expected %d\n", 
                        //         attrs[i].name, set_value, attrs[i].value);
                    }
                }
            }
        }
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

// Lua: sdl.poll_events() -> table of events and imgui ProcessEvent
static int sdl_poll_events_ig(lua_State *L) {
    lua_newtable(L);
    int idx = 1;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Process event for ImGui input
        ImGui_ImplSDL3_ProcessEvent(&event);

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
    {"init", sdl_init},
    {"init_window", sdl_init_window},
    {"poll_events", sdl_poll_events},
    {"poll_events_ig", sdl_poll_events_ig},
    {"quit", sdl_quit},
    {NULL, NULL}
};

int luaopen_module_sdl(lua_State *L) {
    luaL_newlib(L, sdl_lib);
    push_event_constants(L);
    return 1;
}