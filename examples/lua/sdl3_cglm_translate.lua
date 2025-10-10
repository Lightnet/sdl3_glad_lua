-- Set up matrices
local projection = cglm.perspective(math.rad(45), 800 / 600, 0.1, 100.0)
local view = cglm.mat4()
cglm.translate(view, cglm.vec3(0, 0, -3)) -- Move camera back
local model = cglm.mat4_identity()

-- Animation variables
local angle = 0

-- Main loop
while running do
    -- ...
    -- Compute MVP matrix
    local mvp = cglm.mat4()
    cglm.mul(projection, view, mvp)
    cglm.mul(mvp, model, mvp)
    -- ...
end