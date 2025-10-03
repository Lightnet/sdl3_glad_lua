-- Load modules as local variables
local sdl = require("module_sdl")
gl = require("module_gl")
local imgui = require("module_imgui")

-- Initialize SDL with video and events subsystems
local success, err = sdl.init(sdl.constants.SDL_INIT_VIDEO + sdl.constants.SDL_INIT_EVENTS)
if not success then
    print("SDL init failed: " .. err)
    return
end

-- Create an SDL window with OpenGL support
success, err = sdl.init_window(800, 600, sdl.constants.SDL_WINDOW_OPENGL + sdl.constants.SDL_WINDOW_RESIZABLE)
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

-- gl.get_gl_context()

-- Get the OpenGL context
local gl_context, err = gl.get_gl_context()
if not gl_context then
    print("Failed to get GL context: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Initialize ImGui with sdl_window and gl_context
success, err = imgui.init(_G.sdl_window, gl_context)
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
    local events = sdl.poll_events_ig()
    for i, event in ipairs(events) do
        if event.type == sdl.constants.SDL_EVENT_QUIT then
            running = false
        elseif event.type == sdl.constants.SDL_EVENT_WINDOW_RESIZED then
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