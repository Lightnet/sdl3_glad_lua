-- cube3d.lua
local sdl = require("module_sdl")
local gl = require("module_gl")
local cglm = require("module_cglm")

-- Vertex shader source
local vertex_shader_source = [[
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 vertexColor;
uniform mat4 mvp;
void main() {
    gl_Position = mvp * vec4(aPos, 1.0);
    vertexColor = aColor;
}
]]

-- Fragment shader source
local fragment_shader_source = [[
#version 330 core
out vec4 FragColor;
in vec3 vertexColor;
void main() {
    FragColor = vec4(vertexColor, 1.0);
}
]]

-- Initialize SDL
local ok, err = sdl.init(sdl.SDL_INIT_VIDEO)
if not ok then
    error("SDL init failed: " .. err)
end

-- Set OpenGL attributes
sdl.gl_set_attribute(sdl.GL_CONTEXT_PROFILE_MASK, sdl.GL_CONTEXT_PROFILE_CORE)
sdl.gl_set_attribute(sdl.GL_CONTEXT_MAJOR_VERSION, 3)
sdl.gl_set_attribute(sdl.GL_CONTEXT_MINOR_VERSION, 3)

-- Create window
ok, err = sdl.init_window(800, 600, sdl.SDL_WINDOW_RESIZABLE + sdl.SDL_WINDOW_OPENGL)
if not ok then
    sdl.quit()
    error("Window creation failed: " .. err)
end

-- Initialize OpenGL context
ok, err = gl.init()
if not ok then
    sdl.quit()
    error("GL init failed: " .. err)
end

-- Set viewport
gl.viewport(0, 0, 800, 600)

-- Cube vertices (position and color)
local vertices = {
    -- Front face
    -0.5, -0.5,  0.5,  1.0, 0.0, 0.0,  -- Bottom-left-front (red)
     0.5, -0.5,  0.5,  0.0, 1.0, 0.0,  -- Bottom-right-front (green)
     0.5,  0.5,  0.5,  0.0, 0.0, 1.0,  -- Top-right-front (blue)
    -0.5,  0.5,  0.5,  1.0, 1.0, 0.0,  -- Top-left-front (yellow)
    -- Back face
    -0.5, -0.5, -0.5,  1.0, 0.0, 1.0,  -- Bottom-left-back (magenta)
     0.5, -0.5, -0.5,  0.0, 1.0, 1.0,  -- Bottom-right-back (cyan)
     0.5,  0.5, -0.5,  1.0, 0.5, 0.0,  -- Top-right-back (orange)
    -0.5,  0.5, -0.5,  0.5, 0.5, 0.5   -- Top-left-back (gray)
}

-- Cube indices
local indices = {
    1, 2, 3,  3, 4, 1,  -- Front
    6, 5, 8,  8, 7, 6,  -- Back
    5, 1, 4,  4, 8, 5,  -- Left
    2, 6, 7,  7, 3, 2,  -- Right
    4, 3, 7,  7, 8, 4,  -- Top
    5, 6, 2,  2, 1, 5   -- Bottom
}

-- Compile vertex shader
local vertex_shader = gl.create_shader(gl.VERTEX_SHADER)
gl.shader_source(vertex_shader, vertex_shader_source)
local ok, err = gl.compile_shader(vertex_shader)
if not ok then
    gl.delete_shader(vertex_shader)
    error("Vertex shader compilation failed: " .. err)
end

-- Compile fragment shader
local fragment_shader = gl.create_shader(gl.FRAGMENT_SHADER)
gl.shader_source(fragment_shader, fragment_shader_source)
ok, err = gl.compile_shader(fragment_shader)
if not ok then
    gl.delete_shader(vertex_shader)
    gl.delete_shader(fragment_shader)
    error("Fragment shader compilation failed: " .. err)
end

-- Link shader program
local shader_program = gl.create_program()
gl.attach_shader(shader_program, vertex_shader)
gl.attach_shader(shader_program, fragment_shader)
ok, err = gl.link_program(shader_program)
gl.delete_shader(vertex_shader)
gl.delete_shader(fragment_shader)
if not ok then
    gl.delete_program(shader_program)
    error("Shader program linking failed: " .. err)
end

-- Set up VAO, VBO, EBO
local vaos = gl.gen_vertex_arrays(1)
local vao = vaos[1]
local buffers = gl.gen_buffers(2)
local vbo, ebo = buffers[1], buffers[2]

gl.bind_vertex_array(vao)
gl.bind_buffer(gl.ARRAY_BUFFER, vbo)
gl.buffer_data(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW)
gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER, ebo)
gl.buffer_data_uint(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW)

gl.vertex_attrib_pointer(0, 3, gl.FLOAT, gl.FALSE, 6 * 4, 0)
gl.enable_vertex_attrib_array(0)
gl.vertex_attrib_pointer(1, 3, gl.FLOAT, gl.FALSE, 6 * 4, 3 * 4)
gl.enable_vertex_attrib_array(1)

gl.bind_buffer(gl.ARRAY_BUFFER, 0)
gl.bind_vertex_array(0)

-- Enable depth testing
gl.enable(gl.DEPTH_TEST)

-- Get MVP uniform location
local mvp_loc = gl.get_uniform_location(shader_program, "mvp")
if mvp_loc == -1 then
    error("Failed to get MVP uniform location")
end

-- Main loop
local running = true
local angle = 0.0
local anglez = 0.0
while running do
    -- Poll events
    local events = sdl.poll_events()
    for _, event in ipairs(events) do
        if event.type == sdl.SDL_EVENT_QUIT then
            running = false
        elseif event.type == sdl.SDL_EVENT_WINDOW_RESIZED then
            gl.viewport(0, 0, event.width, event.height)
        end
    end

    -- Update rotation
    angle = angle + 0.01
    anglez = anglez + 0.01

    -- Set up matrices
    local model = cglm.mat4_identity()
    model = cglm.rotate_y(model, angle)
    model = cglm.rotate_z(model, anglez)
    local view = cglm.mat4_identity()
    view = cglm.translate(view, {0.0, 0.0, -3.0})
    local proj = cglm.perspective(cglm.to_radians(45.0), 800.0 / 600.0, 0.1, 100.0)
    local temp = cglm.mat4_mul(view, model)
    local mvp = cglm.mat4_mul(proj, temp)

    -- Clear screen
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT + gl.DEPTH_BUFFER_BIT)

    -- Draw cube
    gl.use_program(shader_program)
    gl.uniform_matrix4fv(mvp_loc, 1, gl.FALSE, mvp)
    gl.bind_vertex_array(vao)
    gl.draw_elements(gl.TRIANGLES, 36, gl.UNSIGNED_INT, 0)
    gl.bind_vertex_array(0)

    -- Swap buffers
    sdl.gl_swap_window()
end

-- Cleanup
gl.delete_vertex_arrays({vao})
gl.delete_buffers({vbo, ebo})
gl.delete_program(shader_program)
gl.quit()
sdl.quit()