-- main.lua

local sdl = require("module_sdl")
local gl = require("module_gl") -- Global to ensure _G.gl is set for gl_init
local imgui = require("module_imgui")
local lua_util = require("lua_util")

print("gl type:", type(gl))
print("gl.delete_shader:", type(gl.delete_shader))

-- Initialize SDL video subsystem
local success, err = sdl.init(sdl.SDL_INIT_VIDEO)
if not success then
    lua_util.log("Failed to initialize SDL: " .. err)
    sdl.quit()
    return
end

-- Create window with OpenGL and resizable flags
local window, err = sdl.init_window("sdl3 cube3d", 800, 600, sdl.SDL_WINDOW_OPENGL + sdl.SDL_WINDOW_RESIZABLE)
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

-- Demo: Access stored gl context
local gl_context = gl.get_gl_context()
if not gl_context then
    print("Failed to get GL context: ", select(2, gl.get_gl_context()))
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


-- Vertex Shader
local vertexShaderSource = [[
#version 330 core
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
]]

-- Fragment Shader (fixed comment)
local fragmentShaderSource = [[
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0); // Orange color
}
]]

-- Create and compile vertex shader
local vertexShader = gl.create_shader(gl.VERTEX_SHADER)
gl.shader_source(vertexShader, vertexShaderSource)
success, err = gl.compile_shader(vertexShader)
if not success then
    lua_util.log("Vertex shader compilation failed: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Create and compile fragment shader
local fragmentShader = gl.create_shader(gl.FRAGMENT_SHADER)
gl.shader_source(fragmentShader, fragmentShaderSource)
success, err = gl.compile_shader(fragmentShader)
if not success then
    lua_util.log("Fragment shader compilation failed: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Create shader program
local shaderProgram = gl.create_program()
gl.attach_shader(shaderProgram, vertexShader)
gl.attach_shader(shaderProgram, fragmentShader)
success, err = gl.link_program(shaderProgram)
if not success then
    lua_util.log("Shader program linking failed: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Vertex data for a triangle (x, y, z)
local vertices = {
    -0.5, -0.5, 0.0,  -- Bottom-left
     0.5, -0.5, 0.0,  -- Bottom-right
     0.0,  0.5, 0.0   -- Top
}

-- Convert vertices to a binary string for glBufferData
local vertexData = ""
for _, v in ipairs(vertices) do
    vertexData = vertexData .. string.pack("f", v)
end

-- Set up VAO and VBO
local vao = gl.gen_vertex_arrays()
gl.bind_vertex_array(vao)

local vbo = gl.gen_buffers()
gl.bind_buffer(gl.ARRAY_BUFFER, vbo)
gl.buffer_data(gl.ARRAY_BUFFER, vertexData, #vertexData, gl.STATIC_DRAW)

-- Set vertex attributes
gl.vertex_attrib_pointer(0, 3, gl.FLOAT, false, 3 * 4, 0)
gl.enable_vertex_attrib_array(0)

-- Set initial viewport
gl.viewport(0, 0, 800, 600)

-- Main loop
local running = true
while running do
    -- Handle events
    -- local events = sdl.poll_events()
    local events = sdl.poll_events_ig() -- Use poll_events_ig to process ImGui inputs
    for i, event in ipairs(events) do
        if event.type == sdl.SDL_EVENT_QUIT then
            running = false
        elseif event.type == sdl.SDL_EVENT_WINDOW_RESIZED then
            print("resize window")
            gl.viewport(0, 0, event.width, event.height)
        end
    end

    -- Start ImGui frame
    imgui.new_frame() -- start drawing imgui
    -- create widgets here

    -- Create a simple ImGui window
    local open = imgui.ig_begin("Test Window", true)
    if open then
        imgui.ig_text("Hello, ImGui from Lua!")
        if imgui.ig_button("Click Me") then
            print("Button clicked!")
        end
        imgui.ig_end()
    end

    imgui.render() -- end drawing imgui

    -- Render
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear()

    gl.use_program(shaderProgram)
    gl.bind_vertex_array(vao)
    gl.draw_arrays(gl.TRIANGLES, 0, 3)

    imgui.render_draw_data() -- draw gl imgui 

    gl.swap_buffers()
end

-- Cleanup
imgui.shutdown()
gl.delete_shader(vertexShader)
gl.delete_shader(fragmentShader)
gl.delete_program(shaderProgram)
gl.destroy()
sdl.quit()