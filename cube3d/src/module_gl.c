// module_gl.c
#include <lauxlib.h>
#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <cglm/cglm.h>

// Global OpenGL context
static SDL_GLContext g_gl_context = NULL;

// Helper: Get window from Lua global
static SDL_Window *get_sdl_window(lua_State *L) {
    lua_getglobal(L, "sdl_window");
    SDL_Window *window = (SDL_Window *)lua_topointer(L, -1);
    lua_pop(L, 1);
    return window;
}

// Lua: gl.init() -> bool, err_msg (creates context, loads GLAD, stores as gl.gl_context)
static int gl_init(lua_State *L) {
    SDL_Window *window = get_sdl_window(L);
    if (!window) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "No window found");
        return 2;
    }
    Uint32 flags = SDL_GetWindowFlags(window);
    if (!(flags & SDL_WINDOW_OPENGL)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Window is not an OpenGL window");
        return 2;
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
        char err_msg[512];
        snprintf(err_msg, 512, "SDL_GL_CreateContext failed: %s", SDL_GetError());
        lua_pushstring(L, err_msg);
        return 2;
    }

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Failed to load OpenGL functions");
        SDL_GL_DestroyContext(context);
        return 2;
    }

    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Failed to set VSync: %s\n", SDL_GetError());
    }

    printf("OpenGL loaded: %s %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));

    // Store context in static variable
    g_gl_context = context;

    lua_pushboolean(L, 1);
    lua_pushlightuserdata(L, context);
    return 2; // Return success flag and context
}

// Lua: gl.quit()
static int gl_quit(lua_State *L) {
    if (g_gl_context) {
        SDL_GL_DestroyContext(g_gl_context);
        g_gl_context = NULL;
    }
    return 0;
}

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

// Lua: gl.get_string(name) -> string
static int gl_get_string(lua_State *L) {
    GLenum name = (GLenum)luaL_checkinteger(L, 1);
    const GLubyte *str = glGetString(name);
    if (str) {
        lua_pushstring(L, (const char *)str);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// Lua: gl.viewport(x, y, width, height)
static int gl_viewport(lua_State *L) {
    GLint x = (GLint)luaL_checkinteger(L, 1);
    GLint y = (GLint)luaL_checkinteger(L, 2);
    GLsizei width = (GLsizei)luaL_checkinteger(L, 3);
    GLsizei height = (GLsizei)luaL_checkinteger(L, 4);
    glViewport(x, y, width, height);
    return push_gl_error(L, "glViewport");
}

// Lua: gl.create_shader(type) -> shader_id
static int gl_create_shader(lua_State *L) {
    GLenum type = (GLenum)luaL_checkinteger(L, 1);
    GLuint shader = glCreateShader(type);
    lua_pushinteger(L, shader);
    return 1;
}

// Lua: gl.shader_source(shader, source)
static int gl_shader_source(lua_State *L) {
    GLuint shader = (GLuint)luaL_checkinteger(L, 1);
    const char *source = luaL_checkstring(L, 2);
    glShaderSource(shader, 1, &source, NULL);
    return 0;
}

// Lua: gl.compile_shader(shader) -> bool, err_msg
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

// Lua: gl.create_program() -> program_id
static int gl_create_program(lua_State *L) {
    GLuint program = glCreateProgram();
    lua_pushinteger(L, program);
    return 1;
}

// Lua: gl.attach_shader(program, shader)
static int gl_attach_shader(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    GLuint shader = (GLuint)luaL_checkinteger(L, 2);
    glAttachShader(program, shader);
    return 0;
}

// Lua: gl.link_program(program) -> bool, err_msg
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

// Lua: gl.delete_shader(shader)
static int gl_delete_shader(lua_State *L) {
    GLuint shader = (GLuint)luaL_checkinteger(L, 1);
    glDeleteShader(shader);
    return 0;
}

// Lua: gl.gen_vertex_arrays(n) -> table of vaos
static int gl_gen_vertex_arrays(lua_State *L) {
    GLsizei n = (GLsizei)luaL_checkinteger(L, 1);
    GLuint *vaos = (GLuint *)malloc(n * sizeof(GLuint));
    glGenVertexArrays(n, vaos);
    lua_newtable(L);
    for (int i = 0; i < n; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, vaos[i]);
        lua_settable(L, -3);
    }
    free(vaos);
    return 1;
}

// Lua: gl.gen_buffers(n) -> table of buffers
static int gl_gen_buffers(lua_State *L) {
    GLsizei n = (GLsizei)luaL_checkinteger(L, 1);
    GLuint *buffers = (GLuint *)malloc(n * sizeof(GLuint));
    glGenBuffers(n, buffers);
    lua_newtable(L);
    for (int i = 0; i < n; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, buffers[i]);
        lua_settable(L, -3);
    }
    free(buffers);
    return 1;
}

