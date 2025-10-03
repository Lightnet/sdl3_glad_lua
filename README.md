# sdl3_glad_lua

# License: MIT

# Programing Language:
- c programing language
- lua 
- cmake ( build config )

# libraries:
- SDL 3.2
- Lua 5.4
- cimgui 1.92
- Glad 2.0.8
- zpl-c/enet v2.6.5

# Status:
- prototype.

# SDL3-Lua OpenGL Prototype:

## Overview:
This project is a prototype build integrating SDL 3, Glad 2.0 (for OpenGL loading), cimgui 1.92 (for ImGui-based UI), and Lua 5.4 as the primary scripting engine. The core C/C++ application serves as a lightweight runtime host, delegating nearly all scene setup, rendering, and interaction logic to Lua scripts. This allows for dynamic development of 2D/3D OpenGL scenes and an in-app editor without recompiling the binary. Key design principle: Emulate C-like control flow and OpenGL API exposure in Lua, enabling script-only modifications. Users can "execute to iterate" – load and run Lua scripts at runtime for immediate testing, with no build step required for core features.

This setup draws inspiration from prior Lua-OpenGL bindings but pushes toward fuller API exposure for rapid prototyping, with AI-assisted refinements ( Grok for Vulkan/OpenGL bridging ideas and search engine from other devs.).

## Goals:

- Script-Driven Rendering: Enable Lua scripts to fully define and render OpenGL 2D/3D scenes (e.g., vertices, shaders, textures, cameras) via a thin C-to-Lua binding layer.
- Integrated Editor: Use cimgui to build an in-runtime editor for tweaking scenes, UI, and assets directly in Lua – no external tools or recompiles needed.
- Modularity via Add-ons: Core runtime is minimal; extend with Lua-loaded modules (e.g., physics, audio) without touching C code.
- Simplicity First: Prioritize readable, concise Lua APIs that feel like "C in Lua" – e.g., direct calls to gl.BindTexture() equivalents – while keeping the host binary under 5MB.
- Runtime Iteration: Scripts load/execute on-the-fly; hot-reload scenes during playtesting for game dev workflows.

# Lua Features:
- [x] imgui simple
- [x] opengl glad 2.0
- [x] sdl 3.2
- [x] lua 5.4
- [x] shaders
- [x] triangle 2D
- [ ] network
- [ ] ...

# Examples:
- server.lua
- client.lua
- triangle.lua
- sdl3_cimgui.lua

# lua script:
```lua
-- Load required modules
local sdl = require("module_sdl")
gl = require("module_gl") -- can't use local due config in module_gl.c need rework later.
local imgui = require("module_imgui")

-- Initialize SDL with video and events subsystems
local success, err = sdl.init(sdl.SDL_INIT_VIDEO + sdl.SDL_INIT_EVENTS)
if not success then
    print("SDL init failed: " .. err)
    return
end

-- Create an SDL window with OpenGL support
success, err = sdl.init_window(800, 600, sdl.SDL_WINDOW_OPENGL + sdl.SDL_WINDOW_RESIZABLE)
if not success then
    print("Window creation failed: " .. err)
    sdl.quit()
    return
end

-- Initialize OpenGL context
success, err = gl.init()
if not success then
    print("OpenGL init failed: " .. err)
    sdl.quit()
    return
end

-- Initialize ImGui with sdl_window and gl_context
success, err = imgui.init(_G.sdl_window, gl.gl_context)
if not success then
    print("ImGui init failed: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Main loop
local running = true
while running do
    -- Poll SDL events
    local events = sdl.poll_events_ig() -- Use poll_events_ig to process ImGui inputs
    for i, event in ipairs(events) do
        if event.type == sdl.SDL_EVENT_QUIT then
            running = false
        elseif event.type == sdl.SDL_EVENT_WINDOW_RESIZED then
            gl.viewport(0, 0, event.width, event.height)
        end
    end

    -- Start ImGui frame
    imgui.new_frame()

    -- Create a simple ImGui window
    local open = imgui.ig_begin("Test Window", true)
    if open then
        imgui.ig_text("Hello, ImGui from Lua!")
        if imgui.ig_button("Click Me") then
            print("Button clicked!")
        end
        imgui.ig_end()
    end

    -- Clear the screen
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear()

    -- Render ImGui
    imgui.render()
    imgui.render_draw_data()

    -- Swap buffers
    gl.swap_buffers()
end

-- Cleanup
imgui.shutdown()
gl.destroy()
sdl.quit()
```
  The Grok AI agent keep it simple still need to refine some api to make it easy later.

# Bugs:
- Note that SDL 3.2.22 basic set up for SDL_GL_DOUBLEBUFFER, SDL_GL_DEPTH_SIZE and other but not for SDL_GL_CONTEXT_MAJOR_VERSION and SDL_GL_CONTEXT_MINOR_VERSION. Unless I config it wrong but check on SDL 2.x which is working on simple test.

# Credits:
- Grok AI on x.
- Glad 2.0.8 github
- idea from search engine for running lua script to setup and vulkan script.

