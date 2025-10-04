-- examples/sdl3_cglm_test.lua
local cglm = require "module_cglm"

-- Create vectors
local v1 = cglm.vec3(1, 2, 3)
local v2 = cglm.vec3(4, 5, 6)

-- Test component access
print("v1:", v1)  -- Expected: vec3(1, 2, 3)
print("v1[0]:", v1:get(0))  -- Expected: 1
v1:set(0, 10)
print("Modified v1:", v1)  -- Expected: vec3(10, 2, 3)

-- Test arithmetic
local v3 = v1 + v2
print("v1 + v2:", v3)  -- Expected: vec3(14, 7, 9)
local v4 = v1 - v2
print("v1 - v2:", v4)  -- Expected: vec3(6, -3, -3)
local v5 = v1 * 2
print("v1 * 2:", v5)  -- Expected: vec3(20, 4, 6)

-- Test dot and cross products
print("Dot product:", v1:dot(v2))  -- Expected: 68
local v6 = v1:cross(v2)
print("Cross product:", v6)  -- Expected: vec3(-3, -48, 42)

-- Test normalization and length
local v7 = cglm.vec3(3, 4, 0)
print("Length of v7:", v7:length())  -- Expected: 5
local v8 = v7:normalize()
print("Normalized v7:", v8)  -- Expected: vec3(0.6, 0.8, 0)