// Lua: gl.bind_vertex_array(vao)
static int gl_bind_vertex_array(lua_State *L) {
    GLuint vao = (GLuint)luaL_checkinteger(L, 1);
    glBindVertexArray(vao);
    return 0;
}

// Lua: gl.bind_buffer(target, buffer)
static int gl_bind_buffer(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    GLuint buffer = (GLuint)luaL_checkinteger(L, 2);
    glBindBuffer(target, buffer);
    return 0;
}

// Lua: gl.buffer_data(target, data, usage)
static int gl_buffer_data(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    GLenum usage = (GLenum)luaL_checkinteger(L, 3);

    // Convert Lua table to float array
    lua_len(L, 2);
    size_t len = (size_t)lua_tointeger(L, -1);
    lua_pop(L, 1);
    float *data = (float *)malloc(len * sizeof(float));
    for (size_t i = 0; i < len; i++) {
        lua_geti(L, 2, i + 1);
        data[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    glBufferData(target, len * sizeof(float), data, usage);
    free(data);
    return push_gl_error(L, "glBufferData");
}

// Lua: gl.buffer_data_uint(target, data, usage)
static int gl_buffer_data_uint(lua_State *L) {
    GLenum target = (GLenum)luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    GLenum usage = (GLenum)luaL_checkinteger(L, 3);

    lua_len(L, 2);
    size_t len = (size_t)lua_tointeger(L, -1);
    lua_pop(L, 1);
    unsigned int *data = (unsigned int *)malloc(len * sizeof(unsigned int));
    for (size_t i = 0; i < len; i++) {
        lua_geti(L, 2, i + 1);
        data[i] = (unsigned int)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    glBufferData(target, len * sizeof(unsigned int), data, usage);
    free(data);
    return push_gl_error(L, "glBufferData_uint");
}

// Lua: gl.vertex_attrib_pointer(index, size, type, normalized, stride, offset)
static int gl_vertex_attrib_pointer(lua_State *L) {
    GLuint index = (GLuint)luaL_checkinteger(L, 1);
    GLint size = (GLint)luaL_checkinteger(L, 2);
    GLenum type = (GLenum)luaL_checkinteger(L, 3);
    GLboolean normalized = (GLboolean)luaL_checkinteger(L, 4);
    GLsizei stride = (GLsizei)luaL_checkinteger(L, 5);
    size_t offset = (size_t)luaL_checkinteger(L, 6);
    glVertexAttribPointer(index, size, type, normalized, stride, (void *)offset);
    return push_gl_error(L, "glVertexAttribPointer");
}

// Lua: gl.enable_vertex_attrib_array(index)
static int gl_enable_vertex_attrib_array(lua_State *L) {
    GLuint index = (GLuint)luaL_checkinteger(L, 1);
    glEnableVertexAttribArray(index);
    return 0;
}

// Lua: gl.enable(cap)
static int gl_enable(lua_State *L) {
    GLenum cap = (GLenum)luaL_checkinteger(L, 1);
    glEnable(cap);
    return push_gl_error(L, "glEnable");
}

// Lua: gl.get_uniform_location(program, name) -> location
static int gl_get_uniform_location(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);
    GLint location = glGetUniformLocation(program, name);
    lua_pushinteger(L, location);
    return 1;
}

// Lua: gl.use_program(program)
static int gl_use_program(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    glUseProgram(program);
    return 0;
}

// Lua: gl.uniform_matrix4fv(location, count, transpose, matrix)
static int gl_uniform_matrix4fv(lua_State *L) {
    GLint location = (GLint)luaL_checkinteger(L, 1);
    GLsizei count = (GLsizei)luaL_checkinteger(L, 2);
    GLboolean transpose = (GLboolean)luaL_checkinteger(L, 3); // Use integer instead of boolean
    luaL_checktype(L, 4, LUA_TTABLE);
    float matrix[16];
    for (int i = 0; i < 16; i++) {
        lua_geti(L, 4, i + 1);
        matrix[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    // Debug: Print matrix
    printf("Uniform Matrix4fv (transpose=%d):\n", transpose);
    for (int i = 0; i < 4; i++) {
        printf("[%f, %f, %f, %f]\n", matrix[i*4], matrix[i*4+1], matrix[i*4+2], matrix[i*4+3]);
    }
    glUniformMatrix4fv(location, count, transpose, matrix);
    return push_gl_error(L, "glUniformMatrix4fv");
}

// Lua: gl.polygon_mode(face, mode)
static int gl_polygon_mode(lua_State *L) {
    GLenum face = (GLenum)luaL_checkinteger(L, 1);
    GLenum mode = (GLenum)luaL_checkinteger(L, 2);
    glPolygonMode(face, mode);
    return push_gl_error(L, "glPolygonMode");
}

// Lua: gl.clear_color(r, g, b, a)
static int gl_clear_color(lua_State *L) {
    GLfloat r = (GLfloat)luaL_checknumber(L, 1);
    GLfloat g = (GLfloat)luaL_checknumber(L, 2);
    GLfloat b = (GLfloat)luaL_checknumber(L, 3);
    GLfloat a = (GLfloat)luaL_checknumber(L, 4);
    glClearColor(r, g, b, a);
    return 0;
}

// Lua: gl.clear(mask)
static int gl_clear(lua_State *L) {
    GLbitfield mask = (GLbitfield)luaL_checkinteger(L, 1);
    glClear(mask);
    return push_gl_error(L, "glClear");
}

// Lua: gl.draw_elements(mode, count, type, offset)
static int gl_draw_elements(lua_State *L) {
    GLenum mode = (GLenum)luaL_checkinteger(L, 1);
    GLsizei count = (GLsizei)luaL_checkinteger(L, 2);
    GLenum type = (GLenum)luaL_checkinteger(L, 3);
    size_t offset = (size_t)luaL_checkinteger(L, 4);
    glDrawElements(mode, count, type, (void *)offset);
    return push_gl_error(L, "glDrawElements");
}

// Lua: gl.delete_vertex_arrays(vaos)
static int gl_delete_vertex_arrays(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_len(L, 1);
    size_t len = (size_t)lua_tointeger(L, -1);
    lua_pop(L, 1);
    GLuint *vaos = (GLuint *)malloc(len * sizeof(GLuint));
    for (size_t i = 0; i < len; i++) {
        lua_geti(L, 1, i + 1);
        vaos[i] = (GLuint)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    glDeleteVertexArrays((GLsizei)len, vaos);
    free(vaos);
    return 0;
}

// Lua: gl.delete_buffers(buffers)
static int gl_delete_buffers(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_len(L, 1);
    size_t len = (size_t)lua_tointeger(L, -1);
    lua_pop(L, 1);
    GLuint *buffers = (GLuint *)malloc(len * sizeof(GLuint));
    for (size_t i = 0; i < len; i++) {
        lua_geti(L, 1, i + 1);
        buffers[i] = (GLuint)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    glDeleteBuffers((GLsizei)len, buffers);
    free(buffers);
    return 0;
}

// Lua: gl.delete_program(program)
static int gl_delete_program(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    glDeleteProgram(program);
    return 0;
}

// Lua: gl.get_error() -> error_code or 0
static int gl_get_error(lua_State *L) {
    GLenum err = glGetError();
    lua_pushinteger(L, err);
    return 1;
}

// Lua: gl.depth_func(func)
static int gl_depth_func(lua_State *L) {
    GLenum func = (GLenum)luaL_checkinteger(L, 1);
    glDepthFunc(func);
    return push_gl_error(L, "glDepthFunc");
}

// Lua: gl.get_integer(pname) -> value
static int gl_get_integer(lua_State *L) {
    GLenum pname = (GLenum)luaL_checkinteger(L, 1);
    GLint value;
    glGetIntegerv(pname, &value);
    lua_pushinteger(L, value);
    return 1;
}

// Lua: gl.cull_face(enable, mode) -> bool, err_msg
static int gl_cull_face(lua_State *L) {
    GLboolean enable = (GLboolean)lua_toboolean(L, 1);
    GLenum mode = (GLenum)luaL_optinteger(L, 2, GL_BACK);
    if (enable) {
        glEnable(GL_CULL_FACE);
        glCullFace(mode);
    } else {
        glDisable(GL_CULL_FACE);
    }
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "OpenGL error in gl_cull_face: %d", err);
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}


// Lua: gl.dummy_rotate(angle) -> matrix
static int gl_dummy_rotate(lua_State *L) {
    float angle = (float)luaL_checknumber(L, 1);

    // Model matrix: identity + Y rotation
    mat4 model;
    glm_mat4_identity(model);
    glm_rotate_y(model, angle, model);

    // View matrix: identity + translate (0, 0, -5)
    mat4 view;
    glm_mat4_identity(view);
    vec3 translate = {0.0f, 0.0f, -5.0f}; // Increased distance
    glm_translate(view, translate);

    // Projection matrix: perspective (45°, 800/600, 0.1, 100)
    mat4 proj;
    glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, 100.0f, proj);

    // Debug: Print projection matrix
    printf("Projection matrix:\n");
    for (int i = 0; i < 4; i++) {
        printf("[%f, %f, %f, %f]\n", proj[i][0], proj[i][1], proj[i][2], proj[i][3]);
    }

    // Compute MVP: proj * (view * model)
    mat4 temp, mvp;
    glm_mat4_mul(view, model, temp);
    glm_mat4_mul(proj, temp, mvp);

    // Debug: Print MVP matrix
    printf("MVP matrix:\n");
    for (int i = 0; i < 4; i++) {
        printf("[%f, %f, %f, %f]\n", mvp[i][0], mvp[i][1], mvp[i][2], mvp[i][3]);
    }

    // Push MVP matrix as Lua table
    lua_newtable(L);
    for (int i = 0; i < 16; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushnumber(L, mvp[i / 4][i % 4]);
        lua_settable(L, -3);
    }

    return 1;
}


static int gl_draw_cube(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    GLuint vao = (GLuint)luaL_checkinteger(L, 2);
    GLint mvp_loc = (GLint)luaL_checkinteger(L, 3);
    luaL_checktype(L, 4, LUA_TTABLE);

    float mvp[16];
    for (int i = 0; i < 16; i++) {
        lua_geti(L, 4, i + 1);
        mvp[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    printf("gl_draw_cube: MVP matrix:\n");
    for (int i = 0; i < 4; i++) {
        printf("[%f, %f, %f, %f]\n", mvp[i * 4 + 0], mvp[i * 4 + 1], mvp[i * 4 + 2], mvp[i * 4 + 3]);
    }

    GLint current_program, current_vao;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
    printf("gl_draw_cube: Before - Current program: %d, Current VAO: %d\n", current_program, current_vao);

    glUseProgram(program);
    glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, mvp);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
    printf("gl_draw_cube: After - Current program: %d, Current VAO: %d (Expected VAO: %d)\n", current_program, current_vao, vao);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "OpenGL error in gl_draw_cube: %d", err);
        return 2;
    }

    lua_pushboolean(L, 1);
    return 1;
}

// Lua: gl.draw_cube_vertex(vao) -> bool, err_msg
static int gl_draw_cube_vertex(lua_State *L) {
    GLuint vao = (GLuint)luaL_checkinteger(L, 1);

    // Debug: Check current VAO before binding
    GLint current_vao;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
    printf("gl_draw_cube_vertex: Before - Current VAO: %d (Expected: %d)\n", current_vao, vao);

    // Bind VAO and draw
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Debug: Check VAO after drawing
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
    printf("gl_draw_cube_vertex: After - Current VAO: %d\n", current_vao);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "OpenGL error in gl_draw_cube_vertex: %d", err);
        return 2;
    }

    lua_pushboolean(L, 1);
    return 1;
}

static int gl_draw_program_mat4(lua_State *L) {
    GLuint program = (GLuint)luaL_checkinteger(L, 1);
    GLint mvp_loc = (GLint)luaL_checkinteger(L, 2);
    luaL_checktype(L, 3, LUA_TTABLE);

    // Convert Lua MVP matrix to float array
    float mvp[16];
    for (int i = 0; i < 16; i++) {
        lua_geti(L, 3, i + 1);
        mvp[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    // Debug: Print MVP matrix
    printf("gl_draw_program_mat4: MVP matrix:\n");
    for (int i = 0; i < 4; i++) {
        printf("[%f, %f, %f, %f]\n", mvp[i * 4 + 0], mvp[i * 4 + 1], mvp[i * 4 + 2], mvp[i * 4 + 3]);
    }

    // Debug: Check current program
    GLint current_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
    printf("gl_draw_program_mat4: Before - Current program: %d (Expected: %d)\n", current_program, program);

    // Set shader program and uniform
    glUseProgram(program);
    glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, mvp);

    // Debug: Check program after setting
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
    printf("gl_draw_program_mat4: After - Current program: %d\n", current_program);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "OpenGL error in gl_draw_program_mat4: %d", err);
        return 2;
    }

    lua_pushboolean(L, 1);
    return 1;
}

