-- Load required modules
local sdl = require("module_sdl")
local gl = require("module_gl") 
local imgui = require("module_imgui")
local enet = require("module_enet")

-- Initialize SDL with video and events subsystems
local success, err = sdl.init(sdl.INIT_VIDEO + sdl.INIT_EVENTS)
if not success then
    print("SDL init failed: " .. err)
    return
end

-- Create window with OpenGL and resizable flags
local window, err = sdl.init_window("sdl3 lua", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
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

-- Access stored gl context
-- local gl_context = gl.get_gl_context()
-- if not gl_context then
--     print("Failed to get GL context: ", select(2, gl.get_gl_context()))
--     return
-- end
local connect_status = "None"
-- Initialize ENet
enet.initialize()

-- Create client host (single peer, no bandwidth limits)
local host = enet.host_create({}, 1, 0, 0, 0)
if not host then
    error("Failed to create client host")
end

-- Connect to server
-- local peer = enet.host_connect(host, {host = "localhost", port = 12345}, 0, 0)
local peer = enet.host_connect(host, {host = "127.0.0.1", port = 12345}, 0, 0)
if not peer then
    connect_status = "Off"
    error("Failed to connect")
else
    connect_status = "On"
end


-- Initialize ImGui with sdl_window and gl_context
success, err = imgui.init(window, gl_context)
if not success then
    print("ImGui init failed: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Main loop
local running = true
local connected = false
local message_sent = false
while running do
    -- Poll SDL events
    local events = sdl.poll_events_ig() -- Use poll_events_ig to process ImGui inputs
    for i, event in ipairs(events) do
        if event.type == sdl.EVENT_QUIT then
            running = false
        elseif event.type == sdl.EVENT_WINDOW_RESIZED then
            gl.viewport(0, 0, event.width, event.height)
        end
    end


    local event = enet.host_service(host, 0)
    if type(event) == "table" and event.type then
        if event.type == enet.EVENT_TYPE_CONNECT then
            print("Connected to server!")
            connected = true
            -- Send a message after connect
            -- local packet = enet.packet_create("Hello, server TESTME XD!", enet.PACKET_FLAG_RELIABLE)
            -- enet.peer_send(peer, 0, packet)
            -- message_sent = true
        elseif event.type == enet.EVENT_TYPE_RECEIVE then
            local data = enet.packet_data(event.packet)
            print("Received from server:", data)
            enet.packet_destroy(event.packet)
            -- break  -- Exit after receiving echo
        elseif event.type == enet.EVENT_TYPE_DISCONNECT then
            print("Disconnected from server")
            -- break
            running = false
        end
    elseif event ~= 0 then
        print("Service error:", event)
        -- break
        running = false
    end

    -- Start ImGui frame
    imgui.new_frame() -- start drawing imgui
    -- create widgets here

    -- Create a simple ImGui window
    local open = imgui.ig_begin("Test Window", true)
    if open then
        imgui.ig_text("Hello, ImGui from Lua!")
        imgui.ig_text("Client:")
        imgui.ig_text(connect_status)

        if imgui.ig_button("ping") then
            local packet = enet.packet_create("Hello, server TESTME XD!", enet.PACKET_FLAG_RELIABLE)
            enet.peer_send(peer, 0, packet)
        end


        -- if imgui.ig_button("Click Me") then
        --     print("Button clicked!")
        -- end

        -- if imgui.ig_button("Window ID") then
        --     local id = sdl.get_window_id(window)
        --     local display_id = sdl.get_primary_display(window)
        --     print("window id:", id)
        --     print("display_id:", display_id)
        -- end

        -- if imgui.ig_button("get_current_gl_context") then
        --     local context = sdl.get_current_gl_context()
        --     print("context:", context)
        -- end

        imgui.ig_end()
    end
    imgui.render() -- end drawing imgui

    -- Clear the screen
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT)

    -- Render ImGui
    imgui.render_draw_data() -- draw gl imgui 

    -- Swap window
    sdl.gl_swap_window(window)
end

-- Cleanup

enet.host_service(host, 100)  -- Process disconnect event
enet.host_destroy(host)

imgui.shutdown()
gl.destroy()
sdl.quit()