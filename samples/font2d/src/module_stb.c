// module_stb.c
#include "module_stb.h"
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

typedef struct {
    unsigned char *ttf_buffer;
    unsigned char *bitmap;
    stbtt_bakedchar *cdata;
    int width;
    int height;
    int first_char;
    int num_chars;
} stb_font;

static const char *STB_FONT_MT = "stb_font";

static int stb_bake_font(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    float pixel_height = (float)luaL_checknumber(L, 2);
    int bitmap_width = (int)luaL_checkinteger(L, 3);
    int bitmap_height = (int)luaL_checkinteger(L, 4);
    int first_char = (int)luaL_optinteger(L, 5, 32);
    int num_chars = (int)luaL_optinteger(L, 6, 96);

    FILE *f = fopen(filename, "rb");
    if (!f) {
        return luaL_error(L, "Failed to open font file: %s", filename);
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *ttf_buffer = (unsigned char *)malloc(file_size);
    if (!ttf_buffer) {
        fclose(f);
        return luaL_error(L, "Memory allocation failed for TTF buffer");
    }

    if (fread(ttf_buffer, 1, file_size, f) != (size_t)file_size) {
        free(ttf_buffer);
        fclose(f);
        return luaL_error(L, "Failed to read font file");
    }
    fclose(f);

    unsigned char *bitmap = (unsigned char *)calloc(bitmap_width * bitmap_height, sizeof(unsigned char));
    if (!bitmap) {
        free(ttf_buffer);
        return luaL_error(L, "Memory allocation failed for bitmap");
    }

    stbtt_bakedchar *cdata = (stbtt_bakedchar *)malloc(num_chars * sizeof(stbtt_bakedchar));
    if (!cdata) {
        free(bitmap);
        free(ttf_buffer);
        return luaL_error(L, "Memory allocation failed for cdata");
    }

    int bake_result = stbtt_BakeFontBitmap(ttf_buffer, 0, pixel_height, bitmap, bitmap_width, bitmap_height, first_char, num_chars, cdata);
    if (bake_result <= 0) {
        free(cdata);
        free(bitmap);
        free(ttf_buffer);
        return luaL_error(L, "stbtt_BakeFontBitmap failed with code: %d", bake_result);
    }

    stb_font *font = (stb_font *)lua_newuserdata(L, sizeof(stb_font));
    font->ttf_buffer = ttf_buffer;
    font->bitmap = bitmap;
    font->cdata = cdata;
    font->width = bitmap_width;
    font->height = bitmap_height;
    font->first_char = first_char;
    font->num_chars = num_chars;

    luaL_setmetatable(L, STB_FONT_MT);

    return 1;
}

static int stb_font_gc(lua_State *L) {
    stb_font *font = (stb_font *)luaL_checkudata(L, 1, STB_FONT_MT);
    if (font->ttf_buffer) free(font->ttf_buffer);
    if (font->bitmap) free(font->bitmap);
    if (font->cdata) free(font->cdata);
    return 0;
}

static int stb_font_get_bitmap(lua_State *L) {
    stb_font *font = (stb_font *)luaL_checkudata(L, 1, STB_FONT_MT);
    lua_pushlstring(L, (const char *)font->bitmap, font->width * font->height);
    lua_pushinteger(L, font->width);
    lua_pushinteger(L, font->height);
    return 3;
}

static int stb_font_get_baked_quad(lua_State *L) {
    stb_font *font = (stb_font *)luaL_checkudata(L, 1, STB_FONT_MT);
    int char_code = (int)luaL_checkinteger(L, 2);
    float xpos = (float)luaL_checknumber(L, 3);
    float ypos = (float)luaL_checknumber(L, 4);

    int char_index = char_code - font->first_char;
    if (char_index < 0 || char_index >= font->num_chars) {
        return luaL_error(L, "Character code out of baked range");
    }

    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(&font->cdata[char_index], font->width, font->height, 0, &xpos, &ypos, &q, 1);

    lua_newtable(L);
    lua_pushstring(L, "x0"); lua_pushnumber(L, q.x0); lua_settable(L, -3);
    lua_pushstring(L, "y0"); lua_pushnumber(L, q.y0); lua_settable(L, -3);
    lua_pushstring(L, "x1"); lua_pushnumber(L, q.x1); lua_settable(L, -3);
    lua_pushstring(L, "y1"); lua_pushnumber(L, q.y1); lua_settable(L, -3);
    lua_pushstring(L, "s0"); lua_pushnumber(L, q.s0); lua_settable(L, -3);
    lua_pushstring(L, "t0"); lua_pushnumber(L, q.t0); lua_settable(L, -3);
    lua_pushstring(L, "s1"); lua_pushnumber(L, q.s1); lua_settable(L, -3);
    lua_pushstring(L, "t1"); lua_pushnumber(L, q.t1); lua_settable(L, -3);

    lua_pushnumber(L, xpos);
    lua_pushnumber(L, ypos);

    return 3;
}

static const luaL_Reg stb_font_methods[] = {
    {"get_bitmap", stb_font_get_bitmap},
    {"get_baked_quad", stb_font_get_baked_quad},
    {NULL, NULL}
};

static const luaL_Reg stb_lib[] = {
    {"bake_font", stb_bake_font},
    {NULL, NULL}
};

int luaopen_module_stb(lua_State *L) {
    luaL_newlib(L, stb_lib);

    luaL_newmetatable(L, STB_FONT_MT);
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, stb_font_gc);
    lua_settable(L, -3);

    lua_pushstring(L, "__index");
    lua_newtable(L);
    luaL_setfuncs(L, stb_font_methods, 0);
    lua_settable(L, -3);

    lua_pop(L, 1); // pop metatable

    return 1;
}