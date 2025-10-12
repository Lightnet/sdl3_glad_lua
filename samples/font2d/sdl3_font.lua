-- sdl3_font.lua
-- local bit = require("bit")
local sdl = require("module_sdl")
local gl = require("module_gl")
local stb = require("module_stb")
local lua_util = require("lua_util")

print("bit" .. tostring(bit))
print("bit32" .. tostring(bit32))

-- Initialize SDL
local success, err = sdl.init(sdl.INIT_VIDEO + sdl.INIT_EVENTS)
if not success then
    lua_util.log("Failed to initialize SDL: " .. err)
    sdl.quit()
    return
end

-- Get display scale
local primary_display = sdl.get_primary_display()
local main_scale, err = sdl.get_display_content_scale(primary_display)
if not main_scale then
    lua_util.log("Failed to get display content scale: " .. err)
    sdl.quit()
    return
end
print("main_scale: " .. main_scale)

-- Create window with OpenGL and resizable flags
local window, err = sdl.init_window("sdl3 font 01", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
if not window then
    lua_util.log("Failed to create window: " .. err)
    sdl.quit()
    return
end
-- sdl.set_window_position(window, sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED)
-- sdl.show_window(window)

print("window: " .. tostring(window))
-- Initialize OpenGL
local success, gl_context, err = gl.init(window)
if not success then
    lua_util.log("Failed to initialize OpenGL: " .. err)
    sdl.quit()
    return
end
print("Loaded OpenGL version " .. gl.get_string(gl.VERSION))

-- Load and bake font
local font_size = 32.0 * main_scale
local bitmap_w = 512
local bitmap_h = 512
local font = stb.bake_font("resources/Kenney Mini.ttf", font_size, bitmap_w, bitmap_h)
if not font then
    lua_util.log("Failed to bake font")
    gl.destroy_context(gl_context)
    sdl.quit()
    return
end

local bitmap_str, bitmap_w, bitmap_h = font:get_bitmap()

-- Create texture
local font_tex = gl.gen_textures(1)[1]
gl.bind_texture(gl.TEXTURE_2D, font_tex)
gl.tex_image_2d(gl.TEXTURE_2D, 0, gl.RED, bitmap_w, bitmap_h, 0, gl.RED, gl.UNSIGNED_BYTE, bitmap_str)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)

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
local vs_status = gl.get_shader_iv(vs, gl.COMPILE_STATUS)
if vs_status == 0 then
    local log = gl.get_shader_info_log(vs)
    lua_util.log("Vertex shader compilation failed: " .. log)
    -- Cleanup and return
    gl.destroy_context(gl_context)
    sdl.quit()
    return
end

local fs = gl.create_shader(gl.FRAGMENT_SHADER)
gl.shader_source(fs, fs_src)
gl.compile_shader(fs)
local fs_status = gl.get_shader_iv(fs, gl.COMPILE_STATUS)
if fs_status == 0 then
    local log = gl.get_shader_info_log(fs)
    lua_util.log("Fragment shader compilation failed: " .. log)
    -- Cleanup and return
    gl.destroy_context(gl_context)
    sdl.quit()
    return
end

local program = gl.create_program()
gl.attach_shader(program, vs)
gl.attach_shader(program, fs)
gl.link_program(program)
local link_status = gl.get_program_iv(program, gl.LINK_STATUS)
if link_status == 0 then
    local log = gl.get_program_info_log(program)
    lua_util.log("Program linking failed: " .. log)
    -- Cleanup and return
    gl.destroy_context(gl_context)
    sdl.quit()
    return
end

gl.delete_shader(vs)
gl.delete_shader(fs)

-- VAO and VBO
local vao = gl.gen_vertex_arrays(1)[1]
local vbo = gl.gen_buffers(1)[1]
gl.bind_vertex_array(vao)
gl.bind_buffer(gl.ARRAY_BUFFER, vbo)
gl.enable_vertex_attrib_array(0)
gl.vertex_attrib_pointer(0, 2, gl.FLOAT, gl.FALSE, 16, 0)
gl.enable_vertex_attrib_array(1)
gl.vertex_attrib_pointer(1, 2, gl.FLOAT, gl.FALSE, 16, 8)
gl.bind_vertex_array(0)

gl.enable(gl.BLEND)
gl.blend_func(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)

local clear_color = {0.45, 0.55, 0.60, 1.00}
local text = "Hello World!,. Glad, 2.0.8',"
local done = false

while not done do
    local events = sdl.poll_events()
    for _, event in ipairs(events) do
        -- if event.type == sdl.EVENT_QUIT or (event.type == sdl.EVENT_WINDOW_CLOSE_REQUESTED and event.window.windowID == sdl.get_window_id(window)) then
        if event.type == sdl.EVENT_QUIT or (event.type == sdl.EVENT_WINDOW_CLOSE_REQUESTED) then
            done = true
        end
    end

    local flags = sdl.get_window_flags(window)
    
    -- if bit32.band(flags, sdl.WINDOW_MINIMIZED) ~= 0 then
    --     sdl.delay(10)
    --     goto continue
    -- end

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
        local ch = text:byte(i)
        if ch >= 32 and ch < 128 then
            local q, new_x, new_y = font:get_baked_quad(ch, x, y)
            x = new_x
            y = new_y

            local nx0 = 2.0 * q.x0 / ww - 1.0
            local ny0 = 1.0 - 2.0 * q.y0 / hh
            local nx1 = 2.0 * q.x1 / ww - 1.0
            local ny1 = 1.0 - 2.0 * q.y1 / hh

            -- Triangle 1
            table.insert(vertices, nx0)
            table.insert(vertices, ny0)
            table.insert(vertices, q.s0)
            table.insert(vertices, q.t0)
            table.insert(vertices, nx1)
            table.insert(vertices, ny0)
            table.insert(vertices, q.s1)
            table.insert(vertices, q.t0)
            table.insert(vertices, nx1)
            table.insert(vertices, ny1)
            table.insert(vertices, q.s1)
            table.insert(vertices, q.t1)

            -- Triangle 2
            table.insert(vertices, nx0)
            table.insert(vertices, ny0)
            table.insert(vertices, q.s0)
            table.insert(vertices, q.t0)
            table.insert(vertices, nx1)
            table.insert(vertices, ny1)
            table.insert(vertices, q.s1)
            table.insert(vertices, q.t1)
            table.insert(vertices, nx0)
            table.insert(vertices, ny1)
            table.insert(vertices, q.s0)
            table.insert(vertices, q.t1)
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
    gl.buffer_data(gl.ARRAY_BUFFER, vertices, gl.DYNAMIC_DRAW)
    gl.draw_arrays(gl.TRIANGLES, 0, #vertices / 4)
    gl.bind_vertex_array(0)

    sdl.gl_swap_window(window)

    ::continue::
end

-- Cleanup
gl.delete_program(program)
gl.delete_textures({font_tex})
gl.delete_buffers({vbo})
gl.delete_vertex_arrays({vao})
gl.destroy_context(gl_context)
sdl.quit()