#include "module_gl.h"
#include <SDL3/SDL.h>
#include <glad/gl.h>  // GLAD 2.0
#include <lauxlib.h>
#include <stdio.h>
#include <cglm/cglm.h>

// Static variable to store the OpenGL context
static SDL_GLContext g_gl_context = NULL;

// Helper to check OpenGL errors and push to Lua
static int push_gl_error(lua_State *L, const char *context) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        lua_pushnil(L);
        lua_pushfstring(L, "OpenGL error at %s: %d", context, err);
        return 2;
    }
    return 0;
}

// Helper: Get window from Lua argument
static SDL_Window *get_sdl_window(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        luaL_error(L, "Expected light userdata for window");
        return NULL;
    }
    SDL_Window *window = (SDL_Window *)lua_touserdata(L, 1);
    return window;
}

// Check if OpenGL context is valid
static int check_gl_context(lua_State *L) {
    if (!g_gl_context) {
        lua_pushnil(L);
        lua_pushstring(L, "OpenGL context not initialized");
        return 2;
    }
    return 0;
}

// Updated function: Lua: gl.get_gl_context() -> lightuserdata (SDL_GLContext)
static int gl_get_gl_context(lua_State *L) {
    if (!g_gl_context) {
        lua_pushnil(L);
        lua_pushstring(L, "OpenGL context not initialized");
        return 2;
    }
    lua_pushlightuserdata(L, g_gl_context);
    return 1;
}

// Lua: gl.init(window) -> bool, context, err_msg
static int gl_init(lua_State *L) {
    SDL_Window *window = get_sdl_window(L);
    if (!window) {
        lua_pushboolean(L, 0);
        lua_pushnil(L);
        lua_pushstring(L, "No window found");
        return 3;
    }
    Uint32 flags = SDL_GetWindowFlags(window);
    if (!(flags & SDL_WINDOW_OPENGL)) {
        lua_pushboolean(L, 0);
        lua_pushnil(L);
        lua_pushstring(L, "Window is not an OpenGL window");
        return 3;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    int major, minor, profile, doublebuffer, depth_size;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer);
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth_size);
    printf("GL Attributes: Version %d.%d, Profile %d, Doublebuffer %d, Depth Size %d\n",
           major, minor, profile, doublebuffer, depth_size);

    if (!context) {
        lua_pushboolean(L, 0);
        lua_pushnil(L);
        char err_msg[512];
        snprintf(err_msg, 512, "SDL_GL_CreateContext failed: %s", SDL_GetError());
        lua_pushstring(L, err_msg);
        return 3;
    }

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        lua_pushboolean(L, 0);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to load OpenGL functions");
        SDL_GL_DestroyContext(context);
        return 3;
    }

    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Failed to set VSync: %s\n", SDL_GetError());
    }

    printf("OpenGL loaded: %s %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));

    g_gl_context = context;

    lua_pushboolean(L, 1);
    lua_pushlightuserdata(L, context);
    return 3;
}

