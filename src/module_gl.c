#include "module_gl.h"
#include <SDL3/SDL.h>
#include <glad/gl.h>  // GLAD 2.0
#include <lauxlib.h>

// Forward: Get window from Lua global
static SDL_Window *get_sdl_window(lua_State *L) {
    lua_getglobal(L, "sdl_window");
    SDL_Window *window = (SDL_Window *)lua_topointer(L, -1);
    lua_pop(L, 1);
    return window;
}

// Get GL context from module table (global fetch)
static SDL_GLContext get_gl_gl_context(lua_State *L) {
    lua_getglobal(L, "gl");  // Get the gl module table
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return NULL;
    }
    lua_getfield(L, -1, "gl_context");
    SDL_GLContext context = (SDL_GLContext)lua_topointer(L, -1);
    lua_pop(L, 2);  // Pop context and gl table
    return context;
}

// Lua: module_gl.init() -> bool, err_msg (creates context, loads GLAD, stores as gl.gl_context)
static int gl_init(lua_State *L) {
    SDL_Window *window = get_sdl_window(L);
    if (!window) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "No window found");
        return 2;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, SDL_GetError());
        return 2;
    }

    // Load GLAD 2.0 with SDL's proc loader
    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Failed to load OpenGL functions");
        SDL_GL_DestroyContext(context);
        return 2;
    }

    // Set swap interval (VSync)
    SDL_GL_SetSwapInterval(1);

    // Verify GL version
    printf("OpenGL loaded: %s %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));

    // FIXED: Store context in gl module table (fetch gl table explicitly)
    lua_getglobal(L, "gl");  // Push gl table to stack
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);  // Cleanup if not table
        SDL_GL_DestroyContext(context);
        lua_pushboolean(L, 0);
        lua_pushstring(L, "gl module not loaded");
        return 2;
    }
    lua_pushlightuserdata(L, context);
    lua_setfield(L, -2, "gl_context");  // Set on gl table (-2)
    lua_pop(L, 1);  // Pop gl table

    lua_pushboolean(L, 1);
    return 1;
}

// GL bindings
static int gl_clear(lua_State *L) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return 0;
}

static int gl_clear_color(lua_State *L) {
    float r = (float)luaL_checknumber(L, 1);
    float g = (float)luaL_checknumber(L, 2);
    float b = (float)luaL_checknumber(L, 3);
    float a = (float)luaL_optnumber(L, 4, 1.0f);
    glClearColor(r, g, b, a);
    return 0;
}

static int gl_viewport(lua_State *L) {
    int x = (int)luaL_checkinteger(L, 1);
    int y = (int)luaL_checkinteger(L, 2);
    int w = (int)luaL_checkinteger(L, 3);
    int h = (int)luaL_checkinteger(L, 4);
    glViewport(x, y, w, h);
    return 0;
}

static int gl_swap_buffers(lua_State *L) {
    SDL_Window *window = get_sdl_window(L);
    if (window) {
        SDL_GL_SwapWindow(window);
    }
    return 0;
}

// module_gl.destroy() -> void (cleanup context, clear gl.gl_context)
static int gl_destroy(lua_State *L) {
    SDL_GLContext context = get_gl_gl_context(L);
    if (context) {
        SDL_GL_DestroyContext(context);
        // Clear from module table
        lua_getglobal(L, "gl");
        if (lua_istable(L, -1)) {
            lua_pushnil(L);
            lua_setfield(L, -2, "gl_context");
        }
        lua_pop(L, 1);
        printf("GL context destroyed\n");
    }
    return 0;
}

static const struct luaL_Reg gl_lib[] = {
    {"init", gl_init},
    {"destroy", gl_destroy},
    {"clear", gl_clear},
    {"clear_color", gl_clear_color},
    {"viewport", gl_viewport},
    {"swap_buffers", gl_swap_buffers},
    {NULL, NULL}
};

int luaopen_module_gl(lua_State *L) {
    luaL_newlib(L, gl_lib);
    // Init gl_context as nil
    lua_pushnil(L);
    lua_setfield(L, -2, "gl_context");
    return 1;
}