


# Bugs on Grok:
- Lua string.pack for projection or misconfig


```
Lua string.pack Format Issue
- The original gl_uniform_matrix4fv expected a binary string of 16 GLfloat values (4-byte floats) in column-major order, created by string.pack("f", value) in main.lua.
- Lua’s "f" format uses the platform’s float type, which is typically 4 bytes (IEEE 754), matching GLfloat. However, potential issues include:
    - Alignment/Padding: Lua strings may include padding or alignment that misaligns the data when cast to (const GLfloat *)matrix in C.
    - Endianness: If the Lua environment and C/OpenGL expect different byte orders (e.g., big-endian vs. little-endian), the float values could be misinterpreted.
    - String Integrity: Lua strings can contain null bytes, which might truncate or corrupt the data when passed to C.
```
- Matrix Transfer
    - The original gl_uniform_matrix4fv cast the Lua string directly to (const GLfloat *)matrix without validating its length or contents. If projection_data was malformed (e.g., wrong size or corrupted values), the matrix sent to the shader could have been invalid, causing the quad to be transformed incorrectly (e.g., off-screen or degenerate).
    - The mat4 userdata approach uses cglm’s internal float[4][4] array, which is guaranteed to be 16 contiguous 4-byte floats in column-major order, matching OpenGL’s expectations.

- The mat4 userdata approach is working because it:
    - Avoids string.pack, eliminating potential format issues.
    - Uses cglm’s mat4 type, which is compatible with glUniformMatrix4fv (both use column-major 4x4 float arrays).
    - Matches the logged matrix values, ensuring correct transformation.
- The logs confirm the matrix is correct:
    - Lua: projection[0][0] = 0.002500, [1][1] = 0.003333, [3][0] = -1.000000, [3][1] = -1.000000
    - C: Using cglm.mat4: [0][0]=0.002500, [1][1]=0.003333, [3][0]=-1.000000, [3][1]=-1.000000









