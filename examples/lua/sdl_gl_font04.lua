-- main.lua
-- not align correctly

local sdl = require("module_sdl")
local gl = require("module_gl")
local stb = require("module_stb")
local lua_util = require("lua_util")
local cglm = require("module_cglm")

-- Initialize SDL video subsystem
local success, err = sdl.init(sdl.INIT_VIDEO + sdl.INIT_EVENTS)
if not success then
    lua_util.log("Failed to initialize SDL: " .. err)
    sdl.quit()
    return
end

-- Create window with OpenGL and resizable flags
local window, err = sdl.init_window("sdl3 font 04", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
if not window then
    lua_util.log("Failed to create window: " .. err)
    sdl.quit()
    return
end

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

-- Get font metrics
local ascent, descent, lineGap, err = stb.get_font_vmetrics(font_data)
if not ascent then
    lua_util.log("Failed to get font metrics: " .. err)
    gl.destroy()
    sdl.quit()
    return
end
local font_size = 32
local scale = font_size / ascent
lua_util.log("Font metrics: ascent=" .. ascent .. ", descent=" .. descent .. ", lineGap=" .. lineGap)
lua_util.log("Scaled metrics: ascent=" .. (ascent * scale) .. ", descent=" .. (descent * scale))

-- Bake font
local bitmap, cdata, bitmap_width, bitmap_height, err = stb.bake_font(font_data, font_size, 512, 512)
if not bitmap then
    lua_util.log("Failed to bake font: " .. err)
    gl.destroy()
    sdl.quit()
    return
end
lua_util.log("Font baked: " .. bitmap_width .. "x" .. bitmap_height)

-- Debug bitmap
local success, err = stb.dump_bitmap(bitmap, bitmap_width, bitmap_height)
if not success then
    lua_util.log("Failed to dump bitmap: " .. err)
end

-- Create and set up text texture
local text_texture = gl.gen_textures()
gl.bind_texture(gl.TEXTURE_2D, text_texture)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)
gl.tex_image_2d(gl.TEXTURE_2D, 0, gl.RED, bitmap_width, bitmap_height, 0, gl.RED, gl.UNSIGNED_BYTE, bitmap)
stb.free_bitmap(bitmap)

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
uniform vec4 textColor;
void main() {
    if (isText > 0.5) {
        float alpha = texture(texture1, TexCoord).r;
        FragColor = vec4(textColor.rgb, alpha * textColor.a);
    } else {
        FragColor = texture(texture1, TexCoord);
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
local textColor_loc = gl.get_uniform_location(shaderProgram, "textColor")
if textColor_loc < 0 then
    lua_util.log("Failed to get textColor uniform location")
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

-- Convert image vertices to binary string
local image_vertexData = ""
for _, v in ipairs(image_vertices) do
    image_vertexData = image_vertexData .. string.pack("f", v)
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

-- Set up VAO, VBO for text (dynamic)
local text_vao = gl.gen_vertex_arrays()
gl.bind_vertex_array(text_vao)
local text_vbo = gl.gen_buffers()
gl.bind_buffer(gl.ARRAY_BUFFER, text_vbo)
gl.vertex_attrib_pointer(0, 2, gl.FLOAT, false, 4 * 4, 0)
gl.enable_vertex_attrib_array(0)
gl.vertex_attrib_pointer(1, 2, gl.FLOAT, false, 4 * 4, 2 * 4)
gl.enable_vertex_attrib_array(1)
gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER, ebo)

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
local text = "Hello, World!"
local max_height = ascent * scale -- Maximum glyph height (32 pixels)
while running do
    -- Handle events
    local events = sdl.poll_events()
    for i, event in ipairs(events) do
        if event.type == sdl.EVENT_QUIT then
            running = false
        elseif event.type == sdl.EVENT_WINDOW_RESIZED then
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

    -- Generate text vertices
    local vertices = {}
    local x = 400
    local y = 600 - 128 - 50 -- Baseline position below image (y=422)
    for i = 1, #text do
        local char = string.byte(text, i)
        local x0, y0, x1, y1, s0, t0, s1, t1, x_advance = stb.get_baked_quad(cdata, bitmap_width, bitmap_height, char, x, y)
        if x0 then
            -- Calculate glyph height and offset to align to baseline
            local height = y1 - y0
            local offset = max_height - height -- Move smaller glyphs down
            y0 = y0 + offset
            y1 = y1 + offset
            -- Flip texture coordinates in Y
            local t0_flipped = t1
            local t1_flipped = t0
            -- Triangle 1 (top-left, top-right, bottom-right)
            vertices[#vertices + 1] = x0
            vertices[#vertices + 1] = y1
            vertices[#vertices + 1] = s0
            vertices[#vertices + 1] = t1_flipped
            vertices[#vertices + 1] = x1
            vertices[#vertices + 1] = y1
            vertices[#vertices + 1] = s1
            vertices[#vertices + 1] = t1_flipped
            vertices[#vertices + 1] = x1
            vertices[#vertices + 1] = y0
            vertices[#vertices + 1] = s1
            vertices[#vertices + 1] = t0_flipped
            -- Triangle 2 (top-left, bottom-right, bottom-left)
            vertices[#vertices + 1] = x0
            vertices[#vertices + 1] = y1
            vertices[#vertices + 1] = s0
            vertices[#vertices + 1] = t1_flipped
            vertices[#vertices + 1] = x1
            vertices[#vertices + 1] = y0
            vertices[#vertices + 1] = s1
            vertices[#vertices + 1] = t0_flipped
            vertices[#vertices + 1] = x0
            vertices[#vertices + 1] = y0
            vertices[#vertices + 1] = s0
            vertices[#vertices + 1] = t0_flipped
            x = x + x_advance
            lua_util.log(string.format("Char %d: x0=%.2f, y0=%.2f, x1=%.2f, y1=%.2f, s0=%.2f, t0=%.2f, s1=%.2f, t1=%.2f, x_advance=%.2f, height=%.2f, offset=%.2f", char, x0, y0, x1, y1, s0, t0, s1, t1, x_advance, height, offset))
        else
            lua_util.log("Failed to get quad for char " .. char .. ": " .. (err or "unknown error"))
        end
    end
    local text_vertexData = ""
    for _, v in ipairs(vertices) do
        text_vertexData = text_vertexData .. string.pack("f", v)
    end
    lua_util.log("Text vertices: " .. #vertices)
    lua_util.log("Final text position: x=" .. x .. ", y=" .. y)

    -- Render
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT)

    gl.use_program(shaderProgram)
    gl.uniform_matrix4fv(projection_loc, 1, gl.FALSE, projection)

    -- Draw image quad
    gl.uniform1f(isText_loc, 0.0)
    gl.uniform4f(textColor_loc, 1.0, 1.0, 1.0, 1.0)
    gl.uniform1i(texture_loc, 0)
    gl.active_texture(gl.TEXTURE0)
    gl.bind_texture(gl.TEXTURE_2D, image_texture)
    gl.bind_vertex_array(image_vao)
    gl.draw_elements(gl.TRIANGLES, 6, gl.UNSIGNED_INT, 0)
    lua_util.log("Drew image quad")

    -- Draw text quads
    if #vertices > 0 then
        gl.uniform1f(isText_loc, 1.0)
        gl.uniform4f(textColor_loc, 1.0, 1.0, 1.0, 1.0)
        gl.uniform1i(texture_loc, 1)
        gl.active_texture(gl.TEXTURE1)
        gl.bind_texture(gl.TEXTURE_2D, text_texture)
        gl.bind_vertex_array(text_vao)
        gl.bind_buffer(gl.ARRAY_BUFFER, text_vbo)
        gl.buffer_data(gl.ARRAY_BUFFER, text_vertexData, #text_vertexData, gl.DYNAMIC_DRAW)
        gl.draw_arrays(gl.TRIANGLES, 0, #vertices / 4)
        lua_util.log("Drew text quads")
    end

    -- Check for OpenGL errors
    local err = gl.get_error()
    if err ~= 0 then
        lua_util.log("OpenGL error: " .. err)
    end

    
    -- Swap window
    sdl.gl_swap_window(window)
end

-- Cleanup
gl.delete_shader(vertexShader)
gl.delete_shader(fragmentShader)
gl.delete_program(shaderProgram)
gl.delete_textures({image_texture, text_texture})
gl.delete_buffers({image_vbo, text_vbo, ebo})
gl.delete_vertex_arrays({image_vao, text_vao})
stb.free_cdata(cdata)
gl.destroy()
sdl.quit()