// module_cglm.c
#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <cglm/cglm.h>

// Userdata type names
#define VEC3_TYPE "cglm.vec3"
#define VEC4_TYPE "cglm.vec4"
#define MAT4_TYPE "cglm.mat4"

// Helper to check userdata types
static vec3* check_vec3(lua_State *L, int idx) {
    void *ud = luaL_checkudata(L, idx, VEC3_TYPE);
    luaL_argcheck(L, ud != NULL, idx, "vec3 expected");
    return (vec3*)ud;
}

static vec4* check_vec4(lua_State *L, int idx) {
    void *ud = luaL_checkudata(L, idx, VEC4_TYPE);
    luaL_argcheck(L, ud != NULL, idx, "vec4 expected");
    return (vec4*)ud;
}

static mat4* check_mat4(lua_State *L, int idx) {
    void *ud = luaL_checkudata(L, idx, MAT4_TYPE);
    luaL_argcheck(L, ud != NULL, idx, "mat4 expected");
    return (mat4*)ud;
}

// Helper to push userdata types
static vec3* push_vec3(lua_State *L, float x, float y, float z) {
    vec3 *v = (vec3*)lua_newuserdata(L, sizeof(vec3));
    (*v)[0] = x; (*v)[1] = y; (*v)[2] = z;
    luaL_getmetatable(L, VEC3_TYPE);
    lua_setmetatable(L, -2);
    return v;
}

static vec4* push_vec4(lua_State *L, float x, float y, float z, float w) {
    vec4 *v = (vec4*)lua_newuserdata(L, sizeof(vec4));
    (*v)[0] = x; (*v)[1] = y; (*v)[2] = z; (*v)[3] = w;
    luaL_getmetatable(L, VEC4_TYPE);
    lua_setmetatable(L, -2);
    return v;
}

static mat4* push_mat4(lua_State *L, mat4 m) { // Removed const
    mat4 *m_out = (mat4*)lua_newuserdata(L, sizeof(mat4));
    glm_mat4_copy(m, *m_out);
    luaL_getmetatable(L, MAT4_TYPE);
    lua_setmetatable(L, -2);
    return m_out;
}

// --- vec3 functions ---
static int vec3_new(lua_State *L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_checknumber(L, 3);
    push_vec3(L, x, y, z);
    return 1;
}

static int vec3_add(lua_State *L) {
    vec3 *v1 = check_vec3(L, 1);
    vec3 *v2 = check_vec3(L, 2);
    vec3 result;
    glm_vec3_add(*v1, *v2, result);
    push_vec3(L, result[0], result[1], result[2]);
    return 1;
}

static int vec3_sub(lua_State *L) {
    vec3 *v1 = check_vec3(L, 1);
    vec3 *v2 = check_vec3(L, 2);
    vec3 result;
    glm_vec3_sub(*v1, *v2, result);
    push_vec3(L, result[0], result[1], result[2]);
    return 1;
}

static int vec3_mul(lua_State *L) {
    vec3 *v;
    float scalar;
    if (lua_isnumber(L, 1)) {
        scalar = (float)luaL_checknumber(L, 1);
        v = check_vec3(L, 2);
    } else {
        v = check_vec3(L, 1);
        scalar = (float)luaL_checknumber(L, 2);
    }
    vec3 result;
    glm_vec3_scale(*v, scalar, result);
    push_vec3(L, result[0], result[1], result[2]);
    return 1;
}

static int vec3_dot(lua_State *L) {
    vec3 *v1 = check_vec3(L, 1);
    vec3 *v2 = check_vec3(L, 2);
    lua_pushnumber(L, glm_vec3_dot(*v1, *v2));
    return 1;
}

static int vec3_cross(lua_State *L) {
    vec3 *v1 = check_vec3(L, 1);
    vec3 *v2 = check_vec3(L, 2);
    vec3 result;
    glm_vec3_cross(*v1, *v2, result);
    push_vec3(L, result[0], result[1], result[2]);
    return 1;
}

