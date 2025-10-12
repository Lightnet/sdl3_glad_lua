// module_stb.c

#include "module_stb.h"
#include <lauxlib.h>
#include <stdlib.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static int stb_bake_font_bitmap(lua_State *L) {
    size_t font_len;
    const unsigned char *font_buffer = (const unsigned char *)luaL_checklstring(L, 1, &font_len);
    float pixel_height = (float)luaL_checknumber(L, 2);
    int bitmap_w = (int)luaL_checkinteger(L, 3);
    int bitmap_h = (int)luaL_checkinteger(L, 4);
    int first_char = (int)luaL_checkinteger(L, 5);
    int num_chars = (int)luaL_checkinteger(L, 6);
    unsigned char *bitmap = (unsigned char *)malloc(bitmap_w * bitmap_h);
    if (!bitmap) {
        return luaL_error(L, "malloc failed for bitmap");
    }
    stbtt_bakedchar *cdata = (stbtt_bakedchar *)malloc(num_chars * sizeof(stbtt_bakedchar));
    if (!cdata) {
        free(bitmap);
        return luaL_error(L, "malloc failed for cdata");
    }
    int result = stbtt_BakeFontBitmap(font_buffer, 0, pixel_height, bitmap, bitmap_w, bitmap_h, first_char, num_chars, cdata);
    if (result <= 0) {
        free(bitmap);
        free(cdata);
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushinteger(L, result);
        return 3;
    }
    lua_pushlstring(L, (const char *)bitmap, bitmap_w * bitmap_h);
    lua_newtable(L);
    for (int i = 0; i < num_chars; i++) {
        lua_pushinteger(L, i + 1);
        lua_newtable(L);
        lua_pushnumber(L, cdata[i].x0); lua_setfield(L, -2, "x0");
        lua_pushnumber(L, cdata[i].y0); lua_setfield(L, -2, "y0");
        lua_pushnumber(L, cdata[i].x1); lua_setfield(L, -2, "x1");
        lua_pushnumber(L, cdata[i].y1); lua_setfield(L, -2, "y1");
        lua_pushnumber(L, cdata[i].xoff); lua_setfield(L, -2, "xoff");
        lua_pushnumber(L, cdata[i].yoff); lua_setfield(L, -2, "yoff");
        lua_pushnumber(L, cdata[i].xadvance); lua_setfield(L, -2, "xadvance");
        lua_settable(L, -3);
    }
    lua_pushinteger(L, result);
    free(bitmap);
    free(cdata);
    return 3;
}

static int stb_get_baked_quad(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    int bitmap_w = (int)luaL_checkinteger(L, 2);
    int bitmap_h = (int)luaL_checkinteger(L, 3);
    int char_code = (int)luaL_checkinteger(L, 4);
    float x = (float)luaL_checknumber(L, 5);
    float y = (float)luaL_checknumber(L, 6);
    int char_index = char_code - 32; // Assuming first_char=32, ASCII 32-127
    if (char_index < 0 || char_index >= 96) {
        return luaL_error(L, "Character out of baked range");
    }
    lua_geti(L, 1, char_index + 1);
    if (!lua_istable(L, -1)) {
        return luaL_error(L, "Invalid cdata table entry");
    }
    stbtt_bakedchar cd;
    lua_getfield(L, -1, "x0"); cd.x0 = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, -1, "y0"); cd.y0 = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, -1, "x1"); cd.x1 = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, -1, "y1"); cd.y1 = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, -1, "xoff"); cd.xoff = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, -1, "yoff"); cd.yoff = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, -1, "xadvance"); cd.xadvance = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_pop(L, 1);
    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(&cd, bitmap_w, bitmap_h, char_index, &x, &y, &q, 1);
    lua_newtable(L);
    lua_pushnumber(L, q.x0); lua_setfield(L, -2, "x0");
    lua_pushnumber(L, q.y0); lua_setfield(L, -2, "y0");
    lua_pushnumber(L, q.x1); lua_setfield(L, -2, "x1");
    lua_pushnumber(L, q.y1); lua_setfield(L, -2, "y1");
    lua_pushnumber(L, q.s0); lua_setfield(L, -2, "s0");
    lua_pushnumber(L, q.t0); lua_setfield(L, -2, "t0");
    lua_pushnumber(L, q.s1); lua_setfield(L, -2, "s1");
    lua_pushnumber(L, q.t1); lua_setfield(L, -2, "t1");
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 3;
}

static const struct luaL_Reg stb_lib[] = {
    {"bake_font_bitmap", stb_bake_font_bitmap},
    {"get_baked_quad", stb_get_baked_quad},
    {NULL, NULL}
};

int luaopen_module_stb(lua_State *L) {
    luaL_newlib(L, stb_lib);
    return 1;
}