


# 
```lua
local cglm = require "module_cglm"
```
# vec3
```lua
local v1 = cglm.vec3(1, 2, 3)
```
# vec3 -> set
```lua
vec3:set(index, value)
```
```lua
local v1 = cglm.vec3(1, 2, 3)
v1:set(0, 10)
```

```lua
local v = cglm.vec3(1, 2, 3)
v:set(0, 10) -- Sets first component (x) to 10
v:set(1, 20) -- Sets second component (y) to 20
v:set(2, 30) -- Sets third component (z) to 30
print(v) -- Outputs: vec3(10, 20, 30)
```


# vect -> get(index):

```lua
local v1 = cglm.vec3(1, 2, 3)
v1:get(0)
```