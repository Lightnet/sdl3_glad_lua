-- main.lua (or sdl_gl_image05.lua)

local sdl = require("module_sdl")
local gl = require("module_gl")
local stb = require("module_stb")
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

-- Enable blending
gl.enable(gl.BLEND)
gl.blend_func(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)

-- Load image
local image_data, width, height, channels, err = stb.load_image("resources/ph16.png")
if not image_data then
    lua_util.log("Failed to load image: " .. err)
    gl.destroy()
    sdl.quit()
    return
end
lua_util.log("Image loaded: " .. width .. "x" .. height .. ", channels: " .. channels)

-- Create and set up image texture
local image_texture = gl.gen_textures()
gl.bind_texture(gl.TEXTURE_2D, image_texture)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
local format = channels == 4 and gl.RGBA or gl.RGB
gl.tex_image_2d(gl.TEXTURE_2D, 0, format, width, height, 0, format, gl.UNSIGNED_BYTE, image_data)
stb.free_image(image_data)

-- Load font
local font_data, font_size, err = stb.load_font("resources/Kenney Mini.ttf")
if not font_data then
    lua_util.log("Failed to load font: " .. err)
    gl.destroy()
    sdl.quit()
    return
end
lua_util.log("Font loaded, size: " .. font_size)

-- Render text to bitmap (increased bitmap size to avoid clipping)
local text_bitmap, text_width, text_height, text_channels, err = stb.render_text(font_data, 24, "Hello, World!", 512, 512)
if not text_bitmap then
    lua_util.log("Failed to render text: " .. err)
    gl.destroy()
    sdl.quit()
    return
end
lua_util.log("Text rendered: " .. text_width .. "x" .. text_height .. ", channels: " .. text_channels)

-- Create and set up text texture
local text_texture = gl.gen_textures()
gl.bind_texture(gl.TEXTURE_2D, text_texture)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)
print("gl.TEXTURE_2D")
print(gl.TEXTURE_2D)
print(gl.ALPHA)
gl.tex_image_2d(gl.TEXTURE_2D, 0, gl.ALPHA, text_width, text_height, 0, gl.ALPHA, gl.UNSIGNED_BYTE, text_bitmap)
stb.free_bitmap(text_bitmap)

-- Vertex Shader
local vertexShaderSource = [[
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
]]

