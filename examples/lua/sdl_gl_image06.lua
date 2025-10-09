-- main.lua (or sdl_gl_image05.lua)

local sdl = require("module_sdl")
local gl = require("module_gl")
local lua_util = require("lua_util")
local cglm = require("module_cglm")

-- Initialize SDL video subsystem
local success, err = sdl.init(sdl.SDL_INIT_VIDEO + sdl.SDL_INIT_EVENTS)
if not success then
    lua_util.log("Failed to initialize SDL: " .. err)
    sdl.quit()
    return
end

-- Create window with OpenGL and resizable flags
success, err = sdl.init_window(800, 600, sdl.SDL_WINDOW_OPENGL + sdl.SDL_WINDOW_RESIZABLE)
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

-- Vertex Shader (with projection)
local vertexShaderSource = [[
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
]]

-- Fragment Shader (solid red color)
local fragmentShaderSource = [[
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red
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

-- Get uniform location for projection
gl.use_program(shaderProgram)
local projection_loc = gl.get_uniform_location(shaderProgram, "projection")
if projection_loc < 0 then
    lua_util.log("Failed to get projection uniform location")
    gl.destroy()
    sdl.quit()
    return
end

-- Vertex data for a 128x128 quad centered in 800x600 world
local half_width = 128 / 2
local half_height = 128 / 2
local center_x = 800 / 2
local center_y = 600 / 2
local vertices = {
    center_x - half_width, center_y - half_height, -- Bottom-left
    center_x + half_width, center_y - half_height, -- Bottom-right
    center_x + half_width, center_y + half_height, -- Top-right
    center_x - half_width, center_y + half_height  -- Top-left
}
local indices = {
    0, 1, 2, -- First triangle
    2, 3, 0  -- Second triangle
}

-- Convert vertices to a binary string
local vertexData = ""
for _, v in ipairs(vertices) do
    vertexData = vertexData .. string.pack("f", v)
end

-- Convert indices to a binary string
local indexData = ""
for _, i in ipairs(indices) do
    indexData = indexData .. string.pack("I", i)
end

-- Set up VAO, VBO, and EBO
local vao = gl.gen_vertex_arrays()
gl.bind_vertex_array(vao)

local vbo = gl.gen_buffers()
gl.bind_buffer(gl.ARRAY_BUFFER, vbo)
gl.buffer_data(gl.ARRAY_BUFFER, vertexData, #vertexData, gl.STATIC_DRAW)

local ebo = gl.gen_buffers()
gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER, ebo)
gl.buffer_data(gl.ELEMENT_ARRAY_BUFFER, indexData, #indexData, gl.STATIC_DRAW)

-- Set vertex attributes (only position)
gl.vertex_attrib_pointer(0, 2, gl.FLOAT, false, 2 * 4, 0)
gl.enable_vertex_attrib_array(0)

-- Set initial viewport
gl.viewport(0, 0, 800, 600)

-- Main loop
local running = true
while running do
    -- Handle events
    local events = sdl.poll_events()
    for i, event in ipairs(events) do
        if event.type == sdl.SDL_EVENT_QUIT then
            running = false
        elseif event.type == sdl.SDL_EVENT_WINDOW_RESIZED then
            lua_util.log("Window resized to " .. event.width .. "x" .. event.height)
            gl.viewport(0, 0, event.width, event.height)
            -- Note: Not updating projection for resize to test static matrix
        end
    end

    -- Render
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear()

    gl.use_program(shaderProgram)
    gl.dummy_uniform_matrix4fv(projection_loc, 1, false)
    lua_util.log("Set dummy projection matrix")
    gl.bind_vertex_array(vao)
    gl.draw_elements(gl.TRIANGLES, 6, gl.UNSIGNED_INT, 0)

    -- Check for OpenGL errors
    local err = gl.get_error()
    if err ~= 0 then
        lua_util.log("OpenGL error: " .. err)
    end

    gl.swap_buffers()
end

-- Cleanup
gl.delete_shader(vertexShader)
gl.delete_shader(fragmentShader)
gl.delete_program(shaderProgram)
gl.destroy()
sdl.quit()