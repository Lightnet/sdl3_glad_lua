# Lua ENet API Documentation

This module provides a Lua interface to the ENet networking library, enabling fast and reliable UDP networking for games and real-time applications. The module exposes functions to initialize ENet, create and manage hosts and peers, send packets, and handle network events.

# Module Functions

## enet.initialize()

Description: Initializes the ENet library. Must be called before any other ENet functions.

Parameters: None

Return:
- integer: 0 on success, non-zero on failure.

Example:

lua
```lua
local enet = require("module_enet")
local result = enet.initialize()
if result == 0 then
    print("ENet initialized successfully")
else
    print("ENet initialization failed")
end
```

---

## enet.deinitialize()

Description: Deinitializes the ENet library, cleaning up resources. Should be called when done using ENet.

Parameters: None

Return: None

Example:

lua
```lua
local enet = require("module_enet")
enet.initialize()
-- Perform networking operations
enet.deinitialize()
print("ENet deinitialized")
```

---

## enet.host_create(address, peerCount, channelCount, incomingBandwidth, outgoingBandwidth)

Description: Creates an ENet host for either server or client functionality.

Parameters:
- address (table, optional): A table with fields:
    - host (string): IP address (e.g., "127.0.0.1") or hostname. If omitted, binds to all interfaces (server) or sets to undefined (client).
    - port (number): Port number.
    - sin6_scope_id (number, optional): IPv6 scope ID for link-local addresses.
- peerCount (number, optional): Maximum number of peers (default: 1).
- channelCount (number, optional): Number of channels for communication (default: 0).
- incomingBandwidth (number, optional): Incoming bandwidth limit in bytes/sec (default: 0, unlimited).
- outgoingBandwidth (number, optional): Outgoing bandwidth limit in bytes/sec (default: 0, unlimited).

Return:
- userdata (ENetHost): The created host, or nil if creation failed.

Example:

lua
```lua
local enet = require("module_enet")
enet.initialize()
local host = enet.host_create({ host = "0.0.0.0", port = 6789 }, 32, 2)
if host then
    print("Server created on port 6789")
else
    print("Failed to create server")
end
```

---

## enet.host_destroy(host)

Description: Destroys an ENet host, freeing its resources.

Parameters:
- host (userdata, ENetHost): The host to destroy.

Return: None

Example:

lua
```lua
local enet = require("module_enet")
enet.initialize()
local host = enet.host_create({ host = "0.0.0.0", port = 6789 }, 32, 2)
-- Use host
enet.host_destroy(host)
print("Host destroyed")
```

---

## enet.host_connect(host, address, channelCount, data)

Description: Initiates a connection from a host to a remote peer.

Parameters:
- host (userdata, ENetHost): The client host initiating the connection.
- address (table): A table with fields:
    - host (string): Remote IP address or hostname.
    - port (number): Remote port.
    - sin6_scope_id (number, optional): IPv6 scope ID for link-local addresses.
- channelCount (number, optional): Number of channels for communication (default: 0).
- data (number, optional): User-defined data to send with the connect request (default: 0).

Return:
- userdata (ENetPeer): The peer representing the connection, or nil if the connection failed.

Example:

lua
```lua
local enet = require("module_enet")
enet.initialize()
local host = enet.host_create(nil, 1, 2) -- Client host
local peer = enet.host_connect(host, { host = "127.0.0.1", port = 6789 }, 2)
if peer then
    print("Connecting to server...")
else
    print("Connection failed")
end
```

---

## enet.host_service(host, timeout)

Description: Polls the host for network events (connect, disconnect, receive) within a specified timeout.

Parameters:
- host (userdata, ENetHost): The host to poll.
- timeout (number, optional): Timeout in milliseconds (default: 1000).

Return:
- integer: Number of events processed, or -1 if the host is invalid.
- table (if events processed > 0): Event details with fields:
    - type (number): Event type (enet.EVENT_TYPE_NONE, enet.EVENT_TYPE_CONNECT, enet.EVENT_TYPE_DISCONNECT, or enet.EVENT_TYPE_RECEIVE).
    - peer (userdata, ENetPeer, optional): Peer associated with the event.
    - channelID (number): Channel ID of the event.
    - packet (userdata, ENetPacket, optional): Packet received (for EVENT_TYPE_RECEIVE).
    - data (number): User-defined data associated with the event.

Example:

lua
```lua
local enet = require("module_enet")
enet.initialize()
local host = enet.host_create({ host = "0.0.0.0", port = 6789 }, 32, 2)
while true do
    local result, event = enet.host_service(host, 1000)
    if result > 0 then
        if event.type == enet.EVENT_TYPE_CONNECT then
            print("Peer connected")
        elseif event.type == enet.EVENT_TYPE_RECEIVE then
            local data = enet.packet_data(event.packet)
            print("Received data: " .. data)
            enet.packet_destroy(event.packet)
        elseif event.type == enet.EVENT_TYPE_DISCONNECT then
            print("Peer disconnected")
        end
    end
end
```

---

## enet.packet_create(data, flags)

Description: Creates a new ENet packet for sending data.Parameters:
- data (string): The data to include in the packet.
- flags (number, optional): Packet flags (e.g., enet.PACKET_FLAG_RELIABLE, default: reliable).

