local enet = require("module_enet")

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
    error("Failed to connect")
end

print("Connecting to server...")

-- Service loop to handle connect and send/receive
local connected = false
local message_sent = false
while not connected or not message_sent do
    local event = enet.host_service(host, 1000)
    if type(event) == "table" and event.type then
        if event.type == enet.EVENT_TYPE_CONNECT then
            print("Connected to server!")
            connected = true
            -- Send a message after connect
            local packet = enet.packet_create("Hello, server TESTME XD!", enet.PACKET_FLAG_RELIABLE)
            enet.peer_send(peer, 0, packet)
            message_sent = true
        elseif event.type == enet.EVENT_TYPE_RECEIVE then
            local data = enet.packet_data(event.packet)
            print("Received from server:", data)
            enet.packet_destroy(event.packet)
            break  -- Exit after receiving echo
        elseif event.type == enet.EVENT_TYPE_DISCONNECT then
            print("Disconnected from server")
            break
        end
    elseif event ~= 0 then
        print("Service error:", event)
        break
    end
end

-- Cleanup (disconnect and destroy)
-- enet.peer_send(peer, 0, nil)  -- Trigger disconnect --error crashed. just to test.
enet.host_service(host, 100)  -- Process disconnect event
enet.host_destroy(host)
enet.deinitialize()