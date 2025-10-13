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
local window, err = sdl.init_window("sdl3 enet client lua", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
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
local connected = false
local peer = nil
local connect_id = nil
local last_reconnect = 0
local reconnect_interval = 5000
-- Initialize ENet
enet.initialize()

-- Create client host (single peer, no bandwidth limits)
local host = enet.host_create({}, 1, 0, 0, 0)
if not host then
    error("Failed to create client host")
end

-- Function to attempt connection
local function connect_to_server()
    peer = enet.host_connect(host, {host = "127.0.0.1", port = 12345}, 0, 0)
    if not peer then
        connect_status = "Failed to connect"
        return false
    end
    connect_status = "Connecting..."
    connect_id = enet.peer_get_connect_id(peer)
    if not connect_id then
        print("Failed to get connect ID, using random ID")
        connect_id = tostring(math.random(1000000))
    end
    print("Set client connectID:", connect_id)
    enet.host_flush(host)
    return true
end

-- Initial connection attempt
if not connect_to_server() then
    error("Initial connection failed")
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

    -- Automatic reconnection
    if not connected and sdl.get_ticks() - last_reconnect > reconnect_interval then
        if connect_to_server() then
            print("Reconnection attempt started")
        else
            print("Reconnection failed")
        end
        last_reconnect = sdl.get_ticks()
    end

    -- Handle ENet events
    local event = enet.host_service(host, 0)
    if type(event) == "table" and event.type then
        if event.type == enet.EVENT_TYPE_CONNECT then
            local incoming_id = enet.peer_get_incoming_peer_id(peer)
            local state = enet.peer_get_state(peer)
            print("Connected to server! connectID:", connect_id, "incoming_peer_id:", incoming_id, "state:", state)
            connect_status = "Connected"
            connected = true
            local data = {message = "Hello, server!", connect_id = tostring(connect_id), timestamp = sdl.get_ticks()}
            local packet = enet.packet_create_table(data, enet.PACKET_FLAG_RELIABLE)
            local result = enet.peer_send(peer, 0, packet)
            if result == 0 then
                print("Sent initial table message")
            else
                print("Failed to send initial table message:", result)
            end
            enet.peer_ping(peer)
            enet.host_flush(host)

        elseif event.type == enet.EVENT_TYPE_RECEIVE then
            local data = enet.packet_data(event.packet)
            if type(data) == "table" then
                print("Received table from server:")
                for k, v in pairs(data) do
                    print("  ", k, "=", v)
                end
            else
                print("Received string from server:", data)
            end
            enet.packet_destroy(event.packet)

        elseif event.type == enet.EVENT_TYPE_DISCONNECT then
            local state = enet.peer_get_state(peer)
            print("Disconnected from server. connectID:", connect_id, "state:", state)
            connect_status = "Disconnected"
            connected = false
            peer = nil
            connect_id = nil
        end
    elseif event ~= 0 then
        print("Check events error:", event)
        connect_status = "Service error"
        connected = false
        peer = nil
        connect_id = nil
    end

    -- Start ImGui frame
    imgui.new_frame() -- start drawing imgui
    -- create widgets here

    -- Create a simple ImGui window
    local open = imgui.ig_begin("Test Window", true)
    if open then
        imgui.ig_text("Hello, ImGui from Lua!")
        imgui.ig_text("Client Status: " .. connect_status)
        if peer_id then
            imgui.ig_text("Peer ID: " .. peer_id)
        end

        if imgui.ig_button("Ping") then
             if connected and peer then
                local packet = enet.packet_create("Ping from client! connectID: " .. connect_id, enet.PACKET_FLAG_RELIABLE)
                local result = enet.peer_send(peer, 0, packet)
                if result == 0 then
                    print("Sent ping to server")
                else
                    print("Failed to send ping:", result)
                end
                enet.host_flush(host)
            else
                print("Cannot ping: Not connected")
            end
        end

        if imgui.ig_button("send table") then
            if connected and peer then
                enet.peer_ping(peer)
                local data = {message = "Ping from client!", connect_id = tostring(connect_id), timestamp = sdl.get_ticks(),test="data"}
                local packet = enet.packet_create_table(data, enet.PACKET_FLAG_RELIABLE)
                local result = enet.peer_send(peer, 0, packet)
                if result == 0 then
                    print("Sent table ping to server")
                else
                    print("Failed to send table ping:", result)
                end
                enet.host_flush(host)
            else
                print("Cannot ping: Not connected")
            end
        end


        if not connected and imgui.ig_button("Reconnect") then
            if connect_to_server() then
                print("Manual reconnection attempt started")
            else
                print("Manual reconnection failed")
            end
            last_reconnect = sdl.get_ticks()
        end

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
if peer and connected then
    enet.peer_disconnect(peer, 0)
    local timeout = 1000
    local start = sdl.get_ticks()
    while sdl.get_ticks() - start < timeout do
        local event = enet.host_service(host, 0)
        if type(event) == "table" and event.type == enet.EVENT_TYPE_DISCONNECT then
            print("Graceful disconnect completed")
            break
        end
        sdl.delay(10)
    end
end
-- enet.peer_disconnect(peer, 0) -- disconnect
-- enet.host_service(host, 0)  -- Process disconnect event, need to send to server
enet.host_destroy(host)

imgui.shutdown()
gl.destroy()
sdl.quit()