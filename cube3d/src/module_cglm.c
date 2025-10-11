// module_cglm.c
#include <lauxlib.h>
#include <cglm/cglm.h>

// Lua: cglm.mat4_identity() -> matrix
static int cglm_mat4_identity(lua_State *L) {
    mat4 matrix;
    glm_mat4_identity(matrix);
    lua_newtable(L);
    for (int i = 0; i < 16; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushnumber(L, matrix[i / 4][i % 4]);
        lua_settable(L, -3);
    }
    return 1;
}

// Lua: cglm.rotate_y(matrix, angle) -> matrix
static int cglm_rotate_y(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    float angle = (float)luaL_checknumber(L, 2);
    mat4 matrix;
    for (int i = 0; i < 16; i++) {
        lua_geti(L, 1, i + 1);
        matrix[i / 4][i % 4] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    mat4 result;
    glm_rotate_y(matrix, angle, result);
    lua_newtable(L);
    for (int i = 0; i < 16; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushnumber(L, result[i / 4][i % 4]);
        lua_settable(L, -3);
    }
    return 1;
}

// Lua: cglm.rotate_z(matrix, angle) -> matrix
static int cglm_rotate_z(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    float angle = (float)luaL_checknumber(L, 2);
    mat4 matrix;
    for (int i = 0; i < 16; i++) {
        lua_geti(L, 1, i + 1);
        matrix[i / 4][i % 4] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    mat4 result;
    glm_rotate_z(matrix, angle, result);
    lua_newtable(L);
    for (int i = 0; i < 16; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushnumber(L, result[i / 4][i % 4]);
        lua_settable(L, -3);
    }
    return 1;
}

// Lua: cglm.translate(matrix, vec) -> matrix
static int cglm_translate(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TTABLE);
    mat4 matrix;
    vec3 vector;
    for (int i = 0; i < 16; i++) {
        lua_geti(L, 1, i + 1);
        matrix[i / 4][i % 4] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    for (int i = 0; i < 3; i++) {
        lua_geti(L, 2, i + 1);
        vector[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    // glm_translate modifies the input matrix in-place
    glm_translate(matrix, vector);
    lua_newtable(L);
    for (int i = 0; i < 16; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushnumber(L, matrix[i / 4][i % 4]);
        lua_settable(L, -3);
    }
    return 1;
}

// Lua: cglm.perspective(fovy, aspect, near, far) -> matrix
static int cglm_perspective(lua_State *L) {
    float fovy = (float)luaL_checknumber(L, 1);
    float aspect = (float)luaL_checknumber(L, 2);
    float near = (float)luaL_checknumber(L, 3);
    float far = (float)luaL_checknumber(L, 4);
    mat4 matrix;
    glm_perspective(fovy, aspect, near, far, matrix);
    lua_newtable(L);
    for (int i = 0; i < 16; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushnumber(L, matrix[i / 4][i % 4]);
        lua_settable(L, -3);
    }
    return 1;
}

// Lua: cglm.mat4_mul(matrix1, matrix2) -> matrix
static int cglm_mat4_mul(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TTABLE);
    mat4 m1, m2, result;
    for (int i = 0; i < 16; i++) {
        lua_geti(L, 1, i + 1);
        m1[i / 4][i % 4] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_geti(L, 2, i + 1);
        m2[i / 4][i % 4] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }
    glm_mat4_mul(m1, m2, result);
    lua_newtable(L);
    for (int i = 0; i < 16; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushnumber(L, result[i / 4][i % 4]);
        lua_settable(L, -3);
    }
    return 1;
}

// Lua: cglm.to_radians(degrees) -> radians
static int cglm_to_radians(lua_State *L) {
    float degrees = (float)luaL_checknumber(L, 1);
    lua_pushnumber(L, glm_rad(degrees));
    return 1;
}

static const struct luaL_Reg cglm_lib[] = {
    {"mat4_identity", cglm_mat4_identity},
    {"rotate_y", cglm_rotate_y},
    {"rotate_z", cglm_rotate_z},
    {"translate", cglm_translate},
    {"perspective", cglm_perspective},
    {"mat4_mul", cglm_mat4_mul},
    {"to_radians", cglm_to_radians},
    {NULL, NULL}
};

int luaopen_module_cglm(lua_State *L) {
    luaL_newlib(L, cglm_lib);
    return 1;
}