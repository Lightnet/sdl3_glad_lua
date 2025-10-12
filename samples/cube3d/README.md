# error 3d result of plane 3d fixed.
```
glUniformMatrix4fv(location, count, GL_FALSE, matrix);
```
for 3D render
0 = false

It does not work with lua_toboolean. It might be flip. It would return 1 and not zero.