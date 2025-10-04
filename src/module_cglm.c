// module_glm.c
#include "module_cglm.h"
#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <cglm/cglm.h>
// #include <string.h>

// vector 3 use array [0,0,0] not x,y,z

// Userdata type name for vec3
#define VEC3_TYPE "cglm.vec3"

// Helper to check if a userdata is a vec3
static vec3* check_vec3(lua_State *L, int idx) {
    void *ud = luaL_checkudata(L, idx, VEC3_TYPE);
    luaL_argcheck(L, ud != NULL, idx, "vec3 expected");
    return (vec3*)ud;
}

// Helper to push a vec3 to the Lua stack
static vec3* push_vec3(lua_State *L, float x, float y, float z) {
    vec3 *v = (vec3*)lua_newuserdata(L, sizeof(vec3));
    (*v)[0] = x;
    (*v)[1] = y;
    (*v)[2] = z;
    luaL_getmetatable(L, VEC3_TYPE);
    lua_setmetatable(L, -2);
    return v;
}

// Constructor: cglm.vec3(x, y, z)
static int vec3_new(lua_State *L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_checknumber(L, 3);
    push_vec3(L, x, y, z);
    return 1;
}

// Addition: vec3 + vec3
static int vec3_add(lua_State *L) {
    vec3 *v1 = check_vec3(L, 1);
    vec3 *v2 = check_vec3(L, 2);
    vec3 result;
    glm_vec3_add(*v1, *v2, result);
    push_vec3(L, result[0], result[1], result[2]);
    return 1;
}

// Subtraction: vec3 - vec3
static int vec3_sub(lua_State *L) {
    vec3 *v1 = check_vec3(L, 1);
    vec3 *v2 = check_vec3(L, 2);
    vec3 result;
    glm_vec3_sub(*v1, *v2, result);
    push_vec3(L, result[0], result[1], result[2]);
    return 1;
}

// Multiplication: vec3 * number or number * vec3
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

// Dot product: vec3:dot(other)
static int vec3_dot(lua_State *L) {
    vec3 *v1 = check_vec3(L, 1);
    vec3 *v2 = check_vec3(L, 2);
    lua_pushnumber(L, glm_vec3_dot(*v1, *v2));
    return 1;
}

// Cross product: vec3:cross(other)
static int vec3_cross(lua_State *L) {
    vec3 *v1 = check_vec3(L, 1);
    vec3 *v2 = check_vec3(L, 2);
    vec3 result;
    glm_vec3_cross(*v1, *v2, result);
    push_vec3(L, result[0], result[1], result[2]);
    return 1;
}

// Normalize: vec3:normalize()
static int vec3_normalize(lua_State *L) {
    vec3 *v = check_vec3(L, 1);
    vec3 result;
    glm_vec3_copy(*v, result);
    glm_vec3_normalize(result);
    push_vec3(L, result[0], result[1], result[2]);
    return 1;
}

// Get length: vec3:length()
static int vec3_length(lua_State *L) {
    vec3 *v = check_vec3(L, 1);
    lua_pushnumber(L, glm_vec3_norm(*v));
    return 1;
}

// Get component: vec3:get(index)
static int vec3_get(lua_State *L) {
    vec3 *v = check_vec3(L, 1);
    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index > 2) {
        luaL_error(L, "Index out of bounds: %d (must be 0, 1, or 2)", index);
    }
    lua_pushnumber(L, (*v)[index]);
    return 1;
}

// Set component: vec3:set(index, value)
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

// To string: vec3:__tostring
static int vec3_tostring(lua_State *L) {
    vec3 *v = check_vec3(L, 1);
    char buf[128];
    snprintf(buf, sizeof(buf), "vec3(%f, %f, %f)", (*v)[0], (*v)[1], (*v)[2]);
    lua_pushstring(L, buf);
    return 1;
}

// Index handler: methods only
static int vec3_index(lua_State *L) {
    vec3 *v = check_vec3(L, 1);
    const char *key = luaL_checkstring(L, 2);
    fprintf(stderr, "vec3_index: key='%s'\n", key); // Debug output
    luaL_getmetatable(L, VEC3_TYPE);
    lua_getfield(L, -1, key);
    lua_remove(L, -2); // Remove metatable
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushnil(L);
    }
    return 1;
}

// Garbage collection
static int vec3_gc(lua_State *L) {
    // No cleanup needed for vec3 (float[3])
    return 0;
}

// Metatable methods for vec3
static const luaL_Reg vec3_methods[] = {
    {"dot", vec3_dot},
    {"cross", vec3_cross},
    {"normalize", vec3_normalize},
    {"length", vec3_length},
    {"get", vec3_get},
    {"set", vec3_set},
    {NULL, NULL}
};

// Module functions
static const luaL_Reg module_glm_funcs[] = {
    {"vec3", vec3_new},
    {NULL, NULL}
};

// Module entry point
int luaopen_module_cglm(lua_State *L) {
    // Create vec3 metatable
    luaL_newmetatable(L, VEC3_TYPE);
    
    // Set metatable as its own __index to allow method lookup
    lua_pushvalue(L, -1); // Duplicate metatable
    lua_setfield(L, -2, "__index"); // Set metatable.__index = metatable
    
    // Set other metamethods
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
    
    // Register methods in metatable
    luaL_setfuncs(L, vec3_methods, 0);

    // Create module table
    luaL_newlib(L, module_glm_funcs);
    return 1;
}