Return:
- userdata (ENetPacket): The created packet, or nil if creation failed.

Example:

lua

```lua
local enet = require("module_enet")
local packet = enet.packet_create("Hello, world!", enet.PACKET_FLAG_RELIABLE)
if packet then
    print("Packet created")
else
    print("Failed to create packet")
end
```

---

## enet.packet_destroy(packet)

Description: Destroys an ENet packet, freeing its resources.Parameters:
- packet (userdata, ENetPacket): The packet to destroy.

Return: None

Example:

lua
```lua
local enet = require("module_enet")
local packet = enet.packet_create("Hello, world!")
-- Use packet
enet.packet_destroy(packet)
print("Packet destroyed")
```

---

## enet.packet_data(packet)

Description: Retrieves the data from an ENet packet.

Parameters:
- packet (userdata, ENetPacket): The packet to read.

Return:
- string: The packet's data, or nil if the packet is invalid.

Example:

lua
```lua
local enet = require("module_enet")
local packet = enet.packet_create("Hello, world!")
local data = enet.packet_data(packet)
print("Packet data: " .. data) -- Outputs: Packet data: Hello, world!
enet.packet_destroy(packet)
```

---

## enet.peer_send(peer, channelID, packet)

Description: Sends a packet to a peer over a specific channel.

Parameters:
- peer касается (userdata, ENetPeer): The peer to send to.
- channelID (number): The channel ID to use (0-based).
- packet (userdata, ENetPacket): The packet to send.

Return:
- integer: 0 on success, negative on failure. On success, the packet is owned by ENet and should not be destroyed manually.

Example:

lua
```lua
local enet = require("module_enet")
enet.initialize()
local host = enet.host_create(nil, 1, 2)
local peer = enet.host_connect(host, { host = "127.0.0.1", port = 6789 }, 2)
local packet = enet.packet_create("Hello, server!", enet.PACKET_FLAG_RELIABLE)
local result = enet.peer_send(peer, 0, packet)
if result == 0 then
    print("Packet sent successfully")
else
    print("Failed to send packet")
end
```

---

# Constants

The module provides the following constants:

- Event Types:
    - enet.EVENT_TYPE_NONE: No event occurred.
    - enet.EVENT_TYPE_CONNECT: A peer connected.
    - enet.EVENT_TYPE_DISCONNECT: A peer disconnected.
    - enet.EVENT_TYPE_RECEIVE: A packet was received.
- Packet Flags:
    - enet.PACKET_FLAG_RELIABLE: Ensures reliable delivery.
    - enet.PACKET_FLAG_NO_ALLOCATE: Packet uses pre-allocated data (not commonly used).

---

# Example: Simple Client-Server

Below is a complete example demonstrating a server and client using the ENet Lua module.Server (server.lua):

lua

```lua
local enet = require("module_enet")
enet.initialize()

local host = enet.host_create({ host = "0.0.0.0", port = 6789 }, 32, 2)
if not host then
    error("Failed to create server")
end

print("Server running on port 6789...")

while true do
    local result, event = enet.host_service(host, 1000)
    if result > 0 then
        if event.type == enet.EVENT_TYPE_CONNECT then
            print("Client connected")
        elseif event.type == enet.EVENT_TYPE_RECEIVE then
            local data = enet.packet_data(event.packet)
            print("Received: " .. data)
            local response = enet.packet_create("Hello, client!", enet.PACKET_FLAG_RELIABLE)
            enet.peer_send(event.peer, 0, response)
            enet.packet_destroy(event.packet)
        elseif event.type == enet.EVENT_TYPE_DISCONNECT then
            print("Client disconnected")
        end
    end
end
```

# Client (client.lua):

lua
```lua
local enet = require("module_enet")
enet.initialize()

local host = enet.host_create(nil, 1, 2)
if not host then
    error("Failed to create client")
end

local peer = enet.host_connect(host, { host = "127.0.0.1", port = 6789 }, 2)
if not peer then
    error("Failed to connect to server")
end

local packet = enet.packet_create("Hello, server!", enet.PACKET_FLAG_RELIABLE)
enet.peer_send(peer, 0, packet)

print("Client connecting...")

while true do
    local result, event = enet.host_service(host, 1000)
    if result > 0 then
        if event.type == enet.EVENT_TYPE_CONNECT then
            print("Connected to server")
        elseif event.type == enet.EVENT_TYPE_RECEIVE then
            local data = enet.packet_data(event.packet)
            print("Received: " .. data)
            enet.packet_destroy(event.packet)
        elseif event.type == enet.EVENT_TYPE_DISCONNECT then
            print("Disconnected from server")
            break
        end
    end
end

enet.deinitialize()
```

---

# Notes:

- Memory Management: The module handles garbage collection for ENetHost, ENetPeer, and ENetPacket objects via Lua metatables. Peers are managed by their host, so their __gc method only nullifies the reference.
- Packet Ownership: After a successful enet.peer_send, ENet takes ownership of the packet, and it should not be destroyed manually.
- Error Handling: Functions return nil or negative integers on failure. Always check return values.
- IPv6 Support: The sin6_scope_id field in address tables supports IPv6 link-local addresses.

This documentation covers the core functionality of the Lua ENet module. For advanced usage, refer to the ENet library documentation for underlying behavior details.