static int vec3_normalize(lua_State *L) {
    vec3 *v = check_vec3(L, 1);
    vec3 result;
    glm_vec3_copy(*v, result);
    glm_vec3_normalize(result);
    push_vec3(L, result[0], result[1], result[2]);
    return 1;
}

static int vec3_length(lua_State *L) {
    vec3 *v = check_vec3(L, 1);
    lua_pushnumber(L, glm_vec3_norm(*v));
    return 1;
}

static int vec3_get(lua_State *L) {
    vec3 *v = check_vec3(L, 1);
    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index > 2) {
        luaL_error(L, "Index out of bounds: %d (must be 0, 1, or 2)", index);
    }
    lua_pushnumber(L, (*v)[index]);
    return 1;
}

static int vec3_set(lua_State *L) {
    vec3 *v = check_vec3(L, 1);
    int index = luaL_checkinteger(L, 2);
    float value = (float)luaL_checknumber(L, 3);
    if (index < 0 || index > 2) {
        luaL_error(L, "Index out of bounds: %d (must be 0, 1, or 2)", index);
    }
    (*v)[index] = value;
    return 0;
}

static int vec3_tostring(lua_State *L) {
    vec3 *v = check_vec3(L, 1);
    char buf[128];
    snprintf(buf, sizeof(buf), "vec3(%f, %f, %f)", (*v)[0], (*v)[1], (*v)[2]);
    lua_pushstring(L, buf);
    return 1;
}

static int vec3_index(lua_State *L) {
    vec3 *v = check_vec3(L, 1);
    const char *key = luaL_checkstring(L, 2);
    luaL_getmetatable(L, VEC3_TYPE);
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushnil(L);
    }
    return 1;
}

// --- vec4 functions ---
static int vec4_new(lua_State *L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_checknumber(L, 3);
    float w = (float)luaL_checknumber(L, 4);
    push_vec4(L, x, y, z, w);
    return 1;
}

static int vec4_add(lua_State *L) {
    vec4 *v1 = check_vec4(L, 1);
    vec4 *v2 = check_vec4(L, 2);
    vec4 result;
    glm_vec4_add(*v1, *v2, result);
    push_vec4(L, result[0], result[1], result[2], result[3]);
    return 1;
}

static int vec4_sub(lua_State *L) {
    vec4 *v1 = check_vec4(L, 1);
    vec4 *v2 = check_vec4(L, 2);
    vec4 result;
    glm_vec4_sub(*v1, *v2, result);
    push_vec4(L, result[0], result[1], result[2], result[3]);
    return 1;
}

static int vec4_mul(lua_State *L) {
    vec4 *v;
    float scalar;
    if (lua_isnumber(L, 1)) {
        scalar = (float)luaL_checknumber(L, 1);
        v = check_vec4(L, 2);
    } else {
        v = check_vec4(L, 1);
        scalar = (float)luaL_checknumber(L, 2);
    }
    vec4 result;
    glm_vec4_scale(*v, scalar, result);
    push_vec4(L, result[0], result[1], result[2], result[3]);
    return 1;
}

static int vec4_dot(lua_State *L) {
    vec4 *v1 = check_vec4(L, 1);
    vec4 *v2 = check_vec4(L, 2);
    lua_pushnumber(L, glm_vec4_dot(*v1, *v2));
    return 1;
}

static int vec4_normalize(lua_State *L) {
    vec4 *v = check_vec4(L, 1);
    vec4 result;
    glm_vec4_copy(*v, result);
    glm_vec4_normalize(result);
    push_vec4(L, result[0], result[1], result[2], result[3]);
    return 1;
}

static int vec4_length(lua_State *L) {
    vec4 *v = check_vec4(L, 1);
    lua_pushnumber(L, glm_vec4_norm(*v));
    return 1;
}

