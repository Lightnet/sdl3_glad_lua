// module_stb.c
#include "module_stb.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

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

// Lua: stb.load_font(filename) -> font_data, size, err_msg
static int stb_load_font(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    FILE *file = fopen(filename, "rb");
    if (!file) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to open font file");
        return 2;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *font_data = (unsigned char *)malloc(size);
    if (!font_data) {
        fclose(file);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to allocate memory for font");
        return 2;
    }

    fread(font_data, 1, size, file);
    fclose(file);

    unsigned char **ud = (unsigned char **)lua_newuserdata(L, sizeof(unsigned char *));
    *ud = font_data;
    luaL_getmetatable(L, "stb.font");
    lua_setmetatable(L, -2);
    lua_pushinteger(L, size);
    return 2;
}

// Lua: stb.font.__gc
static int stb_font_gc(lua_State *L) {
    unsigned char **ud = (unsigned char **)luaL_checkudata(L, 1, "stb.font");
    if (*ud) {
        free(*ud);
        *ud = NULL;
    }
    return 0;
}

// Lua: stb.bake_font(font_data, font_size, bitmap_width, bitmap_height) -> bitmap, cdata, bitmap_width, bitmap_height
static int stb_bake_font(lua_State *L) {
    unsigned char **font_data = (unsigned char **)luaL_checkudata(L, 1, "stb.font");
    float font_size = (float)luaL_checknumber(L, 2);
    int bitmap_width = (int)luaL_checkinteger(L, 3);
    int bitmap_height = (int)luaL_checkinteger(L, 4);

    unsigned char *bitmap = (unsigned char *)calloc(bitmap_width * bitmap_height, 1);
    if (!bitmap) {
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        lua_pushstring(L, "Failed to allocate bitmap");
        return 5;
    }

    stbtt_bakedchar *cdata = (stbtt_bakedchar *)malloc(96 * sizeof(stbtt_bakedchar));
    if (!cdata) {
        free(bitmap);
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        lua_pushstring(L, "Failed to allocate cdata");
        return 5;
    }

    int result = stbtt_BakeFontBitmap(*font_data, 0, font_size, bitmap, bitmap_width, bitmap_height, 32, 96, cdata);
    if (result < 0) {
        free(bitmap);
        free(cdata);
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        lua_pushstring(L, "Failed to bake font");
        return 5;
    }

    lua_pushlightuserdata(L, bitmap);
    lua_pushlightuserdata(L, cdata);
    lua_pushinteger(L, bitmap_width);
    lua_pushinteger(L, bitmap_height);
    return 4;
}

// Lua: stb.free_bitmap(bitmap)
static int stb_free_bitmap(lua_State *L) {
    void *bitmap = lua_touserdata(L, 1);
    if (bitmap) {
        free(bitmap);
    }
    return 0;
}

// Lua: stb.free_cdata(cdata)
static int stb_free_cdata(lua_State *L) {
    void *cdata = lua_touserdata(L, 1);
    if (cdata) {
        free(cdata);
    }
    return 0;
}

// Lua: stb.get_baked_quad(cdata, bitmap_width, bitmap_height, char, x, y) -> x0, y0, x1, y1, s0, t0, s1, t1, x_advance
static int stb_get_baked_quad(lua_State *L) {
    stbtt_bakedchar *cdata = (stbtt_bakedchar *)lua_touserdata(L, 1);
    int bitmap_width = (int)luaL_checkinteger(L, 2);
    int bitmap_height = (int)luaL_checkinteger(L, 3);
    int char_code = (int)luaL_checkinteger(L, 4);
    float x = (float)luaL_checknumber(L, 5);
    float y = (float)luaL_checknumber(L, 6);
    
    stbtt_aligned_quad q;
    float xpos = x;
    float ypos = y;
    
    stbtt_GetBakedQuad(cdata, bitmap_width, bitmap_height, char_code - 32, &xpos, &ypos, &q, 1);
    
    lua_pushnumber(L, q.x0);
    lua_pushnumber(L, q.y0);
    lua_pushnumber(L, q.x1);
    lua_pushnumber(L, q.y1);
    lua_pushnumber(L, q.s0);
    lua_pushnumber(L, q.t0);
    lua_pushnumber(L, q.s1);
    lua_pushnumber(L, q.t1);
    lua_pushnumber(L, xpos - x); // x_advance
    return 9;
}



// Lua: stb.get_font_vmetrics(font_data) -> ascent, descent, lineGap
static int stb_get_font_vmetrics(lua_State *L) {
    unsigned char **font_data = (unsigned char **)luaL_checkudata(L, 1, "stb.font");
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, *font_data, stbtt_GetFontOffsetForIndex(*font_data, 0))) {
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to initialize font");
        return 4;
    }
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    lua_pushinteger(L, ascent);
    lua_pushinteger(L, descent);
    lua_pushinteger(L, lineGap);
    return 3;
}

