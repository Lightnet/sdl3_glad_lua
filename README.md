# sdl3_glad_lua

# license: MIT

# libraries:
- SDL 3.2
- Lua 5.4
- cimgui 1.92
- Glad 2.0.8

# Status:
- prototype.

# SDL3-Lua OpenGL Prototype

OverviewThis project is a prototype build integrating SDL 3, Glad 2.0 (for OpenGL loading), cimgui 1.92 (for ImGui-based UI), and Lua 5.4 as the primary scripting engine. The core C/C++ application serves as a lightweight runtime host, delegating nearly all scene setup, rendering, and interaction logic to Lua scripts. This allows for dynamic development of 2D/3D OpenGL scenes and an in-app editor without recompiling the binary. Key design principle: Emulate C-like control flow and OpenGL API exposure in Lua, enabling script-only modifications. Users can "execute to iterate" – load and run Lua scripts at runtime for immediate testing, with no build step required for core features.

This setup draws inspiration from prior Lua-OpenGL bindings but pushes toward fuller API exposure for rapid prototyping, with AI-assisted refinements ( Grok for Vulkan/OpenGL bridging ideas and search engine from other devs.).

## Goals:

- Script-Driven Rendering: Enable Lua scripts to fully define and render OpenGL 2D/3D scenes (e.g., vertices, shaders, textures, cameras) via a thin C-to-Lua binding layer.
- Integrated Editor: Use cimgui to build an in-runtime editor for tweaking scenes, UI, and assets directly in Lua – no external tools or recompiles needed.
- Modularity via Add-ons: Core runtime is minimal; extend with Lua-loaded modules (e.g., physics, audio) without touching C code.
- Simplicity First: Prioritize readable, concise Lua APIs that feel like "C in Lua" – e.g., direct calls to gl.BindTexture() equivalents – while keeping the host binary under 5MB.
- Runtime Iteration: Scripts load/execute on-the-fly; hot-reload scenes during playtesting for game dev workflows.


# Credits:
- Grok AI on x.
- Glad 2.0.8 github
- idea from search engine for running lua script to setup and vulkan script.