static int vec4_get(lua_State *L) {
    vec4 *v = check_vec4(L, 1);
    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index > 3) {
        luaL_error(L, "Index out of bounds: %d (must be 0, 1, 2, or 3)", index);
    }
    lua_pushnumber(L, (*v)[index]);
    return 1;
}

static int vec4_set(lua_State *L) {
    vec4 *v = check_vec4(L, 1);
    int index = luaL_checkinteger(L, 2);
    float value = (float)luaL_checknumber(L, 3);
    if (index < 0 || index > 3) {
        luaL_error(L, "Index out of bounds: %d (must be 0, 1, 2, or 3)", index);
    }
    (*v)[index] = value;
    return 0;
}

static int vec4_tostring(lua_State *L) {
    vec4 *v = check_vec4(L, 1);
    char buf[128];
    snprintf(buf, sizeof(buf), "vec4(%f, %f, %f, %f)", (*v)[0], (*v)[1], (*v)[2], (*v)[3]);
    lua_pushstring(L, buf);
    return 1;
}

static int vec4_index(lua_State *L) {
    vec4 *v = check_vec4(L, 1);
    const char *key = luaL_checkstring(L, 2);
    luaL_getmetatable(L, VEC4_TYPE);
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushnil(L);
    }
    return 1;
}

// --- mat4 functions ---
static int mat4_new(lua_State *L) {
    mat4 m;
    glm_mat4_identity(m); // Default to identity matrix
    if (lua_istable(L, 1)) {
        // Expect a table of 16 numbers in row-major order
        for (int i = 1; i <= 16; i++) {
            lua_rawgeti(L, 1, i);
            if (!lua_isnumber(L, -1)) {
                luaL_error(L, "Expected 16 numbers in table for mat4");
            }
            // Map table index to matrix (row-major input to column-major storage)
            int row = (i - 1) / 4;
            int col = (i - 1) % 4;
            m[row][col] = (float)lua_tonumber(L, -1);
            lua_pop(L, 1);
        }
    }
    push_mat4(L, m);
    return 1;
}

static int mat4_identity(lua_State *L) {
    mat4 m;
    glm_mat4_identity(m);
    push_mat4(L, m);
    return 1;
}

static int mat4_mul(lua_State *L) {
    mat4 *m1 = check_mat4(L, 1);
    mat4 *m2 = check_mat4(L, 2);
    mat4 result;
    glm_mat4_mul(*m1, *m2, result);
    push_mat4(L, result);
    return 1;
}

static int mat4_get(lua_State *L) {
    mat4 *m = check_mat4(L, 1);
    int row = luaL_checkinteger(L, 2);
    int col = luaL_checkinteger(L, 3);
    if (row < 0 || row > 3 || col < 0 || col > 3) {
        luaL_error(L, "Index out of bounds: (%d, %d) (must be 0-3)", row, col);
    }
    lua_pushnumber(L, (*m)[row][col]);
    return 1;
}

static int mat4_set(lua_State *L) {
    mat4 *m = check_mat4(L, 1);
    int row = luaL_checkinteger(L, 2);
    int col = luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);
    if (row < 0 || row > 3 || col < 0 || col > 3) {
        luaL_error(L, "Index out of bounds: (%d, %d) (must be 0-3)", row, col);
    }
    (*m)[row][col] = value;
    return 0;
}

static int mat4_tostring(lua_State *L) {
    mat4 *m = check_mat4(L, 1);
    char buf[256];
    snprintf(buf, sizeof(buf),
             "mat4(\n  %f, %f, %f, %f,\n  %f, %f, %f, %f,\n  %f, %f, %f, %f,\n  %f, %f, %f, %f)",
             (*m)[0][0], (*m)[0][1], (*m)[0][2], (*m)[0][3],
             (*m)[1][0], (*m)[1][1], (*m)[1][2], (*m)[1][3],
             (*m)[2][0], (*m)[2][1], (*m)[2][2], (*m)[2][3],
             (*m)[3][0], (*m)[3][1], (*m)[3][2], (*m)[3][3]);
    lua_pushstring(L, buf);
    return 1;
}

