-- main.lua: Base Lua script for SDL3 window
local sdl = require("module_sdl")
local util = require("module_lua")

util.log("Starting Lua script...")

-- Check if another path exists (demo)
if util.path_exists("somefile.txt") then
    util.log("somefile.txt found!")
end

-- Simple event handler
while true do
    local events = sdl.poll_events()
    for i = 1, #events do
        local ev = events[i]
        if ev.type == "SDL_EVENT_QUIT" then
            util.log("Quit event!")
            sdl.quit()
            return
        end
    end
end