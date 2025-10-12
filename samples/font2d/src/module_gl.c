// module_gl.c
#include "module_gl.h"
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <string.h>

static int gl_init(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        lua_pushboolean(L, 0);
        lua_pushnil(L);
        lua_pushstring(L, "Expected light userdata for window");
        return 3;
    }
    SDL_Window *window = (SDL_Window *)lua_touserdata(L, 1);
    if (!window) {
        lua_pushboolean(L, 0);
        lua_pushnil(L);
        lua_pushstring(L, "Invalid window handle");
        return 3;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        lua_pushboolean(L, 0);
        lua_pushnil(L);
        lua_pushstring(L, SDL_GetError());
        return 3;
    }

    if (SDL_GL_MakeCurrent(window, context) < 0) {
        SDL_GL_DestroyContext(context);
        lua_pushboolean(L, 0);
        lua_pushnil(L);
        lua_pushstring(L, SDL_GetError());
        return 3;
    }

    int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
    if (version == 0) {
        SDL_GL_DestroyContext(context);
        lua_pushboolean(L, 0);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to initialize GLAD");
        return 3;
    }

    lua_pushboolean(L, 1);
    lua_pushlightuserdata(L, context);
    return 2;
}

static int gl_destroy_context(lua_State *L) {
    SDL_GLContext context = (SDL_GLContext)lua_touserdata(L, 1);
    SDL_GL_DestroyContext(context);
    return 0;
}

static int gl_get_string(lua_State *L) {
    GLenum name = (GLenum)luaL_checkinteger(L, 1);
    const GLubyte *str = glGetString(name);
    lua_pushstring(L, (const char *)str);
    return 1;
}

static int gl_gen_textures(lua_State *L) {
    GLsizei n = (GLsizei)luaL_checkinteger(L, 1);
    GLuint *textures = (GLuint *)malloc(n * sizeof(GLuint));
    glGenTextures(n, textures);
    lua_newtable(L);
    for (GLsizei i = 0; i < n; ++i) {
        lua_pushinteger(L, textures[i]);
        lua_rawseti(L, -2, i + 1);
    }
    free(textures);
    return 1;
}

