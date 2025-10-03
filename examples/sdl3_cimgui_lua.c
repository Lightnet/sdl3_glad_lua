// SDL 3.2.22
// cimgui 1.92.2 or master
// lua 5.4.8

// main.c
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <cimgui.h>
#include <cimgui_impl.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define igGetIO igGetIO_Nil

// Lua binding for igBegin
static int lua_igBegin(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    bool p_open = lua_toboolean(L, 2);
    int flags = luaL_optinteger(L, 3, 0);
    bool *p_open_ptr = p_open ? &p_open : NULL;
    bool result = igBegin(name, p_open_ptr, flags);
    if (p_open_ptr) lua_pushboolean(L, *p_open_ptr);
    else lua_pushnil(L);
    lua_pushboolean(L, result);
    return 2;
}

// Lua binding for igEnd
static int lua_igEnd(lua_State *L) {
    igEnd();
    return 0;
}

// Lua binding for igText
static int lua_igText(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    igText("%s", text);
    return 0;
}

// Lua binding for igCheckbox
static int lua_igCheckbox(lua_State *L) {
    const char *label = luaL_checkstring(L, 1);
    bool value = lua_toboolean(L, 2);
    bool result = igCheckbox(label, &value);
    lua_pushboolean(L, value);
    lua_pushboolean(L, result);
    return 2;
}

// Lua binding for igSliderFloat
static int lua_igSliderFloat(lua_State *L) {
    const char *label = luaL_checkstring(L, 1);
    float value = (float)luaL_checknumber(L, 2);
    float min = (float)luaL_checknumber(L, 3);
    float max = (float)luaL_checknumber(L, 4);
    const char *format = luaL_optstring(L, 5, "%.3f");
    int flags = luaL_optinteger(L, 6, 0);
    bool result = igSliderFloat(label, &value, min, max, format, flags);
    lua_pushnumber(L, value);
    lua_pushboolean(L, result);
    return 2;
}

// Lua binding for igColorEdit4
static int lua_igColorEdit4(lua_State *L) {
    const char *label = luaL_checkstring(L, 1);
    ImVec4 color;
    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "x"); color.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
        lua_getfield(L, 2, "y"); color.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
        lua_getfield(L, 2, "z"); color.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
        lua_getfield(L, 2, "w"); color.w = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
    } else {
        color = (ImVec4){0.45f, 0.55f, 0.60f, 1.00f};
    }
    int flags = luaL_optinteger(L, 3, 0);
    bool result = igColorEdit4(label, (float*)&color, flags);
    lua_newtable(L);
    lua_pushnumber(L, color.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, color.y); lua_setfield(L, -2, "y");
    lua_pushnumber(L, color.z); lua_setfield(L, -2, "z");
    lua_pushnumber(L, color.w); lua_setfield(L, -2, "w");
    lua_pushboolean(L, result);
    return 2;
}

// Lua binding for igButton
static int lua_igButton(lua_State *L) {
    const char *label = luaL_checkstring(L, 1);
    ImVec2 size = {0, 0};
    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "x"); size.x = (float)luaL_optnumber(L, -1, 0); lua_pop(L, 1);
        lua_getfield(L, 2, "y"); size.y = (float)luaL_optnumber(L, -1, 0); lua_pop(L, 1);
    }
    bool result = igButton(label, size);
    lua_pushboolean(L, result);
    return 1;
}

// Lua binding for igSameLine
static int lua_igSameLine(lua_State *L) {
    float offset = (float)luaL_optnumber(L, 1, 0.0f);
    float spacing = (float)luaL_optnumber(L, 2, -1.0f);
    igSameLine(offset, spacing);
    return 0;
}

// Lua binding for igGetFramerate
static int lua_igGetFramerate(lua_State *L) {
    ImGuiIO *io = igGetIO();
    lua_pushnumber(L, io->Framerate);
    return 1;
}

int main() {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        fprintf(stderr, "Failed to init video! %s\n", SDL_GetError());
        return 1;
    }

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_OPENGL;
    SDL_Window *window = SDL_CreateWindow("Dear cImGui SDL3+OpenGL3 example", (int)(1280 * main_scale), (int)(720 * main_scale), window_flags);
    if (!window) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Create OpenGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_GL_MakeCurrent(window, gl_context);

    // Setup Dear ImGui context
    igCreateContext(NULL); // imgui
    ImGuiIO *io = igGetIO(); // imgui
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // imgui
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // imgui

    // Setup Dear ImGui style
    igStyleColorsDark(NULL); // imgui
    ImGuiStyle *style = igGetStyle(); // imgui
    ImGuiStyle_ScaleAllSizes(style, main_scale); // imgui

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context); // imgui
    ImGui_ImplOpenGL3_Init("#version 330"); // imgui

    // Initialize Lua
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    // Register cimgui functions to Lua
    lua_newtable(L);
    lua_pushcfunction(L, lua_igBegin); lua_setfield(L, -2, "Begin");
    lua_pushcfunction(L, lua_igEnd); lua_setfield(L, -2, "End");
    lua_pushcfunction(L, lua_igText); lua_setfield(L, -2, "Text");
    lua_pushcfunction(L, lua_igCheckbox); lua_setfield(L, -2, "Checkbox");
    lua_pushcfunction(L, lua_igSliderFloat); lua_setfield(L, -2, "SliderFloat");
    lua_pushcfunction(L, lua_igColorEdit4); lua_setfield(L, -2, "ColorEdit4");
    lua_pushcfunction(L, lua_igButton); lua_setfield(L, -2, "Button");
    lua_pushcfunction(L, lua_igSameLine); lua_setfield(L, -2, "SameLine");
    lua_pushcfunction(L, lua_igGetFramerate); lua_setfield(L, -2, "GetFramerate");
    lua_setglobal(L, "imgui");

    // Load script.lua
    if (luaL_dofile(L, "script.lua") != LUA_OK) {
        fprintf(stderr, "Error loading script.lua: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return -1;
    }

    // Global clear color
    ImVec4 clear_color = {0.45f, 0.55f, 0.60f, 1.00f};

    // Main loop
    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event); // imgui
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame(); // imgui
        ImGui_ImplSDL3_NewFrame(); // imgui
        igNewFrame(); // imgui

        // Call Lua render_frame function
        lua_getglobal(L, "render_frame");
        lua_newtable(L);
        lua_pushnumber(L, clear_color.x); lua_setfield(L, -2, "x");
        lua_pushnumber(L, clear_color.y); lua_setfield(L, -2, "y");
        lua_pushnumber(L, clear_color.z); lua_setfield(L, -2, "z");
        lua_pushnumber(L, clear_color.w); lua_setfield(L, -2, "w");
        if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
            fprintf(stderr, "Error in render_frame: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        } else if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "x"); clear_color.x = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_getfield(L, -1, "y"); clear_color.y = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_getfield(L, -1, "z"); clear_color.z = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_getfield(L, -1, "w"); clear_color.w = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_pop(L, 1);
        }

        // Rendering
        igRender(); // imgui
        glViewport(0, 0, (int)io->DisplaySize.x, (int)io->DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData()); // imgui
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    lua_close(L);
    ImGui_ImplOpenGL3_Shutdown(); // imgui
    ImGui_ImplSDL3_Shutdown(); // imgui
    igDestroyContext(NULL); // imgui
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}