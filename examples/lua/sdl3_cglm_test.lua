-- examples/sdl3_cglm_test.lua
local cglm = require "module_cglm"

-- Test vec3
print("--- Testing vec3 ---")
local v1 = cglm.vec3(1, 2, 3)
local v2 = cglm.vec3(4, 5, 6)
print("v1:", v1)  -- Expected: vec3(1, 2, 3)
print("v1[0]:", v1:get(0))  -- Expected: 1
v1:set(0, 10)
print("Modified v1:", v1)  -- Expected: vec3(10, 2, 3)
print("v1 + v2:", v1 + v2)  -- Expected: vec3(14, 7, 9)
print("v1 - v2:", v1 - v2)  -- Expected: vec3(6, -3, -3)
print("v1 * 2:", v1 * 2)  -- Expected: vec3(20, 4, 6)
print("Dot product:", v1:dot(v2))  -- Expected: 68
print("Cross product:", v1:cross(v2))  -- Expected: vec3(-3, -48, 42)
local v3 = cglm.vec3(3, 4, 0)
print("Length of v3:", v3:length())  -- Expected: 5
print("Normalized v3:", v3:normalize())  -- Expected: vec3(0.6, 0.8, 0)

-- Test vec4
print("\n--- Testing vec4 ---")
local v4 = cglm.vec4(1, 2, 3, 4)
local v5 = cglm.vec4(5, 6, 7, 8)
print("v4:", v4)  -- Expected: vec4(1, 2, 3, 4)
print("v4[0]:", v4:get(0))  -- Expected: 1
v4:set(0, 10)
print("Modified v4:", v4)  -- Expected: vec4(10, 2, 3, 4)
print("v4 + v5:", v4 + v5)  -- Expected: vec4(15, 8, 10, 12)
print("v4 - v5:", v4 - v5)  -- Expected: vec4(5, -4, -4, -4)
print("v4 * 2:", v4 * 2)  -- Expected: vec4(20, 4, 6, 8)
print("Dot product:", v4:dot(v5))  -- Expected: 115
print("Length of v4:", v4:length())  -- Expected: sqrt(129) ≈ 11.357816
print("Normalized v4:", v4:normalize())  -- Expected: vec4(0.880771, 0.176154, 0.264231, 0.352308)

-- Test mat4
print("\n--- Testing mat4 ---")
local m1 = cglm.mat4_identity()
print("Identity matrix:", m1)  -- Expected: identity matrix
local m2 = cglm.mat4 {
    1, 0, 0, 2,
    0, 1, 0, 3,
    0, 0, 1, 4,
    0, 0, 0, 1
} -- Translation matrix (2, 3, 4)
print("Translation matrix:", m2)
print("m2[0][3]:", m2:get(0, 3))  -- Expected: 2
m2:set(0, 3, 5)
print("Modified m2:", m2)  -- Expected: m2 with [0][3] = 5
local m3 = m1 * m2
print("m1 * m2:", m3)  -- Expected: same as modified m2