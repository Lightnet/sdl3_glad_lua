-- main.lua: Base Lua script for SDL3+GL window
local sdl = require("module_sdl")
local gl = require("module_gl")
local util = require("module_lua")

util.log("Starting Lua script...")

-- Init OpenGL
local ok, err = sdl.init_gl()
if not ok then
    util.log("Failed to init GL: " .. (err or "Unknown"))
    return
end

-- Check path (demo)
if util.path_exists("somefile.txt") then
    util.log("somefile.txt found!")
end

-- Main loop: Events + Render + Swap
while true do
    local events = sdl.poll_events()
    local should_quit = false
    for i = 1, #events do
        local ev = events[i]
        if ev.type == sdl.constants.SDL_EVENT_QUIT then
            util.log("Quit event!")
            should_quit = true
        elseif ev.type == sdl.constants.SDL_EVENT_KEY_DOWN then
            util.log("Key down: " .. (ev.key or "unknown"))
        elseif ev.type == sdl.constants.SDL_EVENT_MOUSE_BUTTON_DOWN then
            util.log("Mouse button " .. ev.button .. " at (" .. ev.x .. ", " .. ev.y .. ")")
        end
    end
    
    if should_quit then
        sdl.quit()
        break
    end

    -- Render: Clear to teal, viewport, clear, swap
    gl.clear_color(0.2, 0.3, 0.3, 1.0)  -- Dark teal
    gl.viewport(0, 0, 800, 600)
    gl.clear()
    gl.swap_buffers()  -- New: Flip buffers to display

    -- ~60 FPS delay (Lua: os.clock() or coroutine.yield() for advanced)
    local start_time = os.clock()
    while os.clock() - start_time < 1/60 do end  -- Simple busy-wait; replace with sleep if needed
end