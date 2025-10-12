# OpenGL Lua Module API Documentation 
(module_gl)

This document provides detailed documentation for the Lua interface to the OpenGL module (module_gl.c). The module integrates OpenGL functionality with Lua using SDL3 and GLAD for OpenGL function loading, and cglm for matrix operations. Each function is described with its purpose, parameters, return values, and an example usage.

---

# Functions

## gl.init(window)

Description: Initializes an OpenGL context for the given SDL window, loads OpenGL functions using GLAD, and sets up VSync. Stores the context globally for use in other functions.

Parameters:
- window (lightuserdata): The SDL window (SDL_Window*) to associate with the OpenGL context.

Return:
- success (boolean): true if initialization succeeds, false otherwise.
- context (lightuserdata): The created SDL_GLContext, or nil on failure.
- err_msg (string): Error message if initialization fails, nil otherwise.

Example:

lua
```lua
local sdl = require("sdl")
local gl = require("module_gl")
local window = sdl.create_window("OpenGL Window", 0, 0, 800, 600, sdl.WINDOW_OPENGL)
local gl_context, success, err = gl.init(window)

print("success: " .. tostring(success))
print("gl_context: " .. tostring(gl_context))

if not success then
    print("Error:", err)
else
    print("OpenGL initialized with context:".. tostring(gl_context))
end
```

---

## gl.destroy()

Description: Destroys the current OpenGL context and clears the global context variable.

Parameters: None

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.destroy()
```

---

## gl.get_gl_context()

Description: Retrieves the current OpenGL context.

Parameters: None

Return:
- context (lightuserdata): The current SDL_GLContext, or nil if not initialized.
- err_msg (string): Error message if context is not initialized, nil otherwise.

Example:

lua
```lua
local gl = require("module_gl")
local context, err = gl.get_gl_context()
if not context then
    print("Error:", err)
else
    print("Context:", context)
end
```

---

## gl.clear(mask)

Description: Clears the specified buffers (e.g., color or depth) in the OpenGL context.

Parameters:
- mask (integer): Bitmask of buffers to clear (e.g., gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT).

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.clear(gl.COLOR_BUFFER_BIT + gl.DEPTH_BUFFER_BIT)
```

---

## gl.clear_color(r, g,, b, a)

Description: Sets the clear color for the color buffer.

Parameters:
- r (number): Red component (0.0 to 1.0).
- g (number): Green component (0.0 to 1.0).
- b (number): Blue component (0.0 to 1.0).
- a (number, optional): Alpha component (0.0 to 1.0, defaults to 1.0).

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.clear_color(0.2, 0.3, 0.3, 1.0)
gl.clear(gl.COLOR_BUFFER_BIT)
```

---

## gl.viewport(x, y, w, h)

Description: Sets the viewport rectangle for rendering.

Parameters:
- x (integer): X-coordinate of the lower-left corner.
- y (integer): Y-coordinate of the lower-left corner.
- w (integer): Width of the viewport.
- h (integer): Height of the viewport.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.viewport(0, 0, 800, 600)
```

---

## gl.create_shader(type)

Description: Creates an OpenGL shader object of the specified type.

Parameters:
- type (integer): Shader type (e.g., gl.VERTEX_SHADER, gl.FRAGMENT_SHADER).

Return:
- shader_id (integer): The created shader ID.

Example:

lua
```lua
local gl = require("module_gl")
local vertex_shader = gl.create_shader(gl.VERTEX_SHADER)
```

---

## gl.delete_shader(shader)

Description: Deletes the specified OpenGL shader object.

Parameters:
- shader (integer): The shader ID to delete.

Return:
- nil or (nil, error_message): Returns nil on success, or nil and an error message if the OpenGL context is invalid.

Example:

lua
```lua
local gl = require("module_gl")
local shader = gl.create_shader(gl.VERTEX_SHADER)
gl.delete_shader(shader)
```

---

## gl.shader_source(shader, source)

Description: Sets the source code for a shader.