// Lua: gl.dummy_uniform_mvp(mvp_loc, count, transpose, mvp) -> bool, err_msg
static int gl_dummy_uniform_mvp(lua_State *L) {
    GLint mvp_loc = (GLint)luaL_checkinteger(L, 1);
    GLsizei count = (GLsizei)luaL_checkinteger(L, 2);
    GLboolean transpose = (GLboolean)lua_toboolean(L, 3);
    luaL_checktype(L, 4, LUA_TTABLE);

    // Convert Lua MVP matrix to float array
    float mvp[16];
    for (int i = 0; i < 16; i++) {
        lua_geti(L, 4, i + 1);
        mvp[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    // Debug: Print MVP matrix
    printf("gl_dummy_uniform_mvp: MVP matrix:\n");
    for (int i = 0; i < 4; i++) {
        printf("[%f, %f, %f, %f]\n", mvp[i * 4 + 0], mvp[i * 4 + 1], mvp[i * 4 + 2], mvp[i * 4 + 3]);
    }

    // Set uniform matrix
    glUniformMatrix4fv(mvp_loc, count, transpose, mvp);
    // glUniformMatrix4fv(mvp_loc, count, GL_FALSE, mvp);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "OpenGL error in gl_dummy_uniform_mvp: %d", err);
        return 2;
    }

    lua_pushboolean(L, 1);
    return 1;
}


// Register GL constants
static void push_gl_constants(lua_State *L) {
    lua_pushinteger(L, GL_VERSION);
    lua_setfield(L, -2, "VERSION");
    lua_pushinteger(L, GL_VERTEX_SHADER);
    lua_setfield(L, -2, "VERTEX_SHADER");
    lua_pushinteger(L, GL_FRAGMENT_SHADER);
    lua_setfield(L, -2, "FRAGMENT_SHADER");
    lua_pushinteger(L, GL_ARRAY_BUFFER);
    lua_setfield(L, -2, "ARRAY_BUFFER");
    lua_pushinteger(L, GL_ELEMENT_ARRAY_BUFFER);
    lua_setfield(L, -2, "ELEMENT_ARRAY_BUFFER");
    lua_pushinteger(L, GL_STATIC_DRAW);
    lua_setfield(L, -2, "STATIC_DRAW");
    lua_pushinteger(L, GL_FLOAT);
    lua_setfield(L, -2, "FLOAT");
    lua_pushinteger(L, GL_FALSE);
    lua_setfield(L, -2, "FALSE");
    lua_pushinteger(L, GL_TRUE);
    lua_setfield(L, -2, "TRUE");
    lua_pushinteger(L, GL_DEPTH_TEST);
    lua_setfield(L, -2, "DEPTH_TEST");
    lua_pushinteger(L, GL_COLOR_BUFFER_BIT);
    lua_setfield(L, -2, "COLOR_BUFFER_BIT");
    lua_pushinteger(L, GL_DEPTH_BUFFER_BIT);
    lua_setfield(L, -2, "DEPTH_BUFFER_BIT");
    lua_pushinteger(L, GL_TRIANGLES);
    lua_setfield(L, -2, "TRIANGLES");
    lua_pushinteger(L, GL_UNSIGNED_INT);
    lua_setfield(L, -2, "UNSIGNED_INT");
    lua_pushinteger(L, GL_FRONT_AND_BACK);
    lua_setfield(L, -2, "FRONT_AND_BACK");
    lua_pushinteger(L, GL_LINE);
    lua_setfield(L, -2, "LINE");
    lua_pushinteger(L, GL_LESS);
    lua_setfield(L, -2, "LESS");

    lua_pushinteger(L, GL_CULL_FACE);
    lua_setfield(L, -2, "CULL_FACE");
    lua_pushinteger(L, GL_BACK);
    lua_setfield(L, -2, "BACK");
    lua_pushinteger(L, GL_FRONT);
    lua_setfield(L, -2, "FRONT");
    
}

static const struct luaL_Reg gl_lib[] = {
    {"init", gl_init},
    {"quit", gl_quit},
    {"draw_cube", gl_draw_cube},
    {"draw_cube_vertex", gl_draw_cube_vertex}, 
    {"draw_program_mat4", gl_draw_program_mat4},
    {"dummy_uniform_mvp", gl_dummy_uniform_mvp},
    {"cull_face", gl_cull_face}, 
    {"get_integer", gl_get_integer},
    {"get_error", gl_get_error},
    {"get_string", gl_get_string},
    {"viewport", gl_viewport},
    {"create_shader", gl_create_shader},
    {"shader_source", gl_shader_source},
    {"compile_shader", gl_compile_shader},
    {"create_program", gl_create_program},
    {"attach_shader", gl_attach_shader},
    {"link_program", gl_link_program},
    {"delete_shader", gl_delete_shader},
    {"gen_vertex_arrays", gl_gen_vertex_arrays},
    {"gen_buffers", gl_gen_buffers},
    {"bind_vertex_array", gl_bind_vertex_array},
    {"bind_buffer", gl_bind_buffer},
    {"buffer_data", gl_buffer_data},
    {"buffer_data_uint", gl_buffer_data_uint},
    {"vertex_attrib_pointer", gl_vertex_attrib_pointer},
    {"enable_vertex_attrib_array", gl_enable_vertex_attrib_array},
    {"enable", gl_enable},
    {"get_uniform_location", gl_get_uniform_location},
    {"use_program", gl_use_program},
    {"uniform_matrix4fv", gl_uniform_matrix4fv},
    {"clear_color", gl_clear_color},
    {"clear", gl_clear},
    {"draw_elements", gl_draw_elements},
    {"delete_vertex_arrays", gl_delete_vertex_arrays},
    {"delete_buffers", gl_delete_buffers},
    {"delete_program", gl_delete_program},
    {"polygon_mode", gl_polygon_mode},
    {"depth_func", gl_depth_func},
    {"dummy_rotate", gl_dummy_rotate},
    


    {NULL, NULL}
};

int luaopen_module_gl(lua_State *L) {
    luaL_newlib(L, gl_lib);
    push_gl_constants(L);
    return 1;
}