-- main.lua: Base Lua script for SDL3+GL window
-- Globals: sdl, gl, lua_util (set by C); locals via require for overrides
local sdl = require("module_sdl")
local gl = require("module_gl")
local lua_util = require("lua_util")

-- Initialize SDL with video and events subsystems
local success, err = sdl.init(sdl.INIT_VIDEO + sdl.INIT_EVENTS)
if not success then
    print("SDL init failed: " .. err)
    return
end

-- Create window with OpenGL and resizable flags
local window, err = sdl.init_window("sdl3 cube3d", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
if not window then
    lua_util.log("Failed to create window: " .. err)
    sdl.quit()
    return
end

-- Get the OpenGL context
local gl_context, success, err = gl.init(window)
if not success then
    print("Failed to get GL context: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

print("success: " .. tostring(success))
print("gl_context: " .. tostring(gl_context))

-- Get the OpenGL context
local gl_context, err = gl.get_gl_context()
if not gl_context then
    print("Failed to get GL context: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- -- Check path (demo)
-- if util.path_exists("somefile.txt") then
--     util.log("somefile.txt found!")
-- end

-- Main loop: Events + Render + Swap
local running = true
while running do
    local events = sdl.poll_events()
    for i = 1, #events do
        local ev = events[i]
        if ev.type == sdl.EVENT_QUIT then
            lua_util.log("Quit event!")
            running = false
        elseif ev.type == sdl.EVENT_KEY_DOWN then
            lua_util.log("Key down: " .. (ev.key or "unknown"))
            if ev.key == 27 then  -- ESC to quit
                running = false
            end
        elseif ev.type == sdl.EVENT_MOUSE_BUTTON_DOWN then
            lua_util.log("Mouse button " .. ev.button .. " at (" .. ev.x .. ", " .. ev.y .. ")")
        end
    end

    -- Render: Clear to teal, viewport, clear, swap
    gl.clear_color(0.2, 0.3, 0.3, 1.0)  -- Dark teal
    gl.viewport(0, 0, 800, 600)
    gl.clear(gl.COLOR_BUFFER_BIT)
    

    -- swap window
    sdl.gl_swap_window(window)
    -- -- ~60 FPS delay
    -- local start_time = os.clock()
    -- while os.clock() - start_time < 1/60 do end
end

-- Cleanup on exit
gl.destroy()
sdl.quit()