Parameters:
- shader (integer): The shader ID.
- source (string): The shader source code as a string.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local vertex_shader = gl.create_shader(gl.VERTEX_SHADER)
local vertex_code = [[
#version 330 core
layout(location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
]]
gl.shader_source(vertex_shader, vertex_code)
```

---

## gl.compile_shader(shader)

Description: Compiles the specified shader.

Parameters:
- shader (integer): The shader ID.

Return:
- success (boolean): true if compilation succeeds, false otherwise.
- err_msg (string, optional): Error message if compilation fails.

Example:

lua
```lua
local gl = require("module_gl")
local vertex_code = "#version 330 core\nlayout(location = 0) in vec3 aPos;\nvoid main() { gl_Position = vec4(aPos, 1.0); }\n"
local shader = gl.create_shader(gl.VERTEX_SHADER)
gl.shader_source(shader, vertex_code)
local success, err = gl.compile_shader(shader)
if not success then
    print("Shader compilation failed:", err)
end
```

---

## gl.create_program()

Description: Creates an OpenGL program object.

Parameters: None

Return:
- program_id (integer): The created program ID.

Example:

lua
```lua
local gl = require("module_gl")
local program = gl.create_program()
```

---

## gl.delete_program(program)

Description: Deletes the specified OpenGL program object.

Parameters:
- program (integer): The program ID to delete.

Return:
- nil or (nil, error_message): Returns nil on success, or nil and an error message if the OpenGL context is invalid.

Example:

lua
```lua
local gl = require("module_gl")
local program = gl.create_program()
gl.delete_program(program)
```

---

## gl.attach_shader(program, shader)

Description: Attaches a shader to a program.

Parameters:
- program (integer): The program ID.
- shader (integer): The shader ID.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local program = gl.create_program()
local shader = gl.create_shader(gl.VERTEX_SHADER)
gl.attach_shader(program, shader)
```

---

## gl.link_program(program)

Description: Links the specified program.

Parameters:
- program (integer): The program ID.

Return:
- success (boolean): true if linking succeeds, false otherwise.
- err_msg (string, optional): Error message if linking fails.

Example:

lua
```lua
local gl = require("module_gl")
local program = gl.create_program()
local success, err = gl.link_program(program)
if not success then
    print("Program linking failed:", err)
end
```

---

## gl.use_program(program)

Description: Sets the specified program as the active program.

Parameters:
- program (integer): The program ID.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local program = gl.create_program()
gl.use_program(program)
```

---

## gl.gen_vertex_arrays()

Description: Generates a vertex array object (VAO).

Parameters: None

Return:
- vao_id (integer): The generated VAO ID.

Example:

lua
```lua
local gl = require("module_gl")
local vao = gl.gen_vertex_arrays()
```

---

## gl.bind_vertex_array(vao)

Description: Binds a vertex array object (VAO).

Parameters:
- vao (integer): The VAO ID.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local vao = gl.gen_vertex_arrays()
gl.bind_vertex_array(vao)
```

---

## gl.gen_buffers()

Description: Generates a buffer object (VBO or EBO).

Parameters: None

Return:
- buffer_id (integer): The generated buffer ID.

Example:

lua
```lua
local gl = require("module_gl")
local vbo = gl.gen_buffers()
```

---

## gl.bind_buffer(target, buffer)

Description: Binds a buffer object to the specified target.

Parameters:
- target (integer): The buffer target (e.g., gl.ARRAY_BUFFER, gl.ELEMENT_ARRAY_BUFFER).
- buffer (integer): The buffer ID.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local vbo = gl.gen_buffers()
gl.bind_buffer(gl.ARRAY_BUFFER, vbo)
```

---

## gl.buffer_data(target, data, size, usage)

Description: Allocates and initializes buffer data.

Parameters:
- target (integer): The buffer target (e.g., gl.ARRAY_BUFFER).
- data (string): Raw binary data (e.g., a string of floats).
- size (integer): Size of the data in bytes.
- usage (integer): Buffer usage (e.g., gl.STATIC_DRAW, gl.DYNAMIC_DRAW).

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local vertices = string.pack("fff", 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, -1.0, -1.0, 0.0)
local vbo = gl.gen_buffers()
gl.bind_buffer(gl.ARRAY_BUFFER, vbo)
gl.buffer_data(gl.ARRAY_BUFFER, vertices, #vertices, gl.STATIC_DRAW)
```

---

## gl.vertex_attrib_pointer(index, size, type, normalized, stride, offset)

Description: Specifies the format and location of vertex attribute data.

Parameters:
- index (integer): The attribute index.
- size (integer): Number of components per attribute (1 to 4).
- type (integer): Data type (e.g., gl.FLOAT).
- normalized (boolean): Whether to normalize the data.
- stride (integer): Byte offset between consecutive attributes.
- offset (integer): Byte offset of the first attribute.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.vertex_attrib_pointer(0, 3, gl.FLOAT, false, 12, 0)
gl.enable_vertex_attrib_array(0)
```

---

## gl.enable_vertex_attrib_array(index)

Description: Enables a vertex attribute array.

Parameters:
- index (integer): The attribute index.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.enable_vertex_attrib_array(0)
```

---

## gl.draw_arrays(mode, first, count)

Description: Draws primitives from array data.

Parameters:
- mode (integer): Primitive type (e.g., gl.TRIANGLES).
- first (integer): Starting index in the array.
- count (integer): Number of vertices to draw.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.draw_arrays(gl.TRIANGLES, 0, 3)
```

---

## gl.gen_textures()

Description: Generates a texture object.

Parameters: None

Return:
- texture_id (integer): The generated texture ID.

Example:

lua
```lua
local gl = require("module_gl")
local texture = gl.gen_textures()
```

---

## gl.bind_texture(target, texture)

Description: Binds a texture to the specified target.


Parameters:
- target (integer): Texture target (e.g., gl.TEXTURE_2D).
- texture (integer): The texture ID.

Return: None

Example:

lua
```lua
local gl = requireなどに

("module_gl")
local texture = gl.gen_textures()
gl.bind_texture(gl.TEXTURE_2D, texture)
```

---

## gl.tex_image_2d(target, level, internal_format, width, height, border, format, type, data)

Description: Specifies a 2D texture image.

Parameters:
- target (integer): Texture target (e.g., gl.TEXTURE_2D).
- level (integer): Mipmap level (usually 0).
- internal_format (integer): Internal format (e.g., gl.RGBA).
- width (integer): Texture width.
- height (integer): Texture height.
- border (integer): Border width (usually 0).
- format (integer): Pixel data format (e.g., gl.RGBA).
- type (integer): Pixel data type (e.g., gl.UNSIGNED_BYTE).
- data (lightuserdata): Pixel data or nil for allocation only.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.bind_texture(gl.TEXTURE_2D, texture)
gl.tex_image_2d(gl.TEXTURE_2D, 0, gl.RGBA, 256, 256, 0, gl.RGBA, gl.UNSIGNED_BYTE, nil)
```

---

## gl.tex_parameter_i(target, pname, param)

Description: Sets a texture parameter.

Parameters:
- target (integer): Texture target (e.g., gl.TEXTURE_2D).
- pname (integer): Parameter name (e.g., gl.TEXTURE_MIN_FILTER).
- param (integer): Parameter value (e.g., gl.NEAREST).

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.tex_parameter_i(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
```

---

## gl.draw_elements(mode, count, type, offset)

Description: Draws primitives from element array data.

Parameters:
- mode (integer): Primitive type (e.g., gl.TRIANGLES).
- count (integer): Number of elements to draw.
- type (integer): Data type of indices (e.g., gl.UNSIGNED_INT).
- offset (integer): Byte offset into the index buffer.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.draw_elements(gl.TRIANGLES, 6, gl.UNSIGNED_INT, 0)
```

---

## gl.uniform_matrix4fv(location, count, transpose, matrix)

Description: Sets a 4x4 matrix uniform in the shader.

Parameters:
- location (integer): Uniform location.
- count (integer): Number of matrices.
- transpose (boolean): Whether to transpose the matrix (0 or 1).
- matrix (userdata or string): A cglm mat4 userdata or a 64-byte string of 16 floats.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local cglm = require("cglm")
local matrix = cglm.mat4_identity()
local location = gl.get_uniform_location(program, "uMVP")
gl.uniform_matrix4fv(location, 1, gl.FALSE, matrix)
```

---

## gl.get_uniform_location(program, name)

Description: Gets the location of a uniform variable in a program.

Parameters:
- program (integer): The program ID.
- name (string): The uniform name.

Return:
- location (integer): The uniform location, or -1 if not found.

Example:

lua
```lua
local gl = require("module_gl")
local location = gl.get_uniform_location(program, "uTexture")
```

---

## gl.uniform1i(location, value)

Description: Sets a single integer uniform.

Parameters:
- location (integer): Uniform location.
- value (integer): The integer value.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local location = gl.get_uniform_location(program, "uTexture")
gl.uniform1i(location, 0)
```

---

## gl.uniform1f(location, value)

Description: Sets a single float uniform.

Parameters:
- location (integer Aldo): Uniform location.
- value (number): The float value.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local location = gl.get_uniform_location(program, "uTime")
gl.uniform1f(location, 1.5)
```

---

## gl.uniform4f(location, x, y, z, w)

Description: Sets a vec4 float uniform.

Parameters:
- location (integer): Uniform location.
- x (number): First float component.
- y (number): Second float component.
- z (number): Third float component.
- w (number): Fourth float component.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local location = gl.get_uniform_location(program, "uColor")
gl.uniform4f(location, 1.0, 0.0, 0.0, 1.0)
```

---

## gl.active_texture(texture_unit)

Description: Selects the active texture unit.

Parameters:
- texture_unit (integer): Texture unit (e.g., gl.TEXTURE0).

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.active_texture(gl.TEXTURE0)
```

---

## gl.enable(cap)

Description: Enables an OpenGL capability.

Parameters:
- cap (integer): Capability to enable (e.g., gl.BLEND, gl.DEPTH_TEST).

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.enable(gl.BLEND)
```

---

## gl.disable(cap)

Description: Disables an OpenGL capability.

Parameters:
- cap (integer): Capability to disable (e.g., gl.BLEND, gl.DEPTH_TEST).

Return:
- success (boolean): true if successful, false otherwise.
- err_msg (string, optional): Error message if an OpenGL error occurs.

Example:

lua
```lua
local gl = require("module_gl")
local success, err = gl.disable(gl.BLEND)
if not success then
    print("Error:", err)
end
```

---

## gl.get_error()

Description: Retrieves the current OpenGL error code.

Parameters: None

Return:
- error_code (integer): The OpenGL error code (e.g., gl.NO_ERROR).

Example:

lua
```lua
local gl = require("module_gl")
local err = gl.get_error()
if err ~= gl.NO_ERROR then
    print("OpenGL error:", err)
end
```

---

## gl.blend_func(sfactor, dfactor)

Description: Sets the blending factors for source and destination.

Parameters:
- sfactor (integer): Source blending factor (e.g., gl.SRC_ALPHA).
- dfactor (integer): Destination blending factor (e.g., gl.ONE_MINUS_SRC_ALPHA).

Return: None

Example:

lua
```lua
local gl = require("module_gl")
gl.enable(gl.BLEND)
gl.blend_func(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
```

---

## gl.dummy_uniform_matrix4fv(location, count, transpose)

Description: Sets a hardcoded 4x4 orthographic projection matrix for an 800x600 resolution (for testing purposes).

Parameters:
- location (integer): Uniform location.
- count (integer): Number of matrices.
- transpose (boolean): Whether to transpose the matrix.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local location = gl.get_uniform_location(program, "uMVP")
gl.dummy_uniform_matrix4fv(location, 1, gl.FALSE)
```

---

## gl.delete_textures(textures)

Description: Deletes multiple texture objects.

Parameters:
- textures (table): Lua table of texture IDs.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local textures = {gl.gen_textures(), gl.gen_textures()}
gl.delete_textures(textures)
```

---

## gl.delete_buffers(buffers)

Description: Deletes multiple buffer objects.

Parameters:
- buffers (table): Lua table of buffer IDs.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local buffers = {gl.gen_buffers(), gl.gen_buffers()}
gl.delete_buffers(buffers)
```

---

## gl.delete_vertex_arrays(arrays)

Description: Deletes multiple vertex array objects.

Parameters:
- arrays (table): Lua table of VAO IDs.

Return: None

Example:

lua
```lua
local gl = require("module_gl")
local vaos = {gl.gen_vertex_arrays(), gl.gen_vertex_arrays()}
gl.delete_vertex_arrays(vaos)
```

---

## gl.cull_face(mode)

Description: Specifies which faces to cull.

Parameters:
- mode (integer): Culling mode (e.g., gl.BACK, gl.FRONT, gl.FRONT_AND_BACK).

Return:
- success (boolean): true if successful, false otherwise.
- err_msg (string, optional): Error message if an OpenGL error occurs.

Example:

lua
```lua
local gl = require("module_gl")
gl.enable(gl.CULL_FACE)
local success, err = gl.cull_face(gl.BACK)
if not success then
    print("Error:", err)
end
```

---

## gl.polygon_mode(face, mode)

Description: Sets the polygon rendering mode.

Parameters:
- face (integer): Polygon face (e.g., gl.FRONT_AND_BACK).
- mode (integer): Rendering mode (e.g., gl.LINE, gl.FILL).

Return:
- nil or (nil, error_message): Returns nil on success, or nil and an error message if an OpenGL error occurs.

Example:

lua
```lua
local gl = require("module_gl")
gl.polygon_mode(gl.FRONT_AND_BACK, gl.LINE)
```

---

# Constants

The module defines the following OpenGL constants for use in Lua scripts:

- gl.VERSION
- gl.VERTEX_SHADER
- gl.FRAGMENT_SHADER
- gl.ARRAY_BUFFER
- gl.STATIC_DRAW
- gl.FLOAT
- gl.TRIANGLES
- gl.COLOR_BUFFER_BIT
- gl.DEPTH_BUFFER_BIT
- gl.ELEMENT_ARRAY_BUFFER
- gl.UNSIGNED_INT
- gl.FALSE
- gl.TRUE
- gl.ONE
- gl.RED
- gl.TEXTURE_2D
- gl.TEXTURE_MIN_FILTER
- gl.TEXTURE_MAG_FILTER
- gl.NEAREST
- gl.LINEAR
- gl.RGB
- gl.RGBA
- gl.UNSIGNED_BYTE
- gl.TEXTURE0
- gl.TEXTURE1
- gl.BLEND
- gl.SRC_ALPHA
- gl.ONE_MINUS_SRC_ALPHA
- gl.ALPHA
- gl.TEXTURE_WRAP_S
- gl.TEXTURE_WRAP_T
- gl.CLAMP_TO_EDGE
- gl.DYNAMIC_DRAW
- gl.DEPTH_TEST
- gl.CULL_FACE
- gl.BACK
- gl.FRONT_AND_BACK
- gl.LINE
- gl.LESS
- gl.FRONT

Example Usage:

lua
```lua
local gl = require("module_gl")
print("Vertex shader type:", gl.VERTEX_SHADER)
```

---

# Notes:

- The module requires SDL3, GLAD 2.0, and cglm libraries.
- The OpenGL context must be initialized with gl.init before using most functions.
- Error handling is implemented for many functions, returning error messages when applicable.
- The gl.uniform_matrix4fv function supports both cglm mat4 userdata and raw 64-byte strings for matrix data.
- Memory management for textures, buffers, and vertex arrays is handled via table-based deletion functions.

This API provides a robust interface for OpenGL programming in Lua, suitable for rendering 2D and 3D graphics with SDL3 integration.