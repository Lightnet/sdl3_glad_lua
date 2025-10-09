local enet = require("module_enet")

-- Initialize ENet
enet.initialize()

-- Create server host (no specific address = bind to all interfaces)
local host = enet.host_create({host = "127.0.0.1", port = 12345}, 32, 0, 0, 0)
if not host then
    error("Failed to create server host")
end

print("Server listening on localhost:12345...")

-- Main service loop
while true do
    local event = enet.host_service(host, 1000)  -- 1 second timeout
    if type(event) == "table" and event.type then
        if event.type == enet.EVENT_TYPE_CONNECT then
            print("Client connected from:", event.peer)
        elseif event.type == enet.EVENT_TYPE_DISCONNECT then
            print("Client disconnected:", event.peer)
        elseif event.type == enet.EVENT_TYPE_RECEIVE then
            local data = enet.packet_data(event.packet)
            print("Received from", event.peer, ":", data)
            
            -- Echo back the message
            local echo_packet = enet.packet_create(data, enet.PACKET_FLAG_RELIABLE)
            enet.peer_send(event.peer, event.channelID, echo_packet)
            
            -- Destroy the received packet (ENet owns it after service)
            enet.packet_destroy(event.packet)
        end
    elseif event == 0 then
        -- No event, continue
    else
        print("Service error:", event)
        break
    end
end

-- Cleanup
enet.host_destroy(host)
enet.deinitialize()