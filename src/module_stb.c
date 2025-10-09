// module_stb.c
#include "module_stb.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <lauxlib.h>

// Lua: stb.load_image(file_path) -> data, width, height, channels, err_msg
static int stb_load_image(lua_State *L) {
    const char *file_path = luaL_checkstring(L, 1);
    
    int width, height, channels;
    unsigned char *data = stbi_load(file_path, &width, &height, &channels, 0);
    
    if (!data) {
        lua_pushnil(L);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        lua_pushstring(L, stbi_failure_reason());
        return 5;
    }
    
    // Push results to Lua
    lua_pushlightuserdata(L, data);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    lua_pushinteger(L, channels);
    return 4;
}

// Lua: stb.free_image(data)
static int stb_free_image(lua_State *L) {
    void *data = lua_touserdata(L, 1);
    if (data) {
        stbi_image_free(data);
    }
    return 0;
}

static const struct luaL_Reg stb_lib[] = {
    {"load_image", stb_load_image},
    {"free_image", stb_free_image},
    {NULL, NULL}
};

int luaopen_module_stb(lua_State *L) {
    luaL_newlib(L, stb_lib);
    
    // Expose constants (stb_image doesn't define many, but we can add common ones)
    lua_pushinteger(L, 1); lua_setfield(L, -2, "GREY");        // 1 channel
    lua_pushinteger(L, 2); lua_setfield(L, -2, "GREY_ALPHA");  // 2 channels
    lua_pushinteger(L, 3); lua_setfield(L, -2, "RGB");         // 3 channels
    lua_pushinteger(L, 4); lua_setfield(L, -2, "RGBA");        // 4 channels
    
    return 1;
}