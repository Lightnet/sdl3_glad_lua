-- main.lua

local sdl = require("module_sdl")
gl = require("module_gl") -- Global to ensure _G.gl is set for gl_init
local lua_util = require("lua_util")

-- Initialize SDL video subsystem (required for window creation)
-- In headless mode, this could be skipped
SDL_Init = SDL_Init or function(subsystems) end -- Fallback if not available
SDL_Init(0x00000020) -- SDL_INIT_VIDEO

-- Create window with OpenGL and resizable flags
local success, err = sdl.init_window(800, 600, sdl.constants.SDL_WINDOW_OPENGL + sdl.constants.SDL_WINDOW_RESIZABLE)
if not success then
    lua_util.log("Failed to create window: " .. err)
    sdl.quit()
    return
end

-- Initialize OpenGL
success, err = gl.init()
if not success then
    lua_util.log("Failed to initialize OpenGL: " .. err)
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

-- Fragment Shader (fixed comment syntax)
local fragmentShaderSource = [[
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0); // Orange color
}
]]

-- Create and compile vertex shader
local vertexShader = gl.create_shader(gl.constants.VERTEX_SHADER)
gl.shader_source(vertexShader, vertexShaderSource)
local success, err = gl.compile_shader(vertexShader)
if not success then
    lua_util.log("Vertex shader compilation failed: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Create and compile fragment shader
local fragmentShader = gl.create_shader(gl.constants.FRAGMENT_SHADER)
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
gl.bind_buffer(gl.constants.ARRAY_BUFFER, vbo)
gl.buffer_data(gl.constants.ARRAY_BUFFER, vertexData, #vertexData, gl.constants.STATIC_DRAW)

-- Set vertex attributes
gl.vertex_attrib_pointer(0, 3, gl.constants.FLOAT, false, 3 * 4, 0)
gl.enable_vertex_attrib_array(0)

-- Set initial viewport
gl.viewport(0, 0, 800, 600)

-- Main loop
local running = true
while running do
    -- Handle events
    local events = sdl.poll_events()
    for i, event in ipairs(events) do
        if event.type == sdl.constants.SDL_EVENT_QUIT then
            running = false
        elseif event.type == sdl.constants.SDL_EVENT_WINDOW_RESIZED then
            gl.viewport(0, 0, event.width, event.height)
        end
    end

    -- Render
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear()

    gl.use_program(shaderProgram)
    gl.bind_vertex_array(vao)
    gl.draw_arrays(gl.constants.TRIANGLES, 0, 3)

    gl.swap_buffers()
end

-- Cleanup
gl.destroy()
sdl.quit()