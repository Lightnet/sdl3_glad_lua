-- Load required modules
local sdl = require("module_sdl")
local gl = require("module_gl")
local imgui = require("module_imgui")
local lua_util = require("lua_util") -- Assuming this exists for logging

-- Initialize SDL with video and events subsystems
local success, err = sdl.init(sdl.SDL_INIT_VIDEO + sdl.SDL_INIT_EVENTS)
if not success then
    print("SDL init failed: " .. err)
    return
end

-- Create window with OpenGL and resizable flags
local window, err = sdl.init_window("sdl3 cimgui", 800, 600, sdl.SDL_WINDOW_OPENGL + sdl.SDL_WINDOW_RESIZABLE)
if not window then
    lua_util.log("Failed to create window: " .. err)
    sdl.quit()
    return
end

-- Initialize OpenGL
local success, gl_context, err = gl.init(window)
if not success then
    lua_util.log("Failed to initialize OpenGL: " .. err)
    sdl.quit()
    return
end

print("window:" .. tostring(window))
print("gl_context:" .. tostring(gl_context))

-- Access stored gl context
local gl_context = gl.get_gl_context()
if not gl_context then
    print("Failed to get GL context: ", select(2, gl.get_gl_context()))
    return
end

print("gl_context:" .. tostring(gl_context))
-- print("_G.sdl_window:" .. tostring(_G.sdl_window)) --nope it nil

-- Initialize ImGui
success, err = imgui.init(window, gl_context)
if not success then
    lua_util.log("ImGui init failed: " .. err)
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
    gl.clear(gl.COLOR_BUFFER_BIT)

    -- Render ImGui
    imgui.render()
    imgui.render_draw_data()

    -- Swap window
    sdl.gl_swap_window(window)
end

-- Cleanup
imgui.shutdown()
gl.destroy()
sdl.quit()