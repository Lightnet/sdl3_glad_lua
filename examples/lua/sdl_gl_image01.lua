-- sdl_gl_image01.lua

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

-- Create window with OpenGL and resizable flags
local window, err = sdl.init_window("sdl3 image 01", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
if not window then
    lua_util.log("Failed to create window: " .. err)
    sdl.quit()
    return
end

-- Initialize OpenGL
local success, gl_context, err = gl.init(window)
if not success then
    lua_util.log("Failed to initialize OpenGL: " .. err)
    gl.destroy()
    sdl.quit()
    return
end
print("success: " .. tostring(success))
print("gl_context: " .. tostring(gl_context))

-- Load image
local img, err = stb.load_image("resources/ph16.png", stb.RGBA)
if not img then
    lua_util.log("Failed to load image: " .. err)
    gl.destroy()
    sdl.quit()
    return
end
local image_data = img:get_data()
local width = img:get_width()
local height = img:get_height()
local channels = img:get_channels()
print("Image data size: " .. (width * height * channels) .. ", expected: " .. (width * height * channels))

-- Create and set up texture
local texture = gl.gen_textures()
gl.active_texture(gl.TEXTURE0)
gl.bind_texture(gl.TEXTURE_2D, texture)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
local format = channels == 4 and gl.RGBA or gl.RGB
local tex_success, tex_err = gl.tex_image_2d(gl.TEXTURE_2D, 0, format, width, height, 0, format, gl.UNSIGNED_BYTE, image_data)
if not tex_success then
    lua_util.log("OpenGL error after glTexImage2D: " .. tex_err)
    img:free()
    gl.destroy()
    sdl.quit()
    return
end
img:free() -- Free image data after uploading to GPU

-- Enable blending for alpha (if RGBA)
if channels == 4 then
    gl.enable(gl.BLEND)
    gl.blend_func(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
end

-- Vertex Shader (includes texture coordinates)
local vertexShaderSource = [[
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
]]

-- Fragment Shader (samples texture)
local fragmentShaderSource = [[
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
void main() {
    FragColor = texture(texture1, TexCoord);
}
]]

-- Create and compile vertex shader
local vertexShader = gl.create_shader(gl.VERTEX_SHADER)
gl.shader_source(vertexShader, vertexShaderSource)
local success, err = gl.compile_shader(vertexShader)
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

-- Set up texture uniform
gl.use_program(shaderProgram)
local texture1_location = gl.get_uniform_location(shaderProgram, "texture1")
if texture1_location < 0 then
    lua_util.log("Failed to get uniform location for texture1")
    gl.destroy()
    sdl.quit()
    return
end
gl.uniform1i(texture1_location, 0)
print("Texture1 uniform location: " .. texture1_location)

-- Vertex data for a quad (x, y, s, t)
local vertices = {
    -1.0, -1.0, 0.0, 0.0, -- Bottom-left
     1.0, -1.0, 1.0, 0.0, -- Bottom-right
     1.0,  1.0, 1.0, 1.0, -- Top-right
    -1.0,  1.0, 0.0, 1.0  -- Top-left
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

-- Set vertex attributes (position and texture coordinates)
gl.vertex_attrib_pointer(0, 2, gl.FLOAT, false, 4 * 4, 0)
gl.enable_vertex_attrib_array(0)
gl.vertex_attrib_pointer(1, 2, gl.FLOAT, false, 4 * 4, 2 * 4)
gl.enable_vertex_attrib_array(1)

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

    gl.use_program(shaderProgram)
    gl.active_texture(gl.TEXTURE0)
    gl.bind_texture(gl.TEXTURE_2D, texture)
    print("Bound texture in loop: " .. texture)
    
    local bound_texture = gl.get_integer(gl.TEXTURE_BINDING_2D)
    print("Current bound texture: " .. bound_texture)
    gl.bind_vertex_array(vao)
    gl.draw_elements(gl.TRIANGLES, 6, gl.UNSIGNED_INT, 0)
    local err = gl.get_error()
    print("err:" .. tostring(err))
    print("gl.NO_ERROR:" .. tostring(gl.NO_ERROR))
    if err ~= gl.NO_ERROR then
        lua_util.log("OpenGL error after glDrawElements: " .. err)
    end

    -- Swap window
    sdl.gl_swap_window(window)
end

-- Cleanup
gl.delete_vertex_arrays({vao})
gl.delete_buffers({vbo, ebo})
gl.delete_textures({texture})
gl.delete_shader(vertexShader)
gl.delete_shader(fragmentShader)
gl.delete_program(shaderProgram)
gl.destroy()
sdl.quit()