-- Load required modules
local sdl = require("module_sdl")
local gl = require("module_gl")
local cglm = require("module_cglm")
local imgui = require("module_imgui")
local lua_util = require("lua_util") -- Assuming this exists for logging

-- Initialize SDL with video and events subsystems
local success, err = sdl.init(sdl.INIT_VIDEO + sdl.INIT_EVENTS)
if not success then
    print("SDL init failed: " .. err)
    return
end

-- Create window with OpenGL and resizable flags
local window, err = sdl.init_window("sdl3 cimgui", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
if not window then
    lua_util.log("Failed to create window: " .. err)
    sdl.quit()
    return
end
print("window:" .. tostring(window))
-- Initialize OpenGL
local gl_context, success, err = gl.init(window)
if not success then
    lua_util.log("Failed to initialize OpenGL: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Set up matrices
local projection = cglm.perspective(math.rad(45), 800 / 600, 0.1, 100.0)
local view = cglm.mat4()
cglm.translate(view, cglm.vec3(0, 0, -3)) -- Move camera back
local model = cglm.mat4_identity()

-- Animation variables
local angle = 0

-- Main loop
local running = true
while running do
    -- Handle events
    local events = sdl.poll_events()
    for _, event in ipairs(events) do
        if event.type == sdl.EVENT_QUIT then
            running = false
        elseif event.type == sdl.EVENT_WINDOW_RESIZED then
            gl.viewport(0, 0, event.width, event.height)
            projection = cglm.perspective(math.rad(45), event.width / event.height, 0.1, 100.0)
        end
    end
    -- Compute MVP matrix
    local mvp = cglm.mat4()
    cglm.mat4_mul(projection, view, mvp)
    cglm.mat4_mul(mvp, model, mvp)
    print("mvp: " .. tostring(mvp))
    print("model: " .. tostring(model))

    -- Render
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT + gl.DEPTH_BUFFER_BIT)

    -- ...

    -- swap window
    sdl.gl_swap_window(window)
end

gl.destroy()
sdl.quit()