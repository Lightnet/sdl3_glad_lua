-- main.lua: Base Lua script for SDL3+GL window
-- Globals: sdl, gl, lua_util (set by C); locals via require for overrides
local sdl = sdl or require("module_sdl")
local gl = gl or require("module_gl")
local util = lua_util or require("module_lua")

util.log("Starting Lua script...")

-- Init OpenGL
local ok, err = sdl.init_gl()
if not ok then
    util.log("Failed to init GL: " .. (err or "Unknown"))
    return
end

-- Demo: Access stored context
if gl.gl_context then
    util.log("GL context stored: " .. tostring(gl.gl_context))
else
    util.log("GL context not stored (error)")
end

-- Check path (demo)
if util.path_exists("somefile.txt") then
    util.log("somefile.txt found!")
end

-- Main loop: Events + Render + Swap
local running = true
while running do
    local events = sdl.poll_events()
    for i = 1, #events do
        local ev = events[i]
        if ev.type == sdl.constants.SDL_EVENT_QUIT then
            util.log("Quit event!")
            running = false
        elseif ev.type == sdl.constants.SDL_EVENT_KEY_DOWN then
            util.log("Key down: " .. (ev.key or "unknown"))
            if ev.key == 27 then  -- ESC to quit
                running = false
            end
        elseif ev.type == sdl.constants.SDL_EVENT_MOUSE_BUTTON_DOWN then
            util.log("Mouse button " .. ev.button .. " at (" .. ev.x .. ", " .. ev.y .. ")")
        end
    end

    -- Render: Clear to teal, viewport, clear, swap
    gl.clear_color(0.2, 0.3, 0.3, 1.0)  -- Dark teal
    gl.viewport(0, 0, 800, 600)
    gl.clear()
    gl.swap_buffers()

    -- ~60 FPS delay
    local start_time = os.clock()
    while os.clock() - start_time < 1/60 do end
end

-- Cleanup on exit
gl.destroy()
sdl.quit()