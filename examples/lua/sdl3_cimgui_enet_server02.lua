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
local window, err = sdl.init_window("sdl3 enet server lua", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
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
local server_status = "None"
-- Initialize ENet
enet.initialize()

-- Create server host (no specific address = bind to all interfaces)
local host = enet.host_create({host = "127.0.0.1", port = 12345}, 32, 0, 0, 0)
if not host then
    error("Failed to create server host")
    server_status = "Off"
else
    server_status = "On"
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
local peer_id = nil

local client_peers = {}
local connection_times = {}

local function generatePeerId()
    -- Simple ID generator (replace with a more robust method if needed)
    return tostring(math.random(1000000)) -- Unique string ID
end

local function removePeerById(id)
    if id then
        client_peers[id] = nil
        connection_times[id] = nil
        print("Removed peer with connectID:", id)
    else
        print("Warning: Attempted to remove peer with nil connectID")
    end
end

local function countClients(tbl)
    local count = 0
    for _ in pairs(tbl) do
        count = count + 1
    end
    return count
end

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

    -- local event = enet.host_service(host, 1000)  -- 1 second timeout
    local event = enet.host_service(host, 0)  -- 1 second timeout
    if type(event) == "table" and event.type then
        if event.type == enet.EVENT_TYPE_CONNECT then
             local connect_id = enet.peer_get_connect_id(event.peer)
            if not connect_id then
                print("Error: Failed to get connect ID for peer:", event.peer)
                connect_id = tostring(math.random(1000000)) -- Fallback
            end
            print("[CONNECT] connect_id:", connect_id)
            print("Client connected with connectID:", connect_id, "from:", event.peer)
            client_peers[connect_id] = event.peer -- Update peer userdata
            connection_times[connect_id] = sdl.get_ticks()

        elseif event.type == enet.EVENT_TYPE_DISCONNECT then
           local connect_id = enet.peer_get_connect_id(event.peer)
            if not connect_id then
                print("Error: Failed to get connect ID for disconnecting peer:", event.peer)
            else
                print("[DISCONNECT] connect_id:", connect_id)
                print("Client disconnected with connectID:", connect_id, "from:", event.peer, "duration:", connection_times[connect_id] and (sdl.get_ticks() - connection_times[connect_id]) or 0, "ms")
                removePeerById(connect_id)
            end

        elseif event.type == enet.EVENT_TYPE_RECEIVE then
            local connect_id = enet.peer_get_connect_id(event.peer) or "unknown"
            local data = enet.packet_data(event.packet)
            print("Received from connectID:", connect_id, "peer:", event.peer, ":", data)
            print("event.channelID", event.channelID)
            -- Update peer userdata in client_peers
            if connect_id ~= "unknown" then
                client_peers[connect_id] = event.peer
            end
            local echo_packet = enet.packet_create(data, enet.PACKET_FLAG_RELIABLE)
            local result = enet.peer_send(event.peer, event.channelID, echo_packet)
            if result == 0 then
                print("Echoed packet to connectID:", connect_id)
            else
                print("Failed to echo packet to connectID:", connect_id, "error:", result)
            end
            enet.packet_destroy(event.packet)
        end
    elseif event == 0 then
        -- No event, continue
    else
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
        local text_server_status =  "Server:" .. server_status
        imgui.ig_text(text_server_status)

        imgui.ig_text("Clients: " .. tostring(countClients(client_peers)))

        if imgui.ig_button("client(s)") then
            print("clients: ", countClients(client_peers))
        end

        if imgui.ig_button("ping") then
            print("ping!")
            for connect_id, peer in pairs(client_peers) do
                local packet = enet.packet_create("Hello, client!", enet.PACKET_FLAG_RELIABLE)
                local result = enet.peer_send(peer, 0, packet)
                if result == 0 then
                    print("Sent packet to connectID:", connect_id)
                else
                    print("Failed to send packet to connectID:", connect_id, "error:", result)
                end
            end
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

enet.host_destroy(host)
enet.deinitialize()

imgui.shutdown()
gl.destroy()
sdl.quit()