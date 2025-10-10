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

// Lua: stb.render_text(font_data, font_size, text, width, height) -> bitmap, width, height, channels, err_msg
static int stb_render_text(lua_State *L) {
    unsigned char **font_data = (unsigned char **)luaL_checkudata(L, 1, "stb.font");
    float font_size = (float)luaL_checknumber(L, 2);
    const char *text = luaL_checkstring(L, 3);
    int width = (int)luaL_optinteger(L, 4, 256);
    int height = (int)luaL_optinteger(L, 5, 64);

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, *font_data, stbtt_GetFontOffsetForIndex(*font_data, 0))) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to initialize font");
        return 2;
    }

    unsigned char *bitmap = (unsigned char *)calloc(width * height, 1);
    if (!bitmap) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to allocate bitmap");
        return 2;
    }

    float scale = stbtt_ScaleForPixelHeight(&font, font_size);
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
    float xpos = 2;
    float ypos = 2 + ascent * scale;
    int max_width = 0;

    for (const char *c = text; *c; c++) {
        int advance, lsb, x0, y0, x1, y1;
        stbtt_GetCodepointHMetrics(&font, *c, &advance, &lsb);
        stbtt_GetCodepointBitmapBox(&font, *c, scale, scale, &x0, &y0, &x1, &y1);
        
        int w = x1 - x0;
        int h = y1 - y0;
        if (xpos + w < width && ypos + h < height) {
            stbtt_MakeCodepointBitmap(&font, bitmap + (int)ypos * width + (int)xpos + x0, w, h, width, scale, scale, *c);
            printf("Rendered char '%c': x0=%d, y0=%d, w=%d, h=%d, xpos=%.2f, ypos=%.2f\n", *c, x0, y0, w, h, xpos, ypos);
        }
        xpos += advance * scale;
        if (xpos > max_width) max_width = xpos;
    }

    // Debug bitmap content
    int non_zero_pixels = 0;
    int min_pixel = 255, max_pixel = 0;
    for (int i = 0; i < width * height; i++) {
        if (bitmap[i] > 0) non_zero_pixels++;
        if (bitmap[i] < min_pixel) min_pixel = bitmap[i];
        if (bitmap[i] > max_pixel) max_pixel = bitmap[i];
    }
    printf("Text bitmap: %d non-zero pixels out of %d, min_pixel=%d, max_pixel=%d\n", 
           non_zero_pixels, width * height, min_pixel, max_pixel);

    lua_pushlightuserdata(L, bitmap);
    lua_pushinteger(L, max_width);
    lua_pushinteger(L, (int)(font_size * 1.5));
    lua_pushinteger(L, 1);
    return 4;
}

// Lua: stb.free_bitmap(bitmap)
static int stb_free_bitmap(lua_State *L) {
    unsigned char *bitmap = (unsigned char *)lua_touserdata(L, 1);
    if (bitmap) free(bitmap);
    return 0;
}



// Lua: stb.test_render_text(font, font_size, text, width, height)
static int stb_test_render_text(lua_State *L) {
    unsigned char **font_data = (unsigned char **)luaL_checkudata(L, 1, "stb.font");
    float font_size = (float)luaL_checknumber(L, 2);
    const char *text = luaL_checkstring(L, 3);
    int width = (int)luaL_optinteger(L, 4, 256);
    int height = (int)luaL_optinteger(L, 5, 64);

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, *font_data, stbtt_GetFontOffsetForIndex(*font_data, 0))) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to initialize font");
        return 2;
    }

    unsigned char *bitmap = (unsigned char *)calloc(width * height, 1);
    if (!bitmap) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to allocate bitmap");
        return 2;
    }

    float scale = stbtt_ScaleForPixelHeight(&font, font_size);
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
    float xpos = 2;
    float ypos = 2 + ascent * scale;
    int max_width = 0;

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

    // Debug bitmap content
    int non_zero_pixels = 0;
    int min_pixel = 255, max_pixel = 0;
    for (int i = 0; i < width * height; i++) {
        if (bitmap[i] > 0) non_zero_pixels++;
        if (bitmap[i] < min_pixel) min_pixel = bitmap[i];
        if (bitmap[i] > max_pixel) max_pixel = bitmap[i];
    }
    printf("Test bitmap: %d non-zero pixels out of %d, min_pixel=%d, max_pixel=%d\n", 
           non_zero_pixels, width * height, min_pixel, max_pixel);

    // Sample a 5x5 region
    printf("Sample 5x5 bitmap region (x=10, y=10):\n");
    for (int y = 10; y < 15; y++) {
        printf("Row %d: ", y);
        for (int x = 10; x < 15; x++) {
            int idx = y * width + x;
            printf("%3d ", idx < width * height ? bitmap[idx] : 0);
        }
        printf("\n");
    }

    free(bitmap); // Free bitmap since not used for rendering
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

    // Sample a 5x5 region (adjusted to x=2, y=27)
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
    {"render_text", stb_render_text},
    {"free_bitmap", stb_free_bitmap},

    {"test_render_text", stb_test_render_text}, // Added
    {"dump_bitmap", stb_dump_bitmap}, // Added


    {NULL, NULL}
};

static const luaL_Reg stb_font_meta[] = {
    {"__gc", stb_font_gc},
    {NULL, NULL}
};

int luaopen_module_stb(lua_State *L) {
    luaL_newlib(L, stb_lib);
    
    // Register stb.font metatable
    luaL_newmetatable(L, "stb.font");
    luaL_setfuncs(L, stb_font_meta, 0);
    lua_pop(L, 1);
    
    // Expose constants
    lua_pushinteger(L, 1); lua_setfield(L, -2, "GREY");
    lua_pushinteger(L, 2); lua_setfield(L, -2, "GREY_ALPHA");
    lua_pushinteger(L, 3); lua_setfield(L, -2, "RGB");
    lua_pushinteger(L, 4); lua_setfield(L, -2, "RGBA");
    
    return 1;
}