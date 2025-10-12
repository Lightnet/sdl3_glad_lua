-- sdl3_font.lua

local sdl = require("module_sdl")
local gl = require("module_gl")
local stb = require("module_stb")
local lua_util = require("lua_util")

-- Initialize SDL video subsystem
local success, err = sdl.init(sdl.INIT_VIDEO)
if not success then
    lua_util.log("Failed to initialize SDL: " .. err)
    sdl.quit()
    return
end

-- Get display scale
local display_id = sdl.get_primary_display()
local main_scale, err = sdl.get_display_content_scale(display_id)
if not main_scale then
    lua_util.log("Failed to get display content scale: " .. err)
    sdl.quit()
    return
end
print("main_scale: " .. main_scale)

-- Create window
local window_flags = sdl.WINDOW_RESIZABLE + sdl.WINDOW_HIDDEN + sdl.WINDOW_HIGH_PIXEL_DENSITY + sdl.WINDOW_OPENGL
local window, err = sdl.init_window("SDL3 OpenGL Font Example", 1280 * main_scale, 720 * main_scale, window_flags)
if not window then
    lua_util.log("Failed to create window: " .. err)
    sdl.quit()
    return
end
sdl.set_window_position(window, sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED)
sdl.show_window(window)

-- Initialize OpenGL
local success, gl_context, err = gl.init(window)
if not success then
    lua_util.log("Failed to initialize OpenGL: " .. err)
    sdl.quit()
    return
end

-- Load font
local font_path = "resources/Kenney Mini.ttf"
local font_file = io.open(font_path, "rb")
if not font_file then
    lua_util.log("Error: Failed to open font file 'Kenney Mini.ttf'")
    gl.destroy_context(gl_context)
    sdl.quit()
    return
end
local ttf_buffer = font_file:read("*all")
font_file:close()

local bitmap_w = 512
local bitmap_h = 512
local font_size = 32.0 * main_scale
local bitmap, cdata, baked_num = stb.bake_font_bitmap(ttf_buffer, font_size, bitmap_w, bitmap_h, 32, 96)
if not bitmap then
    lua_util.log("Failed to bake font bitmap")
    gl.destroy_context(gl_context)
    sdl.quit()
    return
end

-- Create OpenGL texture
local font_tex = gl.gen_texture()
gl.bind_texture(gl.TEXTURE_2D, font_tex)
gl.tex_image_2d(gl.TEXTURE_2D, 0, gl.RED, bitmap_w, bitmap_h, 0, gl.RED, gl.UNSIGNED_BYTE, bitmap)
gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)

-- Shaders
local vs_src = [[
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    TexCoord = texCoord;
}
]]

local fs_src = [[
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D textTexture;
uniform vec4 textColor;
void main() {
    float alpha = texture(textTexture, TexCoord).r;
    FragColor = vec4(textColor.rgb, alpha * textColor.a);
}
]]

local vs = gl.create_shader(gl.VERTEX_SHADER)
gl.shader_source(vs, vs_src)
gl.compile_shader(vs)
local success = gl.get_shader_iv(vs, gl.COMPILE_STATUS)
if success == 0 then
    local log = gl.get_shader_info_log(vs)
    lua_util.log("Vertex shader compilation failed: " .. log)
    gl.destroy_context(gl_context)
    sdl.quit()
    return
end

local fs = gl.create_shader(gl.FRAGMENT_SHADER)
gl.shader_source(fs, fs_src)
gl.compile_shader(fs)
success = gl.get_shader_iv(fs, gl.COMPILE_STATUS)
if success == 0 then
    local log = gl.get_shader_info_log(fs)
    lua_util.log("Fragment shader compilation failed: " .. log)
    gl.destroy_context(gl_context)
    sdl.quit()
    return
end

local program = gl.create_program()
gl.attach_shader(program, vs)
gl.attach_shader(program, fs)
gl.link_program(program)
success = gl.get_program_iv(program, gl.LINK_STATUS)
if success == 0 then
    local log = gl.get_program_info_log(program)
    lua_util.log("Program linking failed: " .. log)
    gl.destroy_context(gl_context)
    sdl.quit()
    return
end

gl.delete_shader(vs)
gl.delete_shader(fs)