// Lua: stb.test_render_text(font_data, font_size, width, height, text)
static int stb_test_render_text(lua_State *L) {
    unsigned char **font_data = (unsigned char **)luaL_checkudata(L, 1, "stb.font");
    float font_size = (float)luaL_checknumber(L, 2);
    int width = (int)luaL_checkinteger(L, 3);
    int height = (int)luaL_checkinteger(L, 4);
    const char *text = luaL_checkstring(L, 5);

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, *font_data, stbtt_GetFontOffsetForIndex(*font_data, 0))) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to initialize font");
        return 2;
    }

    float scale = stbtt_ScaleForPixelHeight(&font, font_size);
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
    float xpos = 2;
    float ypos = 2 + ascent * scale;
    int max_width = 0;

    unsigned char *bitmap = (unsigned char *)calloc(width * height, 1);
    if (!bitmap) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to allocate bitmap");
        return 2;
    }

    for (const char *c = text; *c; c++) {
        int advance, lsb, x0, y0, x1, y1;
        stbtt_GetCodepointHMetrics(&font, *c, &advance, &lsb);
        stbtt_GetCodepointBitmapBox(&font, *c, scale, scale, &x0, &y0, &x1, &y1);
        
        int w = x1 - x0;
        int h = y1 - y0;
        if (xpos + w < width && ypos + h < height) {
            stbtt_MakeCodepointBitmap(&font, bitmap + (int)ypos * width + (int)xpos + x0, w, h, width, scale, scale, *c);
            printf("Test rendered char '%c': x0=%d, y0=%d, w=%d, h=%d, xpos=%.2f, ypos=%.2f\n", *c, x0, y0, w, h, xpos, ypos);
        }
        xpos += advance * scale;
        if (xpos > max_width) max_width = xpos;
    }

    int non_zero_pixels = 0;
    int min_pixel = 255, max_pixel = 0;
    for (int i = 0; i < width * height; i++) {
        if (bitmap[i] > 0) non_zero_pixels++;
        if (bitmap[i] < min_pixel) min_pixel = bitmap[i];
        if (bitmap[i] > max_pixel) max_pixel = bitmap[i];
    }
    printf("Test bitmap: %d non-zero pixels out of %d, min_pixel=%d, max_pixel=%d\n", 
           non_zero_pixels, width * height, min_pixel, max_pixel);

    printf("Sample 5x5 bitmap region (x=10, y=10):\n");
    for (int y = 10; y < 15; y++) {
        printf("Row %d: ", y);
        for (int x = 10; x < 15; x++) {
            int idx = y * width + x;
            printf("%3d ", idx < width * height ? bitmap[idx] : 0);
        }
        printf("\n");
    }

    free(bitmap);
    lua_pushboolean(L, 1);
    return 1;
}

// Lua: stb.dump_bitmap(bitmap, width, height)
static int stb_dump_bitmap(lua_State *L) {
    unsigned char *bitmap = (unsigned char *)lua_touserdata(L, 1);
    int width = (int)luaL_checkinteger(L, 2);
    int height = (int)luaL_checkinteger(L, 3);

    if (!bitmap) {
        lua_pushnil(L);
        lua_pushstring(L, "Invalid bitmap");
        return 2;
    }

    int non_zero_pixels = 0;
    int min_pixel = 255, max_pixel = 0;
    for (int i = 0; i < width * height; i++) {
        if (bitmap[i] > 0) non_zero_pixels++;
        if (bitmap[i] < min_pixel) min_pixel = bitmap[i];
        if (bitmap[i] > max_pixel) max_pixel = bitmap[i];
    }
    printf("Dumped bitmap: %d non-zero pixels out of %d, min_pixel=%d, max_pixel=%d\n", 
           non_zero_pixels, width * height, min_pixel, max_pixel);

    printf("Sample 5x5 bitmap region (x=2, y=27):\n");
    for (int y = 27; y < 32; y++) {
        printf("Row %d: ", y);
        for (int x = 2; x < 7; x++) {
            int idx = y * width + x;
            printf("%3d ", idx < width * height ? bitmap[idx] : 0);
        }
        printf("\n");
    }

    lua_pushboolean(L, 1);
    return 1;
}

static const struct luaL_Reg stb_lib[] = {
    {"load_image", stb_load_image},
    {"free_image", stb_free_image},
    {"load_font", stb_load_font},
    {"bake_font", stb_bake_font},
    {"free_bitmap", stb_free_bitmap},
    {"free_cdata", stb_free_cdata},
    {"get_baked_quad", stb_get_baked_quad},
    {"get_font_vmetrics", stb_get_font_vmetrics}, // Added
    {"test_render_text", stb_test_render_text},
    {"dump_bitmap", stb_dump_bitmap},
    {NULL, NULL}
};

static const luaL_Reg stb_font_meta[] = {
    {"__gc", stb_font_gc},
    {NULL, NULL}
};

int luaopen_module_stb(lua_State *L) {
    luaL_newlib(L, stb_lib);
    
    luaL_newmetatable(L, "stb.font");
    luaL_setfuncs(L, stb_font_meta, 0);
    lua_pop(L, 1);
    
    lua_pushinteger(L, 1); lua_setfield(L, -2, "GREY");
    lua_pushinteger(L, 2); lua_setfield(L, -2, "GREY_ALPHA");
    lua_pushinteger(L, 3); lua_setfield(L, -2, "RGB");
    lua_pushinteger(L, 4); lua_setfield(L, -2, "RGBA");
    
    return 1;
}