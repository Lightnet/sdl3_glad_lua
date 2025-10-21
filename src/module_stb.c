// module_stb.c (relevant parts only)
#include "module_stb.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>


static const char *STB_FONT_MT = "stb_font";
static const char *STB_IMAGE_MT = "stb_image";

typedef struct {
    unsigned char *ttf_buffer;
    unsigned char *bitmap;
    stbtt_bakedchar *cdata;
    int width;
    int height;
    int first_char;
    int num_chars;
} stb_font;

typedef struct {
    unsigned char *data;
    int width;
    int height;
    int channels;
} stb_image;

//===============================================
// image
//===============================================

/*
local img, err = stb.load_image("resources/ph16.png", stb.RGBA)
if not img then
    lua_util.log("Failed to load image: " .. err)
    gl.destroy()
    sdl.quit()
    return
end
local image_data = img:get_data()
local width = img:get_width()
local height = img:get_height()
local channels = img:get_channels()
*/

// Lua: stb.load_image(file_path, [desired_channels]) -> image | nil, err_msg
static int stb_load_image(lua_State *L) {
    const char *file_path = luaL_checkstring(L, 1);
    int desired_channels = (int)luaL_optinteger(L, 2, 0); // 0 means use file's channels
    int width, height, channels;
    unsigned char *data = stbi_load(file_path, &width, &height, &channels, desired_channels);
    if (!data) {
        lua_pushnil(L);
        lua_pushstring(L, stbi_failure_reason() ? stbi_failure_reason() : "Unknown error loading image");
        return 2;
    }
    stb_image *img = (stb_image *)lua_newuserdata(L, sizeof(stb_image));
    img->data = data;
    img->width = width;
    img->height = height;
    img->channels = desired_channels ? desired_channels : channels;
    luaL_setmetatable(L, STB_IMAGE_MT);
    return 1;
}

// Lua: image:free()
static int stb_image_free(lua_State *L) {
    stb_image *img = (stb_image *)luaL_checkudata(L, 1, STB_IMAGE_MT);
    if (img->data) {
        stbi_image_free(img->data);
        img->data = NULL; // Prevent double-free
    }
    return 0;
}

static int stb_free_image(lua_State *L) {
    void *data = lua_touserdata(L, 1);
    if (data) {
        stbi_image_free(data);
    }
    return 0;
}

static int stb_image_get_data(lua_State *L) {
    stb_image *img = (stb_image *)luaL_checkudata(L, 1, STB_IMAGE_MT);
    if (!img->data) {
        lua_pushnil(L);
        return 1;
    }
    // printf("Image data: first 4 bytes: %d, %d, %d, %d\n",
    //        img->data[0], img->data[1], img->data[2], img->data[3]);
    lua_pushlightuserdata(L, img->data);
    return 1;
}

// Lua: image.width, image.height, image.channels (accessors)
static int stb_image_get_width(lua_State *L) {
    stb_image *img = (stb_image *)luaL_checkudata(L, 1, STB_IMAGE_MT);
    lua_pushinteger(L, img->width);
    return 1;
}

static int stb_image_get_height(lua_State *L) {
    stb_image *img = (stb_image *)luaL_checkudata(L, 1, STB_IMAGE_MT);
    lua_pushinteger(L, img->height);
    return 1;
}

static int stb_image_get_channels(lua_State *L) {
    stb_image *img = (stb_image *)luaL_checkudata(L, 1, STB_IMAGE_MT);
    lua_pushinteger(L, img->channels);
    return 1;
}

static const luaL_Reg stb_image_methods[] = {
    {"free", stb_image_free},
    {"get_data", stb_image_get_data},
    {"get_width", stb_image_get_width},
    {"get_height", stb_image_get_height},
    {"get_channels", stb_image_get_channels},
    {NULL, NULL}
};

//===============================================
// typefont
//===============================================

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

void init_stb_font_metatable(lua_State *L) {
    luaL_newmetatable(L, STB_FONT_MT);
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, stb_font_gc);
    lua_settable(L, -3);

    lua_pushstring(L, "__index");
    lua_newtable(L);
    luaL_setfuncs(L, stb_font_methods, 0);
    lua_settable(L, -3);

    lua_pop(L, 1); // pop metatable
}

void init_stb_image_metatable(lua_State *L) {
    luaL_newmetatable(L, STB_IMAGE_MT);
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, stb_image_free);
    lua_settable(L, -3);

    lua_pushstring(L, "__index");
    lua_newtable(L);
    luaL_setfuncs(L, stb_image_methods, 0);
    lua_settable(L, -3);

    lua_pop(L, 1); // pop metatable
}

static const luaL_Reg stb_lib[] = {
    {"load_image", stb_load_image},
    {"free_image", stb_free_image},
    {"bake_font", stb_bake_font},
    {NULL, NULL}
};

int luaopen_module_stb(lua_State *L) {
    luaL_newlib(L, stb_lib);

    init_stb_font_metatable(L);
    init_stb_image_metatable(L);

    lua_pushinteger(L, 1); lua_setfield(L, -2, "GREY");
    lua_pushinteger(L, 2); lua_setfield(L, -2, "GREY_ALPHA");
    lua_pushinteger(L, 3); lua_setfield(L, -2, "RGB");
    lua_pushinteger(L, 4); lua_setfield(L, -2, "RGBA");

    return 1;
}