static int mat4_index(lua_State *L) {
    mat4 *m = check_mat4(L, 1);
    const char *key = luaL_checkstring(L, 2);
    luaL_getmetatable(L, MAT4_TYPE);
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushnil(L);
    }
    return 1;
}

// Garbage collection
static int vec3_gc(lua_State *L) { return 0; }
static int vec4_gc(lua_State *L) { return 0; }
static int mat4_gc(lua_State *L) { return 0; }

// Metatable methods
static const luaL_Reg vec3_methods[] = {
    {"dot", vec3_dot},
    {"cross", vec3_cross},
    {"normalize", vec3_normalize},
    {"length", vec3_length},
    {"get", vec3_get},
    {"set", vec3_set},
    {NULL, NULL}
};

static const luaL_Reg vec4_methods[] = {
    {"dot", vec4_dot},
    {"normalize", vec4_normalize},
    {"length", vec4_length},
    {"get", vec4_get},
    {"set", vec4_set},
    {NULL, NULL}
};

static const luaL_Reg mat4_methods[] = {
    {"get", mat4_get},
    {"set", mat4_set},
    {NULL, NULL}
};

// Lua: cglm.ortho(left, right, bottom, top, near, far) -> mat4
static int mat4_ortho(lua_State *L) {
    float left = (float)luaL_checknumber(L, 1);
    float right = (float)luaL_checknumber(L, 2);
    float bottom = (float)luaL_checknumber(L, 3);
    float top = (float)luaL_checknumber(L, 4);
    float near = (float)luaL_checknumber(L, 5);
    float far = (float)luaL_checknumber(L, 6);
    mat4 m;
    glm_ortho(left, right, bottom, top, near, far, m);
    push_mat4(L, m);
    return 1;
}

// Module functions
static const luaL_Reg module_glm_funcs[] = {
    {"vec3", vec3_new},
    {"vec4", vec4_new},
    {"mat4", mat4_new},
    {"mat4_identity", mat4_identity},
    {"ortho", mat4_ortho},
    {NULL, NULL}
};

// Module entry point
int luaopen_module_cglm(lua_State *L) {
    // vec3 metatable
    luaL_newmetatable(L, VEC3_TYPE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, vec3_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, vec3_add);
    lua_setfield(L, -2, "__add");
    lua_pushcfunction(L, vec3_sub);
    lua_setfield(L, -2, "__sub");
    lua_pushcfunction(L, vec3_mul);
    lua_setfield(L, -2, "__mul");
    lua_pushcfunction(L, vec3_gc);
    lua_setfield(L, -2, "__gc");
    luaL_setfuncs(L, vec3_methods, 0);
    lua_pop(L, 1);

    // vec4 metatable
    luaL_newmetatable(L, VEC4_TYPE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, vec4_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, vec4_add);
    lua_setfield(L, -2, "__add");
    lua_pushcfunction(L, vec4_sub);
    lua_setfield(L, -2, "__sub");
    lua_pushcfunction(L, vec4_mul);
    lua_setfield(L, -2, "__mul");
    lua_pushcfunction(L, vec4_gc);
    lua_setfield(L, -2, "__gc");
    luaL_setfuncs(L, vec4_methods, 0);
    lua_pop(L, 1);

    // mat4 metatable
    luaL_newmetatable(L, MAT4_TYPE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, mat4_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, mat4_mul);
    lua_setfield(L, -2, "__mul");
    lua_pushcfunction(L, mat4_gc);
    lua_setfield(L, -2, "__gc");
    luaL_setfuncs(L, mat4_methods, 0);
    lua_pop(L, 1);

    // Create module table
    luaL_newlib(L, module_glm_funcs);
    return 1;
}