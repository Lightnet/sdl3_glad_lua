

```c
GLboolean normalized = (GLboolean)luaL_checkinteger(L, 4); // Expect integer (0 or 1)
```
```lua
gl.vertex_attrib_pointer(0, 2, gl.FLOAT, gl.FALSE, 2 * 4, 0) -- Use gl.FALSE (0)
```