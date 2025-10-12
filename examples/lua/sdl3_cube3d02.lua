-- modules
local sdl = require("module_sdl")
local gl = require("module_gl")
local cglm = require("module_cglm")
local lua_util = require("lua_util")

-- Initialize SDL video subsystem
local success, err = sdl.init(sdl.INIT_VIDEO)
if not success then
    lua_util.log("Failed to initialize SDL: " .. err)
    sdl.quit()
    return
end

-- Create window with OpenGL and resizable flags
local window, err = sdl.init_window("sdl3 cube3d", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
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

-- Vertex Shader with MVP matrix and vertex color
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

-- Fragment Shader
local fragment_shader_source = [[
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vertexColor, 1.0);
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

-- Cube vertex data (8 vertices: x, y, z, r, g, b)
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

-- Cube indices (36 indices for 12 triangles, counterclockwise winding)
local indices = {
    -- Front face
    0, 1, 2,  2, 3, 0,
    -- Right face
    1, 5, 6,  6, 2, 1,
    -- Back face
    5, 4, 7,  7, 6, 5,
    -- Left face
    4, 0, 3,  3, 7, 4,
    -- Top face
    3, 2, 6,  6, 7, 3,
    -- Bottom face
    4, 5, 1,  1, 0, 4
}

-- Convert vertices to binary string (position + color)
local vertex_data = ""
for _, v in ipairs(vertices) do
    vertex_data = vertex_data .. string.pack("f", v)
end
print("vertex_data:" .. vertex_data)

-- Convert indices to binary string (unsigned int)
local index_data = ""
for _, i in ipairs(indices) do
    index_data = index_data .. string.pack("I", i)
end
print("index_data:" .. index_data)


-- Set up VAO, VBO, and EBO
local vao = gl.gen_vertex_arrays()
gl.bind_vertex_array(vao)

local vbo = gl.gen_buffers()
gl.bind_buffer(gl.ARRAY_BUFFER, vbo)
gl.buffer_data(gl.ARRAY_BUFFER, vertex_data, #vertex_data, gl.STATIC_DRAW)

local ebo = gl.gen_buffers()
gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER, ebo)
gl.buffer_data(gl.ELEMENT_ARRAY_BUFFER, index_data, #index_data, gl.STATIC_DRAW)

-- Set vertex attributes (position and color)
gl.vertex_attrib_pointer(0, 3, gl.FLOAT, false, 6 * 4, 0) -- Position (3 floats)
gl.enable_vertex_attrib_array(0)
gl.vertex_attrib_pointer(1, 3, gl.FLOAT, false, 6 * 4, 3 * 4) -- Color (3 floats, offset by 3 floats)
gl.enable_vertex_attrib_array(1)

-- Enable depth testing
gl.enable(gl.DEPTH_TEST)

-- Disable face culling to ensure all faces are visible
gl.cull_face(gl.FALSE)

-- Set initial viewport
gl.viewport(0, 0, 800, 600)

-- Set up matrices
local projection = cglm.perspective(math.rad(45), 800 / 600, 0.1, 100.0)
local view = cglm.mat4()
view = cglm.translate(view, cglm.vec3(0, 0, -3)) -- Move camera back
local model = cglm.mat4_identity()

-- Animation variables
local angle_y = 0
local angle_z = 0

-- Main loop
local running = true
while running do
    -- Handle events
    local events = sdl.poll_events()
    for _, event in ipairs(events) do
        if event.type == sdl.EVENT_QUIT then
            running = false
        elseif event.type == sdl.EVENT_WINDOW_RESIZED then
            gl.viewport(0, 0, event.width, event.height)
            projection = cglm.perspective(math.rad(45), event.width / event.height, 0.1, 100.0)
        end
    end

    -- Update rotation (mimic C code's Y and Z rotations)
    angle_y = angle_y + 0.01
    angle_z = angle_z + 0.01
    -- print("angle_y: " .. angle_y .. ", angle_z: " .. angle_z)
    model = cglm.mat4_identity()
    model = cglm.rotate(model, angle_y, cglm.vec3(0, 1, 0)) -- Rotate around Y-axis
    model = cglm.rotate(model, angle_z, cglm.vec3(0, 0, 1)) -- Rotate around Z-axis

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

    -- Check for OpenGL errors
    local err_code = gl.get_error()
    if err_code ~= 0 then
        lua_util.log("OpenGL error: " .. err_code)
    end

    -- swap window
    sdl.gl_swap_window(window)
end

-- Cleanup
gl.delete_vertex_arrays({vao})
gl.delete_buffers({vbo, ebo})
gl.delete_shader(vertex_shader)
gl.delete_shader(fragment_shader)
gl.delete_program(shader_program)
gl.destroy()
sdl.quit()