-- examples/lua/sdl_gl_font02.lua

local sdl = require("module_sdl")
local gl = require("module_gl")
local stb = require("module_stb")
local lua_util = require("lua_util")
local cglm = require("module_cglm")

-- Test font rendering in C
local font_data, font_size, err = stb.load_font("resources/Kenney Mini.ttf")
if not font_data then
    lua_util.log("Failed to load font: " .. err)
    return
end
lua_util.log("Font loaded, size: " .. font_size)
local success, err = stb.test_render_text(font_data, 32, "Hello, World!", 256, 64)
if not success then
    lua_util.log("Test render text failed: " .. err)
    return
end
lua_util.log("Test render text succeeded")

-- Initialize SDL video subsystem
local success, err = sdl.init(sdl.SDL_INIT_VIDEO + sdl.SDL_INIT_EVENTS)
if not success then
    lua_util.log("Failed to initialize SDL: " .. err)
    sdl.quit()
    return
end

-- Create window
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
lua_util.log("Blending enabled: SRC_ALPHA, ONE_MINUS_SRC_ALPHA")

-- Render text to bitmap
local text_bitmap, text_width, text_height, text_channels, err = stb.render_text(font_data, 32, "Hello, World!", 256, 64)
if not text_bitmap then
    lua_util.log("Failed to render text: " .. (err or "Unknown error"))
    gl.destroy()
    sdl.quit()
    return
end
lua_util.log("Text rendered: " .. text_width .. "x" .. text_height .. ", channels: " .. text_channels)

-- Dump bitmap for debugging
success, err = stb.dump_bitmap(text_bitmap, 256, 64)
if not success then
    lua_util.log("Failed to dump bitmap: " .. err)
end

-- Create and set up text texture
local text_texture = gl.gen_textures()
gl.bind_texture(gl.TEXTURE_2D, text_texture)
lua_util.log("Bound text texture: " .. text_texture)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
gl.tex_image_2d(gl.TEXTURE_2D, 0, gl.RED, text_width, text_height, 0, gl.RED, gl.UNSIGNED_BYTE, text_bitmap)
local err = gl.get_error()
if err ~= 0 then
    lua_util.log("OpenGL error after tex_image_2d: " .. err)
end
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

-- Fragment Shader (Alpha)
local fragmentShaderSource = [[
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
void main() {
    float alpha = texture(texture1, TexCoord).r;
    FragColor = vec4(1.0, 1.0, 1.0, alpha); // White text
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

-- Vertex data for text quad
local center_x = 800 / 2
local text_half_width = text_width / 2
local text_half_height = text_height / 2
local text_center_y = 112
local tex_scale_x = text_width / 256
local tex_scale_y = text_height / 64
local text_vertices = {
    center_x - text_half_width, text_center_y - text_half_height, 0.0, 0.0, -- Bottom-left
    center_x + text_half_width, text_center_y - text_half_height, tex_scale_x, 0.0, -- Bottom-right
    center_x + text_half_width, text_center_y + text_half_height, tex_scale_x, tex_scale_y, -- Top-right
    center_x - text_half_width, text_center_y + text_half_height, 0.0, tex_scale_y  -- Top-left
}
local indices = {
    0, 1, 2, -- First triangle
    2, 3, 0  -- Second triangle
}
lua_util.log("Text quad: width=" .. text_width .. ", height=" .. text_height .. ", center_y=" .. text_center_y)
lua_util.log("Text texture coords: scale_x=" .. tex_scale_x .. ", scale_y=" .. tex_scale_y)

-- Convert vertices to binary strings
local text_vertexData = ""
for _, v in ipairs(text_vertices) do
    text_vertexData = text_vertexData .. string.pack("f", v)
end
local indexData = ""
for _, i in ipairs(indices) do
    indexData = indexData .. string.pack("I", i)
end

-- Set up VAO, VBO, EBO for text
local text_vao = gl.gen_vertex_arrays()
gl.bind_vertex_array(text_vao)
local text_vbo = gl.gen_buffers()
gl.bind_buffer(gl.ARRAY_BUFFER, text_vbo)
gl.buffer_data(gl.ARRAY_BUFFER, text_vertexData, #text_vertexData, gl.STATIC_DRAW)
local ebo = gl.gen_buffers()
gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER, ebo)
gl.buffer_data(gl.ELEMENT_ARRAY_BUFFER, indexData, #indexData, gl.STATIC_DRAW)
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
    gl.uniform1i(texture_loc, 0)
    gl.active_texture(gl.TEXTURE0)
    gl.bind_texture(gl.TEXTURE_2D, text_texture)
    lua_util.log("Bound text texture: " .. text_texture)
    gl.bind_vertex_array(text_vao)
    gl.draw_elements(gl.TRIANGLES, 6, gl.UNSIGNED_INT, 0)
    lua_util.log("Drew text quad")
    local err = gl.get_error()
    if err ~= 0 then
        lua_util.log("OpenGL error after text draw: " .. err)
    end

    gl.swap_buffers()
end

-- Cleanup
gl.delete_shader(vertexShader)
gl.delete_shader(fragmentShader)
gl.delete_program(shaderProgram)
gl.delete_textures({text_texture})
gl.delete_buffers({text_vbo, ebo})
gl.delete_vertex_arrays({text_vao})
gl.destroy()
sdl.quit()