-- test

local sdl = require("module_sdl")
local gl = require("module_gl")
local stb = require("module_stb")
local lua_util = require("lua_util")

-- Initialize SDL video subsystem
local success, err = sdl.init(sdl.INIT_VIDEO + sdl.INIT_EVENTS)
if not success then
    lua_util.log("Failed to initialize SDL: " .. err)
    sdl.quit()
    return
end
-- local GL_DOUBLEBUFFER = sdl.gl_get_attribute(sdl.GL_DOUBLEBUFFER)
-- print("GL_DOUBLEBUFFER 0:" .. tostring(GL_DOUBLEBUFFER))

-- GL_DOUBLEBUFFER = sdl.gl_set_attribute(sdl.GL_DOUBLEBUFFER, 1)
-- print("GL_DOUBLEBUFFER 1:" .. tostring(GL_DOUBLEBUFFER))
--does not work here 
-- GL_DOUBLEBUFFER = sdl.gl_get_attribute(sdl.GL_DOUBLEBUFFER)
-- print("GL_DOUBLEBUFFER 2:" .. tostring(GL_DOUBLEBUFFER))

-- Create window with OpenGL and resizable flags
local window, err = sdl.init_window("sdl3 cube3d", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
if not window then
    lua_util.log("Failed to create window: " .. err)
    sdl.quit()
    return
end

--does not work here 
-- GL_DOUBLEBUFFER = sdl.gl_get_attribute(sdl.GL_DOUBLEBUFFER)
-- print("GL_DOUBLEBUFFER 3:" .. tostring(GL_DOUBLEBUFFER))

-- Initialize OpenGL
local gl_context, success, err = gl.init(window)
if not success then
    lua_util.log("Failed to initialize OpenGL: " .. err)
    gl.destroy()
    sdl.quit()
    return
end
print("success: " .. tostring(success))
print("gl_context: " .. tostring(gl_context))

-- work work here 
-- note there is c gl set in module_sdl.c
GL_DOUBLEBUFFER = sdl.gl_get_attribute(sdl.GL_DOUBLEBUFFER)
print("GL_DOUBLEBUFFER:" .. tostring(GL_DOUBLEBUFFER))

local GL_RED_SIZE = sdl.gl_get_attribute(sdl.GL_RED_SIZE)
print("GL_RED_SIZE:" .. tostring(GL_RED_SIZE))

local GL_GREEN_SIZE = sdl.gl_get_attribute(sdl.GL_GREEN_SIZE)
print("GL_GREEN_SIZE:" .. tostring(GL_GREEN_SIZE))

local GL_BLUE_SIZE = sdl.gl_get_attribute(sdl.GL_BLUE_SIZE)
print("GL_BLUE_SIZE:" .. tostring(GL_BLUE_SIZE))

local GL_ALPHA_SIZE = sdl.gl_get_attribute(sdl.GL_ALPHA_SIZE)
print("GL_ALPHA_SIZE:" .. tostring(GL_ALPHA_SIZE))

local GL_DEPTH_SIZE = sdl.gl_get_attribute(sdl.GL_DEPTH_SIZE)
print("GL_DEPTH_SIZE:" .. tostring(GL_DEPTH_SIZE))

local GL_CONTEXT_PROFILE_MASK = sdl.gl_get_attribute(sdl.GL_CONTEXT_PROFILE_MASK)
print("GL_CONTEXT_PROFILE_MASK:" .. tostring(GL_CONTEXT_PROFILE_MASK))

local GL_CONTEXT_MAJOR_VERSION = sdl.gl_get_attribute(sdl.GL_CONTEXT_MAJOR_VERSION)
print("GL_CONTEXT_MAJOR_VERSION:" .. tostring(GL_CONTEXT_MAJOR_VERSION))

local GL_CONTEXT_MINOR_VERSION = sdl.gl_get_attribute(sdl.GL_CONTEXT_MINOR_VERSION)
print("GL_CONTEXT_MAGL_CONTEXT_MINOR_VERSIONJOR_VERSION:" .. tostring(GL_CONTEXT_MINOR_VERSION))


-- Set initial viewport
gl.viewport(0, 0, 800, 600)

-- Main loop
local running = true
while running do
    -- Handle events
    local events = sdl.poll_events()
    for i, event in ipairs(events) do
        if event.type == sdl.EVENT_QUIT then
            running = false
        elseif event.type == sdl.EVENT_WINDOW_RESIZED then
            lua_util.log("Window resized to " .. event.width .. "x" .. event.height)
            gl.viewport(0, 0, event.width, event.height)
        end
    end

    -- Render
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT)

    -- swap window
    sdl.gl_swap_window(window)
end

gl.destroy()
sdl.quit()