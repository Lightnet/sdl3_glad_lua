#include "module_gl.h"
#include <SDL3/SDL.h>
#include <glad/gl.h>  // GLAD 2.0
#include <lauxlib.h>
#include <stdio.h>

// Static variable to store the OpenGL context
static SDL_GLContext g_gl_context = NULL;

// Forward: Get window from Lua global
static SDL_Window *get_sdl_window(lua_State *L) {
    lua_getglobal(L, "sdl_window");
    SDL_Window *window = (SDL_Window *)lua_topointer(L, -1);
    lua_pop(L, 1);
    return window;
}

// Updated function: Lua: gl.get_gl_context() -> lightuserdata (SDL_GLContext)
static int gl_get_gl_context(lua_State *L) {
    printf("is this local get gl_get_gl_context???\n");
    if (!g_gl_context) {
        lua_pushnil(L);
        lua_pushstring(L, "OpenGL context not initialized");
        return 2;
    }
    lua_pushlightuserdata(L, g_gl_context);
    return 1;
}

// Lua: module_gl.init() -> bool, err_msg (creates context, loads GLAD, stores as gl.gl_context)
// Existing gl_init function (modified to show context storage)
static int gl_init(lua_State *L) {
    printf("get window\n");
    SDL_Window *window = get_sdl_window(L);
    if (!window) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "No window found");
        return 2;
    }
    printf("get flags\n");
    Uint32 flags = SDL_GetWindowFlags(window);
    if (!(flags & SDL_WINDOW_OPENGL)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Window is not an OpenGL window");
        return 2;
    }

    // Log OpenGL attributes for debugging
    int major, minor, profile, doublebuffer, depth_size;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer);
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth_size);
    printf("GL Attributes: Version %d.%d, Profile %d, Doublebuffer %d, Depth Size %d\n",
           major, minor, profile, doublebuffer, depth_size);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        lua_pushboolean(L, 0);
        char err_msg[512];
        snprintf(err_msg, 512, "SDL_GL_CreateContext failed: %s", SDL_GetError());
        lua_pushstring(L, err_msg);
        return 2;
    }

    printf("gladLoadGL\n");
    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Failed to load OpenGL functions");
        SDL_GL_DestroyContext(context);
        return 2;
    }

    printf("SDL_GL_SetSwapInterval\n");
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Failed to set VSync: %s\n", SDL_GetError());
    }

    printf("OpenGL loaded: %s %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));

    // Store context in static variable
    g_gl_context = context;

    // Optionally store in gl module table for backward compatibility
    lua_getglobal(L, "gl");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        SDL_GL_DestroyContext(context);
        g_gl_context = NULL;
        lua_pushboolean(L, 0);
        lua_pushstring(L, "gl module not loaded");
        return 2;
    }
    lua_pushlightuserdata(L, context);
    lua_setfield(L, -2, "gl_context");
    lua_pop(L, 1);

    lua_pushboolean(L, 1);
    printf("end gl set up.\n");
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

// Existing gl_destroy function (unchanged)
static int gl_destroy(lua_State *L) {
    if (g_gl_context) {
        SDL_GL_DestroyContext(g_gl_context);
        g_gl_context = NULL;
    }
    // Clear gl_context in module table for consistency
    lua_getglobal(L, "gl");
    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        lua_setfield(L, -2, "gl_context");
        lua_pop(L, 1);
    }
    return 0;
}

// Shader functions
static int gl_create_shader(lua_State *L) {
    GLenum type = (GLenum)luaL_checkinteger(L, 1);
    GLuint shader = glCreateShader(type);
    lua_pushinteger(L, shader);
    return 1;
}

static int gl_shader_source(lua_State *L) {
    GLuint shader = (GLuint)luaL_checkinteger(L, 1);
    const char *source = luaL_checkstring(L, 2);
    glShaderSource(shader, 1, &source, NULL);
    return 0;
}

