local sdl = require("module_sdl")
local gl = require("module_gl")
local cglm = require("module_cglm")
local lua_util = require("lua_util")

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

-- Vertex Shader with MVP matrix
local vertex_shader_source = [[
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 mvp;
void main() {
    gl_Position = mvp * vec4(aPos, 1.0);
}
]]

-- Fragment Shader
local fragment_shader_source = [[
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0); // Orange color
}
]]

-- Create and compile vertex shader
local vertex_shader = gl.create_shader(gl.VERTEX_SHADER)
gl.shader_source(vertex_shader, vertex_shader_source)
success, err = gl.compile_shader(vertex_shader)
if not success then
    lua_util.log("Vertex shader compilation failed: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Create and compile fragment shader
local fragment_shader = gl.create_shader(gl.FRAGMENT_SHADER)
gl.shader_source(fragment_shader, fragment_shader_source)
success, err = gl.compile_shader(fragment_shader)
if not success then
    lua_util.log("Fragment shader compilation failed: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Create shader program
local shader_program = gl.create_program()
gl.attach_shader(shader_program, vertex_shader)
gl.attach_shader(shader_program, fragment_shader)
success, err = gl.link_program(shader_program)
if not success then
    lua_util.log("Shader program linking failed: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Cube vertex data (8 vertices: x, y, z)
local vertices = {
    -- Front face
    -0.5, -0.5,  0.5,  -- Bottom-left-front
     0.5, -0.5,  0.5,  -- Bottom-right-front
     0.5,  0.5,  0.5,  -- Top-right-front
    -0.5,  0.5,  0.5,  -- Top-left-front
    -- Back face
    -0.5, -0.5, -0.5,  -- Bottom-left-back
     0.5, -0.5, -0.5,  -- Bottom-right-back
     0.5,  0.5, -0.5,  -- Top-right-back
    -0.5,  0.5, -0.5   -- Top-left-back
}

-- Cube indices (36 indices for 12 triangles)
local indices = {
    -- Front face
    0, 1, 2,  0, 2, 3,
    -- Right face
    1, 5, 6,  1, 6, 2,
    -- Back face
    5, 4, 7,  5, 7, 6,
    -- Left face
    4, 0, 3,  4, 3, 7,
    -- Top face
    3, 2, 6,  3, 6, 7,
    -- Bottom face
    0, 4, 5,  0, 5, 1
}

-- Convert vertices to binary string
local vertex_data = ""
for _, v in ipairs(vertices) do
    vertex_data = vertex_data .. string.pack("f", v)
end

-- Convert indices to binary string (unsigned int)
local index_data = ""
for _, i in ipairs(indices) do
    index_data = index_data .. string.pack("I", i)
end

-- Set up VAO, VBO, and EBO
local vao = gl.gen_vertex_arrays()
gl.bind_vertex_array(vao)

local vbo = gl.gen_buffers()
gl.bind_buffer(gl.ARRAY_BUFFER, vbo)
gl.buffer_data(gl.ARRAY_BUFFER, vertex_data, #vertex_data, gl.STATIC_DRAW)

local ebo = gl.gen_buffers()
gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER, ebo)
gl.buffer_data(gl.ELEMENT_ARRAY_BUFFER, index_data, #index_data, gl.STATIC_DRAW)

-- Set vertex attributes
gl.vertex_attrib_pointer(0, 3, gl.FLOAT, false, 3 * 4, 0)
gl.enable_vertex_attrib_array(0)

-- Enable depth testing
gl.enable(gl.DEPTH_TEST)

-- Set initial viewport
gl.viewport(0, 0, 800, 600)

-- Set up matrices
local projection = cglm.perspective(math.rad(45), 800 / 600, 0.1, 100.0)
local view = cglm.mat4()
view = cglm.translate(view, cglm.vec3(0, 0, -3)) -- Move camera back
local model = cglm.mat4_identity()

-- Animation variables
local angle = 0

-- Main loop
local running = true
while running do
    -- Handle events
    local events = sdl.poll_events()
    for _, event in ipairs(events) do
        if event.type == sdl.SDL_EVENT_QUIT then
            running = false
        elseif event.type == sdl.SDL_EVENT_WINDOW_RESIZED then
            gl.viewport(0, 0, event.width, event.height)
            projection = cglm.perspective(math.rad(45), event.width / event.height, 0.1, 100.0)
        end
    end

    -- Update rotation
    angle = angle + 0.01
    print("angle: ".. angle)
    model = cglm.mat4_identity()
    model = cglm.rotate(model, angle, cglm.vec3(1, 1, 0)) -- Rotate around (1,1,0) axis

    -- Compute MVP matrix
    local mvp = cglm.mat4_mul(projection, view)
    mvp = cglm.mat4_mul(mvp, model)

    -- Render
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT + gl.DEPTH_BUFFER_BIT)

    gl.use_program(shader_program)
    local mvp_loc = gl.get_uniform_location(shader_program, "mvp")
    gl.uniform_matrix4fv(mvp_loc, 1, gl.FALSE, mvp)

    gl.bind_vertex_array(vao)
    gl.draw_elements(gl.TRIANGLES, #indices, gl.UNSIGNED_INT, 0)

    sdl.gl_swap_window(window)
end

-- Cleanup
gl.delete_vertex_arrays({vao})
gl.delete_buffers({vbo})
gl.delete_buffers({ebo})
gl.delete_shader(vertex_shader)
gl.delete_shader(fragment_shader)
gl.delete_program(shader_program)
gl.destroy()
sdl.quit()