-- Fragment Shader
local fragmentShaderSource = [[
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
uniform float isText;
void main() {
    if (isText > 0.5) {
        float alpha = texture(texture1, TexCoord).r;
        FragColor = vec4(1.0, 1.0, 1.0, alpha); // White text
    } else {
        FragColor = texture(texture1, TexCoord); // Image texture
    }
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

-- Get uniform locations
gl.use_program(shaderProgram)
local projection_loc = gl.get_uniform_location(shaderProgram, "projection")
if projection_loc < 0 then
    lua_util.log("Failed to get projection uniform location")
    gl.destroy()
    sdl.quit()
    return
end
local texture_loc = gl.get_uniform_location(shaderProgram, "texture1")
if texture_loc < 0 then
    lua_util.log("Failed to get texture1 uniform location")
    gl.destroy()
    sdl.quit()
    return
end
local isText_loc = gl.get_uniform_location(shaderProgram, "isText")
if isText_loc < 0 then
    lua_util.log("Failed to get isText uniform location")
    gl.destroy()
    sdl.quit()
    return
end

-- Vertex data for image quad (128x128 centered)
local half_width = 128 / 2
local half_height = 128 / 2
local center_x = 800 / 2
local center_y = 600 / 2
local image_vertices = {
    center_x - half_width, center_y - half_height, 0.0, 0.0, -- Bottom-left
    center_x + half_width, center_y - half_height, 1.0, 0.0, -- Bottom-right
    center_x + half_width, center_y + half_height, 1.0, 1.0, -- Top-right
    center_x - half_width, center_y + half_height, 0.0, 1.0  -- Top-left
}
local indices = {
    0, 1, 2, -- First triangle
    2, 3, 0  -- Second triangle
}

-- Vertex data for text quad (below image)
local text_half_width = text_width / 2
local text_half_height = text_height / 2
local text_center_y = center_y - half_height - text_half_height - 50 -- Increased gap
local text_vertices = {
    center_x - text_half_width, text_center_y - text_half_height, 0.0, 0.0, -- Bottom-left
    center_x + text_half_width, text_center_y - text_half_height, 1.0, 0.0, -- Bottom-right
    center_x + text_half_width, text_center_y + text_half_height, 1.0, 1.0, -- Top-right
    center_x - text_half_width, text_center_y + text_half_height, 0.0, 1.0  -- Top-left
}

-- Convert vertices to binary strings
local image_vertexData = ""
for _, v in ipairs(image_vertices) do
    image_vertexData = image_vertexData .. string.pack("f", v)
end
local text_vertexData = ""
for _, v in ipairs(text_vertices) do
    text_vertexData = text_vertexData .. string.pack("f", v)
end
local indexData = ""
for _, i in ipairs(indices) do
    indexData = indexData .. string.pack("I", i)
end

-- Set up VAO, VBO, EBO for image
local image_vao = gl.gen_vertex_arrays()
gl.bind_vertex_array(image_vao)
local image_vbo = gl.gen_buffers()
gl.bind_buffer(gl.ARRAY_BUFFER, image_vbo)
gl.buffer_data(gl.ARRAY_BUFFER, image_vertexData, #image_vertexData, gl.STATIC_DRAW)
local ebo = gl.gen_buffers()
gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER, ebo)
gl.buffer_data(gl.ELEMENT_ARRAY_BUFFER, indexData, #indexData, gl.STATIC_DRAW)
gl.vertex_attrib_pointer(0, 2, gl.FLOAT, false, 4 * 4, 0)
gl.enable_vertex_attrib_array(0)
gl.vertex_attrib_pointer(1, 2, gl.FLOAT, false, 4 * 4, 2 * 4)
gl.enable_vertex_attrib_array(1)

-- Set up VAO, VBO for text
local text_vao = gl.gen_vertex_arrays()
gl.bind_vertex_array(text_vao)
local text_vbo = gl.gen_buffers()
gl.bind_buffer(gl.ARRAY_BUFFER, text_vbo)
gl.buffer_data(gl.ARRAY_BUFFER, text_vertexData, #text_vertexData, gl.STATIC_DRAW)
gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER, ebo)
gl.vertex_attrib_pointer(0, 2, gl.FLOAT, false, 4 * 4, 0)
gl.enable_vertex_attrib_array(0)
gl.vertex_attrib_pointer(1, 2, gl.FLOAT, false, 4 * 4, 2 * 4)
gl.enable_vertex_attrib_array(1)

-- Set initial viewport
gl.viewport(0, 0, 800, 600)

-- Create orthographic projection
local projection = cglm.ortho(0, 800, 0, 600, -1, 1)
for col = 0, 3 do
    for row = 0, 3 do
        local value = projection:get(row, col)
        lua_util.log(string.format("projection[%d][%d] = %f", row, col, value))
    end
end

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
            projection = cglm.ortho(0, event.width, 0, event.height, -1, 1)
            for col = 0, 3 do
                for row = 0, 3 do
                    local value = projection:get(row, col)
                    lua_util.log(string.format("projection[%d][%d] = %f", row, col, value))
                end
            end
        end
    end

    -- Render
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear()

    gl.use_program(shaderProgram)
    gl.uniform_matrix4fv(projection_loc, 1, false, projection)

    -- Draw image quad
    gl.uniform1f(isText_loc, 0.0)
    gl.uniform1i(texture_loc, 0)
    gl.active_texture(gl.TEXTURE0)
    gl.bind_texture(gl.TEXTURE_2D, image_texture)
    gl.bind_vertex_array(image_vao)
    gl.draw_elements(gl.TRIANGLES, 6, gl.UNSIGNED_INT, 0)
    lua_util.log("Drew image quad")

    -- Draw text quad
    gl.uniform1f(isText_loc, 1.0)
    gl.uniform1i(texture_loc, 1)
    gl.active_texture(gl.TEXTURE1)
    gl.bind_texture(gl.TEXTURE_2D, text_texture)
    gl.bind_vertex_array(text_vao)
    gl.draw_elements(gl.TRIANGLES, 6, gl.UNSIGNED_INT, 0)
    lua_util.log("Drew text quad")

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
gl.delete_textures({image_texture, text_texture})
gl.delete_buffers({image_vbo, text_vbo, ebo})
gl.delete_vertex_arrays({image_vao, text_vao})
gl.destroy()
sdl.quit()