static int gl_compile_shader(lua_State *L) {
    GLuint shader = (GLuint)luaL_checkinteger(L, 1);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        lua_pushboolean(L, 0);
        lua_pushstring(L, infoLog);
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

static int gl_create_program(lua_State *L) {
    GLuint program = glCreateProgram();
    lua_pushinteger(L, program);
    return 1;
}

static int gl_attach_shader(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    GLuint shader = (GLuint)luaL_checkinteger(L, 2);
    glAttachShader(program, shader);
    return 0;
}

static int gl_link_program(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        lua_pushboolean(L, 0);
        lua_pushstring(L, infoLog);
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

static int gl_use_program(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    glUseProgram(program);
    return 0;
}

// Vertex Array and Buffer functions
static int gl_gen_vertex_arrays(lua_State *L) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    lua_pushinteger(L, vao);
    return 1;
}

static int gl_bind_vertex_array(lua_State *L) {
    GLuint vao = (GLuint)luaL_checkinteger(L, 1);
    glBindVertexArray(vao);
    return 0;
}

static int gl_gen_buffers(lua_State *L) {
    GLuint vbo;
    glGenBuffers(1, &vbo);
    lua_pushinteger(L, vbo);
    return 1;
}

static int gl_bind_buffer(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    GLuint vbo = (GLuint)luaL_checkinteger(L, 2);
    glBindBuffer(target, vbo);
    return 0;
}

static int gl_buffer_data(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    const char *data = luaL_checkstring(L, 2); // Expect a string of raw float data
    size_t size = luaL_checkinteger(L, 3);
    GLenum usage = (GLenum)luaL_checkinteger(L, 4);
    glBufferData(target, size, data, usage);
    return 0;
}

static int gl_vertex_attrib_pointer(lua_State *L) {
    GLuint index = (GLuint)luaL_checkinteger(L, 1);
    GLint size = (GLint)luaL_checkinteger(L, 2);
    GLenum type = (GLenum)luaL_checkinteger(L, 3);
    GLboolean normalized = (GLboolean)lua_toboolean(L, 4);
    GLsizei stride = (GLsizei)luaL_checkinteger(L, 5);
    GLintptr offset = (GLintptr)luaL_checkinteger(L, 6);
    glVertexAttribPointer(index, size, type, normalized, stride, (const void *)offset);
    return 0;
}

static int gl_enable_vertex_attrib_array(lua_State *L) {
    GLuint index = (GLuint)luaL_checkinteger(L, 1);
    glEnableVertexAttribArray(index);
    return 0;
}

// Draw function
static int gl_draw_arrays(lua_State *L) {
    GLenum mode = (GLenum)luaL_checkinteger(L, 1);
    GLint first = (GLint)luaL_checkinteger(L, 2);
    GLsizei count = (GLsizei)luaL_checkinteger(L, 3);
    glDrawArrays(mode, first, count);
    return 0;
}

static const struct luaL_Reg gl_lib[] = {
    {"init", gl_init},
    {"destroy", gl_destroy},
    {"get_gl_context", gl_get_gl_context},
    {"clear", gl_clear},
    {"clear_color", gl_clear_color},
    {"viewport", gl_viewport},
    {"swap_buffers", gl_swap_buffers},
    {"create_shader", gl_create_shader},
    {"shader_source", gl_shader_source},
    {"compile_shader", gl_compile_shader},
    {"create_program", gl_create_program},
    {"attach_shader", gl_attach_shader},
    {"link_program", gl_link_program},
    {"use_program", gl_use_program},
    {"gen_vertex_arrays", gl_gen_vertex_arrays},
    {"bind_vertex_array", gl_bind_vertex_array},
    {"gen_buffers", gl_gen_buffers},
    {"bind_buffer", gl_bind_buffer},
    {"buffer_data", gl_buffer_data},
    {"vertex_attrib_pointer", gl_vertex_attrib_pointer},
    {"enable_vertex_attrib_array", gl_enable_vertex_attrib_array},
    {"draw_arrays", gl_draw_arrays},
    {NULL, NULL}
};

int luaopen_module_gl(lua_State *L) {
    luaL_newlib(L, gl_lib);
    // Init gl_context as nil
    lua_pushnil(L);
    lua_setfield(L, -2, "gl_context");
    // Add constants
    // lua_newtable(L);
    lua_pushinteger(L, GL_VERTEX_SHADER); lua_setfield(L, -2, "VERTEX_SHADER");
    lua_pushinteger(L, GL_FRAGMENT_SHADER); lua_setfield(L, -2, "FRAGMENT_SHADER");
    lua_pushinteger(L, GL_ARRAY_BUFFER); lua_setfield(L, -2, "ARRAY_BUFFER");
    lua_pushinteger(L, GL_STATIC_DRAW); lua_setfield(L, -2, "STATIC_DRAW");
    lua_pushinteger(L, GL_FLOAT); lua_setfield(L, -2, "FLOAT");
    lua_pushinteger(L, GL_TRIANGLES); lua_setfield(L, -2, "TRIANGLES");
    lua_pushinteger(L, GL_COLOR_BUFFER_BIT); lua_setfield(L, -2, "COLOR_BUFFER_BIT");
    lua_pushinteger(L, GL_DEPTH_BUFFER_BIT); lua_setfield(L, -2, "DEPTH_BUFFER_BIT");
    // lua_setfield(L, -2, "constants");
    return 1;
}