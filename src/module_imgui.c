#define igGetIO igGetIO_Nil  // CImGui workaround

#include "module_imgui.h"
#include "module_gl.h"  // For get_gl_gl_context (if needed)
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <cimgui.h>  // CImGui
#include <cimgui_impl.h> // CImGui (your corrected include)
#include <lauxlib.h>

// Forward: Get window from Lua global (unchanged)
static SDL_Window *get_sdl_window(lua_State *L) {
    lua_getglobal(L, "sdl_window");
    SDL_Window *window = (SDL_Window *)lua_topointer(L, -1);
    lua_pop(L, 1);
    return window;
}

// Lua: imgui.init(window, gl_context) -> bool, err_msg (unchanged)
static int imgui_init(lua_State *L) {
    SDL_Window *window = (SDL_Window *)lua_topointer(L, 1);  // Arg 1: window userdata
    void *gl_context_ptr = (void *)lua_topointer(L, 2);  // Arg 2: gl_context userdata
    SDL_GLContext gl_context = (SDL_GLContext)gl_context_ptr;

    if (!window || !gl_context) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Invalid window or gl_context");
        return 2;
    }

    // Setup Dear ImGui context
    igCreateContext(NULL);
    ImGuiIO *io = igGetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup Dear ImGui style
    igStyleColorsDark(NULL);
    ImGuiStyle *style = igGetStyle();
    float main_scale = 1.0f;  // Default; pass from Lua if needed
    ImGuiStyle_ScaleAllSizes(style, main_scale);

    // Setup Platform/Renderer backends
    if (!ImGui_ImplSDL3_InitForOpenGL(window, gl_context)) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "SDL3 backend init failed");
        igDestroyContext(NULL);
        return 2;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "OpenGL3 backend init failed");
        ImGui_ImplSDL3_Shutdown();
        igDestroyContext(NULL);
        return 2;
    }

    printf("ImGui initialized successfully\n");
    lua_pushboolean(L, 1);
    return 1;
}

// Lua: imgui.new_frame() -> void (unchanged)
static int imgui_new_frame(lua_State *L) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    igNewFrame();
    return 0;
}

// imgui.render() -> void (unchanged)
static int imgui_render(lua_State *L) {
    igRender();  // Build draw list
    return 0;
}

// imgui.render_draw_data() -> void (unchanged)
static int imgui_render_draw_data(lua_State *L) {
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());  // Flush to screen
    return 0;
}

// Lua: imgui.shutdown() -> void (unchanged)
static int imgui_shutdown(lua_State *L) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    igDestroyContext(NULL);
    printf("ImGui shutdown\n");
    return 0;
}

// REWORKED: imgui.ig_begin(window_name, [p_open]) -> bool (window open?)
static int imgui_ig_begin(lua_State *L) {
    const char *window_name = luaL_checkstring(L, 1);
    bool p_open = lua_toboolean(L, 2);  // Optional: if true, push modified back
    bool *p_open_ptr = p_open ? &p_open : NULL;

    bool open = igBegin((char*)window_name, p_open_ptr, 0);  // Default flags

    lua_pushboolean(L, open);  // Return if window open
    if (p_open_ptr) {
        lua_pushboolean(L, *p_open_ptr);  // Return modified p_open (if provided)
        return 2;
    }
    return 1;
}

// REWORKED: imgui.ig_text(text) -> void
static int imgui_ig_text(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    igText((char*)text);
    return 0;
}

// REWORKED: imgui.ig_button(label, [size_x, size_y]) -> bool (clicked?)
static int imgui_ig_button(lua_State *L) {
    const char *label = luaL_checkstring(L, 1);
    float size_x = (float)luaL_optnumber(L, 2, 0.0f);
    float size_y = (float)luaL_optnumber(L, 3, 0.0f);
    ImVec2 size = {size_x, size_y};  // C-compatible struct init
    bool clicked = igButton((char*)label, size);
    lua_pushboolean(L, clicked);
    return 1;
}

// New: imgui.ig_end() -> void (end current window)
static int imgui_ig_end(lua_State *L) {
    igEnd();
    return 0;
}

static const struct luaL_Reg imgui_lib[] = {
    {"init", imgui_init},
    {"new_frame", imgui_new_frame},
    {"render", imgui_render},
    {"render_draw_data", imgui_render_draw_data},
    {"shutdown", imgui_shutdown},
    // REWORKED: Prefixed widget bindings (ig_* style)
    {"ig_begin", imgui_ig_begin},
    {"ig_text", imgui_ig_text},
    {"ig_button", imgui_ig_button},
    {"ig_end", imgui_ig_end},  // New
    {NULL, NULL}
};

int luaopen_module_imgui(lua_State *L) {
    luaL_newlib(L, imgui_lib);
    return 1;
}