-- VAO and VBO
local vao = gl.gen_vertex_array()
local vbo = gl.gen_buffer()
gl.bind_vertex_array(vao)
gl.bind_buffer(gl.ARRAY_BUFFER, vbo)
gl.enable_vertex_attrib_array(0)
gl.vertex_attrib_pointer(0, 2, gl.FLOAT, gl.FALSE, 16, 0)
gl.enable_vertex_attrib_array(1)
gl.vertex_attrib_pointer(1, 2, gl.FLOAT, gl.FALSE, 16, 8)
gl.bind_vertex_array(0)

gl.enable(gl.BLEND)
gl.blend_func(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)

-- Clear color
local clear_color = {0.45, 0.55, 0.60, 1.00}

-- Main loop
local done = false
local text = "Hello World! Glad 2.0.8"
while not done do
    local events = sdl.poll_events()
    for i = 1, #events do
        local event = events[i]
        if event.type == sdl.EVENT_QUIT then
            done = true
        elseif event.type == sdl.EVENT_WINDOW_CLOSE_REQUESTED then
            done = true
        end
    end

    local flags = sdl.get_window_flags(window)
    if flags & sdl.WINDOW_MINIMIZED ~= 0 then
        sdl.delay(10)
        goto continue
    end

    local ww, hh = sdl.get_window_size(window)
    gl.viewport(0, 0, ww, hh)

    gl.clear_color(clear_color[1], clear_color[2], clear_color[3], clear_color[4])
    gl.clear(gl.COLOR_BUFFER_BIT)

    -- Render text
    local x = 25.0
    local y = 150.0
    local vertices = {}
    for i = 1, #text do
        local c = string.byte(text, i)
        if c >= 32 and c < 128 then
            local quad, new_x, new_y = stb.get_baked_quad(cdata, bitmap_w, bitmap_h, c, x, y)
            x = new_x
            y = new_y
            local nx0 = 2.0 * quad.x0 / ww - 1.0
            local ny0 = 1.0 - 2.0 * quad.y0 / hh
            local nx1 = 2.0 * quad.x1 / ww - 1.0
            local ny1 = 1.0 - 2.0 * quad.y1 / hh
            -- Triangle 1
            table.insert(vertices, nx0)
            table.insert(vertices, ny0)
            table.insert(vertices, quad.s0)
            table.insert(vertices, quad.t0)
            table.insert(vertices, nx1)
            table.insert(vertices, ny0)
            table.insert(vertices, quad.s1)
            table.insert(vertices, quad.t0)
            table.insert(vertices, nx1)
            table.insert(vertices, ny1)
            table.insert(vertices, quad.s1)
            table.insert(vertices, quad.t1)
            -- Triangle 2
            table.insert(vertices, nx0)
            table.insert(vertices, ny0)
            table.insert(vertices, quad.s0)
            table.insert(vertices, quad.t0)
            table.insert(vertices, nx1)
            table.insert(vertices, ny1)
            table.insert(vertices, quad.s1)
            table.insert(vertices, quad.t1)
            table.insert(vertices, nx0)
            table.insert(vertices, ny1)
            table.insert(vertices, quad.s0)
            table.insert(vertices, quad.t1)
        end
    end

    gl.use_program(program)
    local tex_loc = gl.get_uniform_location(program, "textTexture")
    gl.uniform_1i(tex_loc, 0)
    local color_loc = gl.get_uniform_location(program, "textColor")
    gl.uniform_4f(color_loc, 1.0, 1.0, 1.0, 1.0)
    gl.active_texture(gl.TEXTURE0)
    gl.bind_texture(gl.TEXTURE_2D, font_tex)

    gl.bind_vertex_array(vao)
    gl.bind_buffer(gl.ARRAY_BUFFER, vbo)
    gl.buffer_data_table(gl.ARRAY_BUFFER, vertices, gl.DYNAMIC_DRAW)
    gl.draw_arrays(gl.TRIANGLES, 0, #vertices / 4)
    gl.bind_vertex_array(0)

    sdl.gl_swap_window(window)

    ::continue::
end

-- Cleanup
gl.delete_program(program)
gl.delete_texture(font_tex)
gl.delete_buffer(vbo)
gl.delete_vertex_array(vao)
gl.destroy_context(gl_context)
sdl.quit()