static int gl_delete_textures(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    int n = luaL_len(L, 1);
    GLuint *textures = (GLuint *)malloc(n * sizeof(GLuint));
    for (int i = 1; i <= n; ++i) {
        lua_geti(L, 1, i);
        textures[i - 1] = (GLuint)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    glDeleteTextures(n, textures);
    free(textures);
    return 0;
}

static int gl_gen_buffers(lua_State *L) {
    GLsizei n = (GLsizei)luaL_checkinteger(L, 1);
    GLuint *buffers = (GLuint *)malloc(n * sizeof(GLuint));
    glGenBuffers(n, buffers);
    lua_newtable(L);
    for (GLsizei i = 0; i < n; ++i) {
        lua_pushinteger(L, buffers[i]);
        lua_rawseti(L, -2, i + 1);
    }
    free(buffers);
    return 1;
}

static int gl_delete_buffers(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    int n = luaL_len(L, 1);
    GLuint *buffers = (GLuint *)malloc(n * sizeof(GLuint));
    for (int i = 1; i <= n; ++i) {
        lua_geti(L, 1, i);
        buffers[i - 1] = (GLuint)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    glDeleteBuffers(n, buffers);
    free(buffers);
    return 0;
}

static int gl_gen_vertex_arrays(lua_State *L) {
    GLsizei n = (GLsizei)luaL_checkinteger(L, 1);
    GLuint *vaos = (GLuint *)malloc(n * sizeof(GLuint));
    glGenVertexArrays(n, vaos);
    lua_newtable(L);
    for (GLsizei i = 0; i < n; ++i) {
        lua_pushinteger(L, vaos[i]);
        lua_rawseti(L, -2, i + 1);
    }
    free(vaos);
    return 1;
}

static int gl_delete_vertex_arrays(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    int n = luaL_len(L, 1);
    GLuint *vaos = (GLuint *)malloc(n * sizeof(GLuint));
    for (int i = 1; i <= n; ++i) {
        lua_geti(L, 1, i);
        vaos[i - 1] = (GLuint)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    glDeleteVertexArrays(n, vaos);
    free(vaos);
    return 0;
}

static int gl_bind_texture(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    GLuint texture = (GLuint)luaL_checkinteger(L, 2);
    glBindTexture(target, texture);
    return 0;
}

static int gl_tex_image_2d(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    GLint level = (GLint)luaL_checkinteger(L, 2);
    GLint internalformat = (GLint)luaL_checkinteger(L, 3);
    GLsizei width = (GLsizei)luaL_checkinteger(L, 4);
    GLsizei height = (GLsizei)luaL_checkinteger(L, 5);
    GLint border = (GLint)luaL_checkinteger(L, 6);
    GLenum format = (GLenum)luaL_checkinteger(L, 7);
    GLenum type = (GLenum)luaL_checkinteger(L, 8);
    size_t data_len;
    const char *data = luaL_checklstring(L, 9, &data_len);
    glTexImage2D(target, level, internalformat, width, height, border, format, type, data);
    return 0;
}

static int gl_tex_parameter_i(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    GLenum pname = (GLenum)luaL_checkinteger(L, 2);
    GLint param = (GLint)luaL_checkinteger(L, 3);
    glTexParameteri(target, pname, param);
    return 0;
}

static int gl_create_shader(lua_State *L) {
    GLenum type = (GLenum)luaL_checkinteger(L, 1);
    GLuint shader = glCreateShader(type);
    lua_pushinteger(L, shader);
    return 1;
}

static int gl_shader_source(lua_State *L) {
    GLuint shader = (GLuint)luaL_checkinteger(L, 1);
    const char *source = luaL_checkstring(L, 2);
    GLint length = (GLint)strlen(source);
    glShaderSource(shader, 1, &source, &length);
    return 0;
}

static int gl_compile_shader(lua_State *L) {
    GLuint shader = (GLuint)luaL_checkinteger(L, 1);
    glCompileShader(shader);
    return 0;
}

static int gl_get_shader_iv(lua_State *L) {
    GLuint shader = (GLuint)luaL_checkinteger(L, 1);
    GLenum pname = (GLenum)luaL_checkinteger(L, 2);
    GLint params;
    glGetShaderiv(shader, pname, &params);
    lua_pushinteger(L, params);
    return 1;
}

static int gl_get_shader_info_log(lua_State *L) {
    GLuint shader = (GLuint)luaL_checkinteger(L, 1);
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    lua_pushstring(L, infoLog);
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
    return 0;
}

static int gl_get_program_iv(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    GLenum pname = (GLenum)luaL_checkinteger(L, 2);
    GLint params;
    glGetProgramiv(program, pname, &params);
    lua_pushinteger(L, params);
    return 1;
}

static int gl_get_program_info_log(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    char infoLog[512];
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    lua_pushstring(L, infoLog);
    return 1;
}

static int gl_delete_shader(lua_State *L) {
    GLuint shader = (GLuint)luaL_checkinteger(L, 1);
    glDeleteShader(shader);
    return 0;
}

static int gl_bind_vertex_array(lua_State *L) {
    GLuint array = (GLuint)luaL_checkinteger(L, 1);
    glBindVertexArray(array);
    return 0;
}

static int gl_bind_buffer(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    GLuint buffer = (GLuint)luaL_checkinteger(L, 2);
    glBindBuffer(target, buffer);
    return 0;
}

static int gl_enable_vertex_attrib_array(lua_State *L) {
    GLuint index = (GLuint)luaL_checkinteger(L, 1);
    glEnableVertexAttribArray(index);
    return 0;
}

static int gl_vertex_attrib_pointer(lua_State *L) {
    GLuint index = (GLuint)luaL_checkinteger(L, 1);
    GLint size = (GLint)luaL_checkinteger(L, 2);
    GLenum type = (GLenum)luaL_checkinteger(L, 3);
    GLboolean normalized = (GLboolean)lua_toboolean(L, 4);
    GLsizei stride = (GLsizei)luaL_checkinteger(L, 5);
    uintptr_t pointer = (uintptr_t)luaL_checkinteger(L, 6);
    glVertexAttribPointer(index, size, type, normalized, stride, (const void *)pointer);
    return 0;
}

static int gl_enable(lua_State *L) {
    GLenum cap = (GLenum)luaL_checkinteger(L, 1);
    glEnable(cap);
    return 0;
}

static int gl_blend_func(lua_State *L) {
    GLenum sfactor = (GLenum)luaL_checkinteger(L, 1);
    GLenum dfactor = (GLenum)luaL_checkinteger(L, 2);
    glBlendFunc(sfactor, dfactor);
    return 0;
}

static int gl_viewport(lua_State *L) {
    GLint x = (GLint)luaL_checkinteger(L, 1);
    GLint y = (GLint)luaL_checkinteger(L, 2);
    GLsizei width = (GLsizei)luaL_checkinteger(L, 3);
    GLsizei height = (GLsizei)luaL_checkinteger(L, 4);
    glViewport(x, y, width, height);
    return 0;
}

static int gl_clear_color(lua_State *L) {
    GLfloat red = (GLfloat)luaL_checknumber(L, 1);
    GLfloat green = (GLfloat)luaL_checknumber(L, 2);
    GLfloat blue = (GLfloat)luaL_checknumber(L, 3);
    GLfloat alpha = (GLfloat)luaL_checknumber(L, 4);
    glClearColor(red, green, blue, alpha);
    return 0;
}

static int gl_clear(lua_State *L) {
    GLbitfield mask = (GLbitfield)luaL_checkinteger(L, 1);
    glClear(mask);
    return 0;
}

static int gl_use_program(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    glUseProgram(program);
    return 0;
}

static int gl_get_uniform_location(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);
    GLint location = glGetUniformLocation(program, name);
    lua_pushinteger(L, location);
    return 1;
}

static int gl_uniform_1i(lua_State *L) {
    GLint location = (GLint)luaL_checkinteger(L, 1);
    GLint v0 = (GLint)luaL_checkinteger(L, 2);
    glUniform1i(location, v0);
    return 0;
}

static int gl_uniform_4f(lua_State *L) {
    GLint location = (GLint)luaL_checkinteger(L, 1);
    GLfloat v0 = (GLfloat)luaL_checknumber(L, 2);
    GLfloat v1 = (GLfloat)luaL_checknumber(L, 3);
    GLfloat v2 = (GLfloat)luaL_checknumber(L, 4);
    GLfloat v3 = (GLfloat)luaL_checknumber(L, 5);
    glUniform4f(location, v0, v1, v2, v3);
    return 0;
}

static int gl_active_texture(lua_State *L) {
    GLenum texture = (GLenum)luaL_checkinteger(L, 1);
    glActiveTexture(texture);
    return 0;
}

static int gl_buffer_data(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int num_elements = luaL_len(L, 2);
    GLfloat *data = (GLfloat *)malloc(num_elements * sizeof(GLfloat));
    for (int i = 1; i <= num_elements; ++i) {
        lua_geti(L, 2, i);
        data[i - 1] = (GLfloat)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    GLenum usage = (GLenum)luaL_checkinteger(L, 3);
    glBufferData(target, num_elements * sizeof(GLfloat), data, usage);
    free(data);
    return 0;
}

static int gl_draw_arrays(lua_State *L) {
    GLenum mode = (GLenum)luaL_checkinteger(L, 1);
    GLint first = (GLint)luaL_checkinteger(L, 2);
    GLsizei count = (GLsizei)luaL_checkinteger(L, 3);
    glDrawArrays(mode, first, count);
    return 0;
}

static int gl_delete_program(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    glDeleteProgram(program);
    return 0;
}

static const struct luaL_Reg gl_lib[] = {
    {"init", gl_init},
    {"destroy_context", gl_destroy_context},
    {"get_string", gl_get_string},
    {"gen_textures", gl_gen_textures},
    {"delete_textures", gl_delete_textures},
    {"gen_buffers", gl_gen_buffers},
    {"delete_buffers", gl_delete_buffers},
    {"gen_vertex_arrays", gl_gen_vertex_arrays},
    {"delete_vertex_arrays", gl_delete_vertex_arrays},
    {"bind_texture", gl_bind_texture},
    {"tex_image_2d", gl_tex_image_2d},
    {"tex_parameter_i", gl_tex_parameter_i},
    {"create_shader", gl_create_shader},
    {"shader_source", gl_shader_source},
    {"compile_shader", gl_compile_shader},
    {"get_shader_iv", gl_get_shader_iv},
    {"get_shader_info_log", gl_get_shader_info_log},
    {"create_program", gl_create_program},
    {"attach_shader", gl_attach_shader},
    {"link_program", gl_link_program},
    {"get_program_iv", gl_get_program_iv},
    {"get_program_info_log", gl_get_program_info_log},
    {"delete_shader", gl_delete_shader},
    {"bind_vertex_array", gl_bind_vertex_array},
    {"bind_buffer", gl_bind_buffer},
    {"enable_vertex_attrib_array", gl_enable_vertex_attrib_array},
    {"vertex_attrib_pointer", gl_vertex_attrib_pointer},
    {"enable", gl_enable},
    {"blend_func", gl_blend_func},
    {"viewport", gl_viewport},
    {"clear_color", gl_clear_color},
    {"clear", gl_clear},
    {"use_program", gl_use_program},
    {"get_uniform_location", gl_get_uniform_location},
    {"uniform_1i", gl_uniform_1i},
    {"uniform_4f", gl_uniform_4f},
    {"active_texture", gl_active_texture},
    {"buffer_data", gl_buffer_data},
    {"draw_arrays", gl_draw_arrays},
    {"delete_program", gl_delete_program},
    {NULL, NULL}
};

static void push_gl_constants(lua_State *L) {
    lua_pushinteger(L, GL_VERSION); lua_setfield(L, -2, "VERSION");
    lua_pushinteger(L, GL_TEXTURE_2D); lua_setfield(L, -2, "TEXTURE_2D");
    lua_pushinteger(L, GL_RED); lua_setfield(L, -2, "RED");
    lua_pushinteger(L, GL_UNSIGNED_BYTE); lua_setfield(L, -2, "UNSIGNED_BYTE");
    lua_pushinteger(L, GL_TEXTURE_MIN_FILTER); lua_setfield(L, -2, "TEXTURE_MIN_FILTER");
    lua_pushinteger(L, GL_TEXTURE_MAG_FILTER); lua_setfield(L, -2, "TEXTURE_MAG_FILTER");
    lua_pushinteger(L, GL_LINEAR); lua_setfield(L, -2, "LINEAR");
    lua_pushinteger(L, GL_VERTEX_SHADER); lua_setfield(L, -2, "VERTEX_SHADER");
    lua_pushinteger(L, GL_FRAGMENT_SHADER); lua_setfield(L, -2, "FRAGMENT_SHADER");
    lua_pushinteger(L, GL_COMPILE_STATUS); lua_setfield(L, -2, "COMPILE_STATUS");
    lua_pushinteger(L, GL_LINK_STATUS); lua_setfield(L, -2, "LINK_STATUS");
    lua_pushinteger(L, GL_ARRAY_BUFFER); lua_setfield(L, -2, "ARRAY_BUFFER");
    lua_pushinteger(L, GL_FLOAT); lua_setfield(L, -2, "FLOAT");
    lua_pushinteger(L, GL_FALSE); lua_setfield(L, -2, "FALSE");
    lua_pushinteger(L, GL_BLEND); lua_setfield(L, -2, "BLEND");
    lua_pushinteger(L, GL_SRC_ALPHA); lua_setfield(L, -2, "SRC_ALPHA");
    lua_pushinteger(L, GL_ONE_MINUS_SRC_ALPHA); lua_setfield(L, -2, "ONE_MINUS_SRC_ALPHA");
    lua_pushinteger(L, GL_COLOR_BUFFER_BIT); lua_setfield(L, -2, "COLOR_BUFFER_BIT");
    lua_pushinteger(L, GL_TEXTURE0); lua_setfield(L, -2, "TEXTURE0");
    lua_pushinteger(L, GL_DYNAMIC_DRAW); lua_setfield(L, -2, "DYNAMIC_DRAW");
    lua_pushinteger(L, GL_TRIANGLES); lua_setfield(L, -2, "TRIANGLES");
}

int luaopen_module_gl(lua_State *L) {
    luaL_newlib(L, gl_lib);
    push_gl_constants(L);
    return 1;
}