static int gl_clear(lua_State *L) {
    // Check if OpenGL context is valid
    int ret = check_gl_context(L);
    if (ret) return ret; // Returns nil and error message if context is invalid
    // Get the bitmask from Lua (e.g., GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    GLbitfield mask = (GLbitfield)luaL_checkinteger(L, 1);
    // Call glClear with the provided bitmask
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // test
    glClear(mask);

    return 0; // No return values on success
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

// Existing gl_destroy function
static int gl_destroy(lua_State *L) {
    if (g_gl_context) {
        SDL_GL_DestroyContext(g_gl_context);
        g_gl_context = NULL;
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

static int gl_delete_shader(lua_State *L) {
    int ret = check_gl_context(L);
    if (ret) return ret;
    GLuint shader = (GLuint)luaL_checkinteger(L, 1);
    glDeleteShader(shader);
    return 0;
}

static int gl_delete_program(lua_State *L) {
    int ret = check_gl_context(L);
    if (ret) return ret;
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    glDeleteProgram(program);
    return 0;
}

// Lua: gl.gen_textures() -> texture_id
static int gl_gen_textures(lua_State *L) {
    GLuint texture;
    glGenTextures(1, &texture);
    lua_pushinteger(L, texture);
    return 1;
}

// Lua: gl.bind_texture(target, texture_id)
static int gl_bind_texture(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    GLuint texture = (GLuint)luaL_checkinteger(L, 2);
    glBindTexture(target, texture);
    return 0;
}

// Lua: gl.tex_image_2d(target, level, internal_format, width, height, border, format, type, data)
static int gl_tex_image_2d(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    GLint level = (GLint)luaL_checkinteger(L, 2);
    GLint internal_format = (GLint)luaL_checkinteger(L, 3);
    GLsizei width = (GLsizei)luaL_checkinteger(L, 4);
    GLsizei height = (GLsizei)luaL_checkinteger(L, 5);
    GLint border = (GLint)luaL_checkinteger(L, 6);
    GLenum format = (GLenum)luaL_checkinteger(L, 7);
    GLenum type = (GLenum)luaL_checkinteger(L, 8);
    void *data = lua_touserdata(L, 9);
    glTexImage2D(target, level, internal_format, width, height, border, format, type, data);
    return 0;
}

// Lua: gl.tex_parameter_i(target, pname, param)
static int gl_tex_parameter_i(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    GLenum pname = (GLenum)luaL_checkinteger(L, 2);
    GLint param = (GLint)luaL_checkinteger(L, 3);
    glTexParameteri(target, pname, param);
    return 0;
}

// Lua: gl.draw_elements(mode, count, type, offset)
static int gl_draw_elements(lua_State *L) {
    GLenum mode = (GLenum)luaL_checkinteger(L, 1);
    GLsizei count = (GLsizei)luaL_checkinteger(L, 2);
    GLenum type = (GLenum)luaL_checkinteger(L, 3);
    GLintptr offset = (GLintptr)luaL_checkinteger(L, 4);
    glDrawElements(mode, count, type, (const void *)offset);
    return 0;
}

// Helper to check cglm mat4 userdata
static mat4* check_mat4(lua_State *L, int idx) {
    void *ud = luaL_checkudata(L, idx, "cglm.mat4");
    luaL_argcheck(L, ud != NULL, idx, "cglm.mat4 expected");
    return (mat4*)ud;
}

// Lua: gl.uniform_matrix4fv(location, count, transpose, matrix)
static int gl_uniform_matrix4fv(lua_State *L) {
    GLint location = (GLint)luaL_checkinteger(L, 1);
    GLsizei count = (GLsizei)luaL_checkinteger(L, 2);
    GLboolean transpose = (GLboolean)luaL_checkinteger(L, 3);
    
    // Check if the 4th argument is a cglm mat4 userdata
    if (luaL_testudata(L, 4, "cglm.mat4")) {
        mat4 *matrix = check_mat4(L, 4);
        glUniformMatrix4fv(location, count, transpose, (const GLfloat *)(*matrix));
        // printf("Using cglm.mat4: [0][0]=%f, [1][1]=%f, [3][0]=%f, [3][1]=%f\n",
        //        (*matrix)[0][0], (*matrix)[1][1], (*matrix)[3][0], (*matrix)[3][1]);
    } else {
        // Fallback to string-based version with logging
        const char *matrix = luaL_checkstring(L, 4);
        size_t len = lua_rawlen(L, 4);
        // printf("String-based matrix: length=%zu bytes\n", len);
        if (len != 64) {
            // printf("Warning: Expected 64 bytes (16 floats), got %zu\n", len);
        } else {
            const GLfloat *float_matrix = (const GLfloat *)matrix;
            // printf("Matrix values: [0]=%f, [1]=%f, [5]=%f, [12]=%f, [13]=%f, [15]=%f\n",
            //        float_matrix[0], float_matrix[1], float_matrix[5],
            //        float_matrix[12], float_matrix[13], float_matrix[15]);
        }
        glUniformMatrix4fv(location, count, transpose, (const GLfloat *)matrix);
    }
    return 0;
}

// Lua: gl.get_uniform_location(program, name) -> location
static int gl_get_uniform_location(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);
    GLint location = glGetUniformLocation(program, name);
    lua_pushinteger(L, location);
    return 1;
}

// Lua: gl.uniform1i(location, value)
static int gl_uniform1i(lua_State *L) {
    GLint location = (GLint)luaL_checkinteger(L, 1);
    GLint value = (GLint)luaL_checkinteger(L, 2);
    glUniform1i(location, value);
    return 0;
}

// Lua: gl.active_texture(texture_unit)
static int gl_active_texture(lua_State *L) {
    GLenum texture_unit = (GLenum)luaL_checkinteger(L, 1);
    glActiveTexture(texture_unit);
    return 0;
}

static int gl_enable(lua_State *L) {
    GLenum cap = (GLenum)luaL_checkinteger(L, 1);
    glEnable(cap);
    return 0;
}

static int gl_get_error(lua_State *L) {
    GLenum err = glGetError();
    lua_pushinteger(L, err);
    return 1;
}

static int gl_blend_func(lua_State *L) {
    GLenum sfactor = (GLenum)luaL_checkinteger(L, 1);
    GLenum dfactor = (GLenum)luaL_checkinteger(L, 2);
    glBlendFunc(sfactor, dfactor);
    return 0;
}

// Lua: gl.dummy_uniform_matrix4fv(location, count, transpose) (kept for reference)
// working test.
static int gl_dummy_uniform_matrix4fv(lua_State *L) {
    GLint location = (GLint)luaL_checkinteger(L, 1);
    GLsizei count = (GLsizei)luaL_checkinteger(L, 2);
    GLboolean transpose = (GLboolean)lua_toboolean(L, 3);
    
    // Hardcoded orthographic projection matrix for 800x600
    GLfloat matrix[] = {
        0.0025f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.003333f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    };
    
    glUniformMatrix4fv(location, count, transpose, matrix);
    return 0;
}

// Lua: gl.uniform4f(location, x, y, z, w)
static int gl_uniform4f(lua_State *L) {
    GLint location = (GLint)luaL_checkinteger(L, 1);
    GLfloat x = (GLfloat)luaL_checknumber(L, 2);
    GLfloat y = (GLfloat)luaL_checknumber(L, 3);
    GLfloat z = (GLfloat)luaL_checknumber(L, 4);
    GLfloat w = (GLfloat)luaL_checknumber(L, 5);
    glUniform4f(location, x, y, z, w);
    return 0;
}

// Lua: gl.delete_textures(textures)
static int gl_delete_textures(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    GLsizei n = lua_rawlen(L, 1);
    GLuint *textures = (GLuint *)malloc(n * sizeof(GLuint));
    if (!textures) {
        luaL_error(L, "Failed to allocate memory for textures");
        return 0;
    }

    for (int i = 0; i < n; i++) {
        lua_rawgeti(L, 1, i + 1);
        textures[i] = (GLuint)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    glDeleteTextures(n, textures);
    free(textures);
    return 0;
}

// Lua: gl.delete_buffers(buffers)
static int gl_delete_buffers(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    GLsizei n = lua_rawlen(L, 1);
    GLuint *buffers = (GLuint *)malloc(n * sizeof(GLuint));
    if (!buffers) {
        luaL_error(L, "Failed to allocate memory for buffers");
        return 0;
    }

    for (int i = 0; i < n; i++) {
        lua_rawgeti(L, 1, i + 1);
        buffers[i] = (GLuint)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    glDeleteBuffers(n, buffers);
    free(buffers);
    return 0;
}

// Lua: gl.delete_vertex_arrays(arrays)
static int gl_delete_vertex_arrays(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    GLsizei n = lua_rawlen(L, 1);
    GLuint *arrays = (GLuint *)malloc(n * sizeof(GLuint));
    if (!arrays) {
        luaL_error(L, "Failed to allocate memory for vertex arrays");
        return 0;
    }

    for (int i = 0; i < n; i++) {
        lua_rawgeti(L, 1, i + 1);
        arrays[i] = (GLuint)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    glDeleteVertexArrays(n, arrays);
    free(arrays);
    return 0;
}

// Lua: gl.uniform1f(location, value)
static int gl_uniform1f(lua_State *L) {
    GLint location = (GLint)luaL_checkinteger(L, 1);
    GLfloat value = (GLfloat)luaL_checknumber(L, 2);
    glUniform1f(location, value);
    return 0;
}

// Lua: gl.disable(cap) -> bool, err_msg
static int gl_disable(lua_State *L) {
    GLenum cap = (GLenum)luaL_checkinteger(L, 1); // Expect GLenum like GL_CULL_FACE
    glDisable(cap);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "OpenGL error in gl_disable: %d", err);
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

// Lua: gl.cull_face(mode) -> bool, err_msg
static int gl_cull_face(lua_State *L) {
    GLenum mode = (GLenum)luaL_checkinteger(L, 1); // Expect GLenum like GL_FALSE
    glCullFace(mode);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "OpenGL error in gl_cull_face: %d", err);
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

// Lua: gl.polygon_mode(face, mode)
static int gl_polygon_mode(lua_State *L) {
    GLenum face = (GLenum)luaL_checkinteger(L, 1);
    GLenum mode = (GLenum)luaL_checkinteger(L, 2);
    glPolygonMode(face, mode);
    return push_gl_error(L, "glPolygonMode");
}


static const struct luaL_Reg gl_lib[] = {
    {"init", gl_init},
    {"destroy", gl_destroy},
    {"get_gl_context", gl_get_gl_context},
    {"clear", gl_clear},
    {"clear_color", gl_clear_color},
    {"viewport", gl_viewport},
    {"create_shader", gl_create_shader},
    {"delete_shader", gl_delete_shader},
    {"shader_source", gl_shader_source},
    {"compile_shader", gl_compile_shader},
    {"create_program", gl_create_program},
    {"delete_program", gl_delete_program},
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

    {"gen_textures", gl_gen_textures},
    {"bind_texture", gl_bind_texture},
    {"tex_image_2d", gl_tex_image_2d},
    {"tex_parameter_i", gl_tex_parameter_i},
    {"draw_elements", gl_draw_elements},

    {"uniform_matrix4fv", gl_uniform_matrix4fv},
    {"get_uniform_location", gl_get_uniform_location},

    {"active_texture", gl_active_texture},
    {"uniform1i", gl_uniform1i},
    {"uniform1f", gl_uniform1f},
    {"uniform4f", gl_uniform4f},
    {"enable", gl_enable},
    {"disable", gl_disable},     // gl.disable
    {"get_error", gl_get_error},
    {"blend_func", gl_blend_func},
    {"dummy_uniform_matrix4fv", gl_dummy_uniform_matrix4fv},
    {"delete_textures", gl_delete_textures},
    {"delete_buffers", gl_delete_buffers},

    {"delete_vertex_arrays", gl_delete_vertex_arrays},
    {"cull_face", gl_cull_face}, // This is the new gl_cull_face

    {"polygon_mode", gl_polygon_mode},

    
    {NULL, NULL}
};

int luaopen_module_gl(lua_State *L) {
    luaL_newlib(L, gl_lib);
    // Init gl_context as nil
    lua_pushnil(L);
    lua_setfield(L, -2, "gl_context");
    // Add constants
    lua_pushinteger(L, GL_VERSION); lua_setfield(L, -2, "VERSION");


    lua_pushinteger(L, GL_VERTEX_SHADER); lua_setfield(L, -2, "VERTEX_SHADER");
    lua_pushinteger(L, GL_FRAGMENT_SHADER); lua_setfield(L, -2, "FRAGMENT_SHADER");
    lua_pushinteger(L, GL_ARRAY_BUFFER); lua_setfield(L, -2, "ARRAY_BUFFER");
    lua_pushinteger(L, GL_STATIC_DRAW); lua_setfield(L, -2, "STATIC_DRAW");
    lua_pushinteger(L, GL_FLOAT); lua_setfield(L, -2, "FLOAT");
    lua_pushinteger(L, GL_TRIANGLES); lua_setfield(L, -2, "TRIANGLES");
    lua_pushinteger(L, GL_COLOR_BUFFER_BIT); lua_setfield(L, -2, "COLOR_BUFFER_BIT");
    lua_pushinteger(L, GL_DEPTH_BUFFER_BIT); lua_setfield(L, -2, "DEPTH_BUFFER_BIT");

    lua_pushinteger(L, GL_ELEMENT_ARRAY_BUFFER); lua_setfield(L, -2, "ELEMENT_ARRAY_BUFFER");
    lua_pushinteger(L, GL_UNSIGNED_INT); lua_setfield(L, -2, "UNSIGNED_INT");
    lua_pushinteger(L, GL_FALSE); lua_setfield(L, -2, "FALSE");
    lua_pushinteger(L, GL_TRUE); lua_setfield(L, -2, "TRUE");
    lua_pushinteger(L, GL_ONE); lua_setfield(L, -2, "ONE");
    lua_pushinteger(L, GL_RED); lua_setfield(L, -2, "RED");

    lua_pushinteger(L, GL_TEXTURE_2D); lua_setfield(L, -2, "TEXTURE_2D");
    lua_pushinteger(L, GL_TEXTURE_MIN_FILTER); lua_setfield(L, -2, "TEXTURE_MIN_FILTER");
    lua_pushinteger(L, GL_TEXTURE_MAG_FILTER); lua_setfield(L, -2, "TEXTURE_MAG_FILTER");
    lua_pushinteger(L, GL_NEAREST); lua_setfield(L, -2, "NEAREST");
    lua_pushinteger(L, GL_LINEAR); lua_setfield(L, -2, "LINEAR");
    lua_pushinteger(L, GL_RGB); lua_setfield(L, -2, "RGB");
    lua_pushinteger(L, GL_RGBA); lua_setfield(L, -2, "RGBA");
    lua_pushinteger(L, GL_UNSIGNED_BYTE); lua_setfield(L, -2, "UNSIGNED_BYTE");

    lua_pushinteger(L, GL_TEXTURE0); lua_setfield(L, -2, "TEXTURE0");
    lua_pushinteger(L, GL_TEXTURE1); lua_setfield(L, -2, "TEXTURE1");
    lua_pushinteger(L, GL_BLEND); lua_setfield(L, -2, "BLEND");
    lua_pushinteger(L, GL_SRC_ALPHA); lua_setfield(L, -2, "SRC_ALPHA");
    lua_pushinteger(L, GL_ONE_MINUS_SRC_ALPHA); lua_setfield(L, -2, "ONE_MINUS_SRC_ALPHA");
    lua_pushinteger(L, GL_ALPHA); lua_setfield(L, -2, "ALPHA");
    lua_pushinteger(L, GL_TEXTURE_WRAP_S); lua_setfield(L, -2, "TEXTURE_WRAP_S");
    lua_pushinteger(L, GL_TEXTURE_WRAP_T); lua_setfield(L, -2, "TEXTURE_WRAP_T");
    lua_pushinteger(L, GL_CLAMP_TO_EDGE); lua_setfield(L, -2, "CLAMP_TO_EDGE");
    lua_pushinteger(L, GL_DYNAMIC_DRAW); lua_setfield(L, -2, "DYNAMIC_DRAW");

    lua_pushinteger(L, GL_DEPTH_TEST); lua_setfield(L, -2, "DEPTH_TEST");

    lua_pushinteger(L, GL_CULL_FACE); lua_setfield(L, -2, "CULL_FACE");
    lua_pushinteger(L, GL_BACK); lua_setfield(L, -2, "BACK");
    lua_pushinteger(L, GL_FRONT_AND_BACK); lua_setfield(L, -2, "FRONT_AND_BACK");
    lua_pushinteger(L, GL_LINE); lua_setfield(L, -2, "LINE");
    lua_pushinteger(L, GL_LESS); lua_setfield(L, -2, "LESS");
    lua_pushinteger(L, GL_FRONT); lua_setfield(L, -2, "FRONT");

    return 1;
}