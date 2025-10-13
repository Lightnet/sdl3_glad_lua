#include "module_enet.h"
#include <lauxlib.h>
#include <lualib.h>
#include <enet.h>
#include <string.h>
#include <stdlib.h>

// Metatable names
#define ENET_HOST_MT "ENetHost"
#define ENET_PEER_MT "ENetPeer"
#define ENET_PACKET_MT "ENetPacket"
#define ENET_PEER_REGISTRY "enet_peer_registry"

// Table packet prefix
#define TABLE_PACKET_PREFIX "TABLE:"

// Helper to push ENetHost userdata
static ENetHost** push_enet_host(lua_State *L) {
    ENetHost **host = (ENetHost**)lua_newuserdata(L, sizeof(ENetHost*));
    luaL_getmetatable(L, ENET_HOST_MT);
    lua_setmetatable(L, -2);
    return host;
}

// Helper to push ENetPeer userdata
static ENetPeer** push_enet_peer(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)lua_newuserdata(L, sizeof(ENetPeer*));
    luaL_getmetatable(L, ENET_PEER_MT);
    lua_setmetatable(L, -2);
    return peer;
}

// Helper to push ENetPacket userdata
static ENetPacket** push_enet_packet(lua_State *L) {
    ENetPacket **packet = (ENetPacket**)lua_newuserdata(L, sizeof(ENetPacket*));
    luaL_getmetatable(L, ENET_PACKET_MT);
    lua_setmetatable(L, -2);
    return packet;
}

// enet.initialize()
static int l_enet_initialize(lua_State *L) {
    int result = enet_initialize();
    lua_pushinteger(L, result);
    return 1;
}

// enet.deinitialize()
static int l_enet_deinitialize(lua_State *L) {
    enet_deinitialize();
    return 0;
}

// enet.host_create(address, peerCount, channelCount, incomingBandwidth, outgoingBandwidth)
static int l_enet_host_create(lua_State *L) {
    ENetAddress *address = NULL;
    if (lua_istable(L, 1)) {
        address = (ENetAddress*)malloc(sizeof(ENetAddress));
        if (!address) {
            fprintf(stderr, "enet.host_create: Failed to allocate address\n");
            lua_pushnil(L);
            return 1;
        }
        memset(address, 0, sizeof(ENetAddress));
        lua_getfield(L, 1, "host");
        if (lua_isstring(L, -1)) {
            enet_address_set_host_new(address, lua_tostring(L, -1));
        }
        lua_pop(L, 1);
        lua_getfield(L, 1, "port");
        if (lua_isnumber(L, -1)) {
            address->port = (enet_uint16)lua_tointeger(L, -1);
        }
        lua_pop(L, 1);
        lua_getfield(L, 1, "sin6_scope_id");
        if (lua_isnumber(L, -1)) {
            address->sin6_scope_id = (enet_uint32)lua_tointeger(L, -1);
        }
        lua_pop(L, 1);
    }

    size_t peerCount = (size_t)luaL_optinteger(L, 2, 1);
    size_t channelCount = (size_t)luaL_optinteger(L, 3, 0);
    enet_uint32 incomingBandwidth = (enet_uint32)luaL_optinteger(L, 4, 0);
    enet_uint32 outgoingBandwidth = (enet_uint32)luaL_optinteger(L, 5, 0);

    ENetHost *host = enet_host_create(address, peerCount, channelCount, incomingBandwidth, outgoingBandwidth);
    if (address) free(address);
    if (host == NULL) {
        lua_pushnil(L);
    } else {
        ENetHost **ud = push_enet_host(L);
        *ud = host;
    }
    return 1;
}

// enet.host_destroy(host)
static int l_enet_host_destroy(lua_State *L) {
    ENetHost **host = (ENetHost**)luaL_checkudata(L, 1, ENET_HOST_MT);
    if (host && *host) {
        enet_host_destroy(*host);
        *host = NULL;
    }
    return 0;
}

// enet.host_connect(host, address, channelCount, data)
static int l_enet_host_connect(lua_State *L) {
    ENetHost **host = (ENetHost**)luaL_checkudata(L, 1, ENET_HOST_MT);
    if (!host || !*host) {
        fprintf(stderr, "enet.host_connect: Invalid host\n");
        lua_pushnil(L);
        return 1;
    }

    ENetAddress address = {0};
    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "host");
        if (lua_isstring(L, -1)) {
            enet_address_set_host_new(&address, lua_tostring(L, -1));
        }
        lua_pop(L, 1);
        lua_getfield(L, 2, "port");
        if (lua_isnumber(L, -1)) {
            address.port = (enet_uint16)lua_tointeger(L, -1);
        }
        lua_pop(L, 1);
        lua_getfield(L, 2, "sin6_scope_id");
        if (lua_isnumber(L, -1)) {
            address.sin6_scope_id = (enet_uint32)lua_tointeger(L, -1);
        }
        lua_pop(L, 1);
    } else {
        lua_pushnil(L);
        return 1;
    }

    size_t channelCount = (size_t)luaL_optinteger(L, 3, 0);
    enet_uint32 data = (enet_uint32)luaL_optinteger(L, 4, 0);

    ENetPeer *peer = enet_host_connect(*host, &address, channelCount, data);
    if (peer == NULL) {
        lua_pushnil(L);
    } else {
        ENetPeer **ud = push_enet_peer(L);
        *ud = peer;
        lua_getfield(L, LUA_REGISTRYINDEX, ENET_PEER_REGISTRY);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_setfield(L, LUA_REGISTRYINDEX, ENET_PEER_REGISTRY);
            lua_getfield(L, LUA_REGISTRYINDEX, ENET_PEER_REGISTRY);
        }
        lua_pushlightuserdata(L, (void*)peer);
        lua_pushvalue(L, -2);
        lua_settable(L, -3);
        lua_pop(L, 1);
    }
    return 1;
}

// enet.host_service(host, timeout)
static int l_enet_host_service(lua_State *L) {
    ENetHost **host = (ENetHost**)luaL_checkudata(L, 1, ENET_HOST_MT);
    if (!host || !*host) {
        fprintf(stderr, "enet.host_service: Invalid host\n");
        lua_pushinteger(L, -1);
        return 1;
    }
    enet_uint32 timeout = (enet_uint32)luaL_optinteger(L, 2, 1000);
    ENetEvent event;

    int result = enet_host_service(*host, &event, timeout);
    if (result > 0) {
        lua_newtable(L);
        lua_pushinteger(L, (lua_Integer)event.type);
        lua_setfield(L, -2, "type");
        if (event.peer) {
            lua_getfield(L, LUA_REGISTRYINDEX, ENET_PEER_REGISTRY);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                lua_newtable(L);
                lua_setfield(L, LUA_REGISTRYINDEX, ENET_PEER_REGISTRY);
                lua_getfield(L, LUA_REGISTRYINDEX, ENET_PEER_REGISTRY);
            }
            lua_pushlightuserdata(L, (void*)event.peer);
            lua_gettable(L, -2);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                ENetPeer **ud = push_enet_peer(L);
                *ud = event.peer;
                lua_pushlightuserdata(L, (void*)event.peer);
                lua_pushvalue(L, -2);
                lua_settable(L, -4);
            }
            lua_setfield(L, -3, "peer");
            lua_pop(L, 1);
        }
        lua_pushinteger(L, (lua_Integer)event.channelID);
        lua_setfield(L, -2, "channelID");
        if (event.packet) {
            ENetPacket **ud = push_enet_packet(L);
            *ud = event.packet;
            lua_setfield(L, -2, "packet");
        }
        lua_pushinteger(L, (lua_Integer)event.data);
        lua_setfield(L, -2, "data");
        return 1;
    }
    lua_pushinteger(L, result);
    return 1;
}

// enet.host_check_events(host)
static int l_enet_host_check_events(lua_State *L) {
    ENetHost **host = (ENetHost**)luaL_checkudata(L, 1, ENET_HOST_MT);
    if (!host || !*host) {
        fprintf(stderr, "enet.host_check_events: Invalid host\n");
        lua_pushinteger(L, -1);
        return 1;
    }
    ENetEvent event;

    int result = enet_host_check_events(*host, &event);
    if (result > 0) {
        lua_newtable(L);
        lua_pushinteger(L, (lua_Integer)event.type);
        lua_setfield(L, -2, "type");
        if (event.peer) {
            lua_getfield(L, LUA_REGISTRYINDEX, ENET_PEER_REGISTRY);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                lua_newtable(L);
                lua_setfield(L, LUA_REGISTRYINDEX, ENET_PEER_REGISTRY);
                lua_getfield(L, LUA_REGISTRYINDEX, ENET_PEER_REGISTRY);
            }
            lua_pushlightuserdata(L, (void*)event.peer);
            lua_gettable(L, -2);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                ENetPeer **ud = push_enet_peer(L);
                *ud = event.peer;
                lua_pushlightuserdata(L, (void*)event.peer);
                lua_pushvalue(L, -2);
                lua_settable(L, -4);
            }
            lua_setfield(L, -3, "peer");
            lua_pop(L, 1);
        }
        lua_pushinteger(L, (lua_Integer)event.channelID);
        lua_setfield(L, -2, "channelID");
        if (event.packet) {
            ENetPacket **ud = push_enet_packet(L);
            *ud = event.packet;
            lua_setfield(L, -2, "packet");
        }
        lua_pushinteger(L, (lua_Integer)event.data);
        lua_setfield(L, -2, "data");
        return 1;
    }
    lua_pushinteger(L, result);
    return 1;
}

// enet.host_bandwidth_limit(host, incomingBandwidth, outgoingBandwidth)
static int l_enet_host_bandwidth_limit(lua_State *L) {
    ENetHost **host = (ENetHost**)luaL_checkudata(L, 1, ENET_HOST_MT);
    if (!host || !*host) {
        fprintf(stderr, "enet.host_bandwidth_limit: Invalid host\n");
        lua_pushnil(L);
        return 1;
    }
    enet_uint32 incomingBandwidth = (enet_uint32)luaL_checkinteger(L, 2);
    enet_uint32 outgoingBandwidth = (enet_uint32)luaL_checkinteger(L, 3);
    enet_host_bandwidth_limit(*host, incomingBandwidth, outgoingBandwidth);
    return 0;
}

// enet.packet_create(data, flags)
static int l_enet_packet_create(lua_State *L) {
    size_t data_len;
    const char *data = luaL_checklstring(L, 1, &data_len);
    enet_uint32 flags = (enet_uint32)luaL_optinteger(L, 2, ENET_PACKET_FLAG_RELIABLE);

    ENetPacket *packet = enet_packet_create(data, data_len, flags);
    if (packet == NULL) {
        fprintf(stderr, "enet.packet_create: Failed to create packet\n");
        lua_pushnil(L);
    } else {
        ENetPacket **ud = push_enet_packet(L);
        *ud = packet;
    }
    return 1;
}

// enet.packet_create_str(string, flags)
static int l_enet_packet_create_str(lua_State *L) {
    size_t data_len;
    const char *data = luaL_checklstring(L, 1, &data_len);
    enet_uint32 flags = (enet_uint32)luaL_optinteger(L, 2, ENET_PACKET_FLAG_RELIABLE);

    ENetPacket *packet = enet_packet_create(data, data_len, flags);
    if (packet == NULL) {
        fprintf(stderr, "enet.packet_create_str: Failed to create packet\n");
        lua_pushnil(L);
    } else {
        ENetPacket **ud = push_enet_packet(L);
        *ud = packet;
    }
    return 1;
}

// enet.packet_create_table(table, flags)
static int l_enet_packet_create_table(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    enet_uint32 flags = (enet_uint32)luaL_optinteger(L, 2, ENET_PACKET_FLAG_RELIABLE);

    luaL_Buffer b;
    luaL_buffinit(L, &b);
    luaL_addstring(&b, TABLE_PACKET_PREFIX); // Add prefix to identify table packets
    lua_pushnil(L); // Start table iteration
    while (lua_next(L, 1)) {
        // Key at -2, value at -1
        if (lua_isstring(L, -2)) {
            size_t key_len;
            const char *key = lua_tolstring(L, -2, &key_len);
            luaL_addlstring(&b, key, key_len);
            luaL_addstring(&b, "=");
        } else {
            luaL_error(L, "enet.packet_create_table: Table keys must be strings");
            return 0;
        }
        if (lua_isstring(L, -1) || lua_isnumber(L, -1)) {
            size_t val_len;
            const char *val = lua_tolstring(L, -1, &val_len);
            luaL_addlstring(&b, val, val_len);
        } else {
            luaL_error(L, "enet.packet_create_table: Table values must be strings or numbers");
            return 0;
        }
        luaL_addstring(&b, ";");
        lua_pop(L, 1); // Pop value, keep key for next iteration
    }
    luaL_pushresult(&b); // Finalize buffer and push string to stack
    size_t data_len;
    const char *data = lua_tolstring(L, -1, &data_len); // Get final string
    ENetPacket *packet = enet_packet_create(data, data_len, flags);
    lua_pop(L, 1); // Remove buffer string from stack
    if (packet == NULL) {
        fprintf(stderr, "enet.packet_create_table: Failed to create packet\n");
        lua_pushnil(L);
    } else {
        ENetPacket **ud = push_enet_packet(L);
        *ud = packet;
    }
    return 1;
}

// enet.packet_destroy(packet)
static int l_enet_packet_destroy(lua_State *L) {
    ENetPacket **packet = (ENetPacket**)luaL_checkudata(L, 1, ENET_PACKET_MT);
    if (packet && *packet) {
        enet_packet_destroy(*packet);
        *packet = NULL;
    }
    return 0;
}

// enet.packet_data(packet)
static int l_enet_packet_data(lua_State *L) {
    ENetPacket **packet = (ENetPacket**)luaL_checkudata(L, 1, ENET_PACKET_MT);
    if (!packet || !*packet || !(*packet)->data) {
        fprintf(stderr, "enet.packet_data: Invalid packet or no data\n");
        lua_pushnil(L);
        return 1;
    }

    // Check if packet is a table (starts with TABLE_PACKET_PREFIX)
    size_t prefix_len = strlen(TABLE_PACKET_PREFIX);
    if ((*packet)->dataLength >= prefix_len &&
        strncmp((const char*)(*packet)->data, TABLE_PACKET_PREFIX, prefix_len) == 0) {
        // Parse as table
        lua_newtable(L);
        const char *data = (const char*)(*packet)->data + prefix_len;
        size_t data_len = (*packet)->dataLength - prefix_len;
        const char *p = data;
        const char *end = data + data_len;
        while (p < end) {
            const char *key_start = p;
            while (p < end && *p != '=') p++;
            if (p >= end || *p != '=') break;
            size_t key_len = p - key_start;
            p++;
            const char *val_start = p;
            while (p < end && *p != ';') p++;
            size_t val_len = p - val_start;
            lua_pushlstring(L, key_start, key_len);
            lua_pushlstring(L, val_start, val_len);
            lua_settable(L, -3);
            if (p < end && *p == ';') p++;
        }
    } else {
        // Return as string
        lua_pushlstring(L, (const char*)(*packet)->data, (*packet)->dataLength);
    }
    return 1;
}

// enet.peer_send(peer, channelID, packet)
static int l_enet_peer_send(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    enet_uint8 channelID = (enet_uint8)luaL_checkinteger(L, 2);
    ENetPacket **packet = (ENetPacket**)luaL_checkudata(L, 3, ENET_PACKET_MT);

    if (!peer || !*peer || !packet || !*packet) {
        fprintf(stderr, "enet.peer_send: Invalid peer or packet\n");
        lua_pushinteger(L, -1);
        return 1;
    }

    int result = enet_peer_send(*peer, channelID, *packet);
    if (result >= 0) {
        *packet = NULL;
    }
    lua_pushinteger(L, result);
    return 1;
}

// enet.host_broadcast(host, channelID, packet)
static int l_enet_host_broadcast(lua_State *L) {
    ENetHost **host = (ENetHost**)luaL_checkudata(L, 1, ENET_HOST_MT);
    enet_uint8 channelID = (enet_uint8)luaL_checkinteger(L, 2);
    ENetPacket **packet = (ENetPacket**)luaL_checkudata(L, 3, ENET_PACKET_MT);

    if (!host || !*host || !packet || !*packet) {
        fprintf(stderr, "enet.host_broadcast: Invalid host or packet\n");
        lua_pushinteger(L, -1);
        return 1;
    }

    enet_host_broadcast(*host, channelID, *packet);
    *packet = NULL;
    lua_pushinteger(L, 0);
    return 1;
}

// enet.host_flush(host)
static int l_enet_host_flush(lua_State *L) {
    ENetHost **host = (ENetHost**)luaL_checkudata(L, 1, ENET_HOST_MT);
    if (!host || !*host) {
        fprintf(stderr, "enet.host_flush: Invalid host\n");
        lua_pushinteger(L, -1);
        return 1;
    }
    enet_host_flush(*host);
    lua_pushinteger(L, 0);
    return 1;
}

// enet.peer_reset(peer)
static int l_enet_peer_reset(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        fprintf(stderr, "enet.peer_reset: Invalid peer\n");
        lua_pushnil(L);
        return 1;
    }
    enet_peer_reset(*peer);
    *peer = NULL;
    return 0;
}

// enet.peer_disconnect(peer, [data])
static int l_enet_peer_disconnect(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        fprintf(stderr, "enet.peer_disconnect: Invalid peer\n");
        lua_pushnil(L);
        return 1;
    }
    enet_uint32 data = (enet_uint32)luaL_optinteger(L, 2, 0);
    enet_peer_disconnect(*peer, data);
    return 0;
}

// enet.peer_disconnect_later(peer, [data])
static int l_enet_peer_disconnect_later(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        fprintf(stderr, "enet.peer_disconnect_later: Invalid peer\n");
        lua_pushnil(L);
        return 1;
    }
    enet_uint32 data = (enet_uint32)luaL_optinteger(L, 2, 0);
    enet_peer_disconnect_later(*peer, data);
    return 0;
}

// enet.peer_disconnect_now(peer, [data])
static int l_enet_peer_disconnect_now(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        fprintf(stderr, "enet.peer_disconnect_now: Invalid peer\n");
        lua_pushnil(L);
        return 1;
    }
    enet_uint32 data = (enet_uint32)luaL_optinteger(L, 2, 0);
    enet_peer_disconnect_now(*peer, data);
    return 0;
}

// enet.peer_set_data(peer, data)
static int l_enet_peer_set_data(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        fprintf(stderr, "enet.peer_set_data: Invalid peer\n");
        lua_pushnil(L);
        return 1;
    }
    if ((*peer)->data) {
        free((*peer)->data);
        (*peer)->data = NULL;
    }
    if (!lua_isnil(L, 2)) {
        const char *data = luaL_checkstring(L, 2);
        (*peer)->data = strdup(data);
        if (!(*peer)->data) {
            fprintf(stderr, "enet.peer_set_data: Failed to allocate memory for data\n");
            lua_pushnil(L);
            return 1;
        }
    }
    return 0;
}

// enet.peer_get_data(peer) -> string or nil
static int l_enet_peer_get_data(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer || !(*peer)->data) {
        fprintf(stderr, "enet.peer_get_data: Peer invalid or no data (peer=%p, data=%p)\n",
                (void*)peer, (void*)(peer ? (*peer)->data : NULL));
        lua_pushnil(L);
        return 1;
    }
    lua_pushstring(L, (const char*)(*peer)->data);
    return 1;
}

// enet.peer_get_connect_id(peer) -> integer
static int l_enet_peer_get_connect_id(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        fprintf(stderr, "enet.peer_get_connect_id: Invalid peer\n");
        lua_pushnil(L);
        return 1;
    }
    lua_pushinteger(L, (lua_Integer)(*peer)->connectID);
    return 1;
}

// enet.peer_get_incoming_peer_id(peer) -> integer
static int l_enet_peer_get_incoming_peer_id(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        fprintf(stderr, "enet.peer_get_incoming_peer_id: Invalid peer\n");
        lua_pushnil(L);
        return 1;
    }
    lua_pushinteger(L, (lua_Integer)(*peer)->incomingPeerID);
    return 1;
}

// enet.peer_get_state(peer) -> integer
static int l_enet_peer_get_state(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        fprintf(stderr, "enet.peer_get_state: Invalid peer\n");
        lua_pushnil(L);
        return 1;
    }
    lua_pushinteger(L, (lua_Integer)(*peer)->state);
    return 1;
}

// enet.peer_throttle_configure(peer, interval, acceleration, deceleration)
static int l_enet_peer_throttle_configure(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        fprintf(stderr, "enet.peer_throttle_configure: Invalid peer\n");
        lua_pushnil(L);
        return 1;
    }
    enet_uint32 interval = (enet_uint32)luaL_checkinteger(L, 2);
    enet_uint32 acceleration = (enet_uint32)luaL_checkinteger(L, 3);
    enet_uint32 deceleration = (enet_uint32)luaL_checkinteger(L, 4);
    enet_peer_throttle_configure(*peer, interval, acceleration, deceleration);
    return 0;
}

// enet.peer_ping(peer)
static int l_enet_peer_ping(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        fprintf(stderr, "enet.peer_ping: Invalid peer\n");
        lua_pushnil(L);
        return 1;
    }
    enet_peer_ping(*peer);
    return 0;
}

// enet.peer_timeout(peer, timeoutLimit, timeoutMinimum, timeoutMaximum)
static int l_enet_peer_timeout(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        fprintf(stderr, "enet.peer_timeout: Invalid peer\n");
        lua_pushnil(L);
        return 1;
    }
    enet_uint32 timeoutLimit = (enet_uint32)luaL_checkinteger(L, 2);
    enet_uint32 timeoutMinimum = (enet_uint32)luaL_checkinteger(L, 3);
    enet_uint32 timeoutMaximum = (enet_uint32)luaL_checkinteger(L, 4);
    enet_peer_timeout(*peer, timeoutLimit, timeoutMinimum, timeoutMaximum);
    return 0;
}

// Garbage collection for ENetHost
static int enet_host_gc(lua_State *L) {
    ENetHost **host = (ENetHost**)luaL_checkudata(L, 1, ENET_HOST_MT);
    if (host && *host) {
        enet_host_destroy(*host);
        *host = NULL;
    }
    return 0;
}

// Garbage collection for ENetPacket
static int enet_packet_gc(lua_State *L) {
    ENetPacket **packet = (ENetPacket**)luaL_checkudata(L, 1, ENET_PACKET_MT);
    if (packet && *packet) {
        enet_packet_destroy(*packet);
        *packet = NULL;
    }
    return 0;
}

// Garbage collection for ENetPeer
static int enet_peer_gc(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (peer && *peer) {
        if ((*peer)->data) {
            free((*peer)->data);
            (*peer)->data = NULL;
        }
        lua_getfield(L, LUA_REGISTRYINDEX, ENET_PEER_REGISTRY);
        if (!lua_isnil(L, -1)) {
            lua_pushlightuserdata(L, (void*)*peer);
            lua_pushnil(L);
            lua_settable(L, -3);
        }
        lua_pop(L, 1);
        *peer = NULL;
    }
    return 0;
}

// Module-level functions
static const luaL_Reg enet_funcs[] = {
    {"initialize", l_enet_initialize},
    {"deinitialize", l_enet_deinitialize},
    {"host_create", l_enet_host_create},
    {"host_destroy", l_enet_host_destroy},
    {"host_connect", l_enet_host_connect},
    {"host_service", l_enet_host_service},
    {"host_check_events", l_enet_host_check_events},
    {"host_flush", l_enet_host_flush},
    {"host_bandwidth_limit", l_enet_host_bandwidth_limit},
    {"packet_create", l_enet_packet_create},
    {"packet_create_str", l_enet_packet_create_str},
    {"packet_create_table", l_enet_packet_create_table},
    {"packet_destroy", l_enet_packet_destroy},
    {"packet_data", l_enet_packet_data},
    {"peer_send", l_enet_peer_send},
    {"peer_reset", l_enet_peer_reset},
    {"peer_disconnect", l_enet_peer_disconnect},
    {"peer_disconnect_later", l_enet_peer_disconnect_later},
    {"peer_disconnect_now", l_enet_peer_disconnect_now},
    {"peer_set_data", l_enet_peer_set_data},
    {"peer_get_data", l_enet_peer_get_data},
    {"peer_get_connect_id", l_enet_peer_get_connect_id},
    {"peer_get_incoming_peer_id", l_enet_peer_get_incoming_peer_id},
    {"peer_get_state", l_enet_peer_get_state},
    {"peer_throttle_configure", l_enet_peer_throttle_configure},
    {"peer_ping", l_enet_peer_ping},
    {"peer_timeout", l_enet_peer_timeout},
    {"host_broadcast", l_enet_host_broadcast},
    {NULL, NULL}
};

// Metatable for ENetHost
static const luaL_Reg enet_host_mt[] = {
    {"__gc", enet_host_gc},
    {"destroy", l_enet_host_destroy},
    {"flush", l_enet_host_flush},
    {"check_events", l_enet_host_check_events},
    {"broadcast", l_enet_host_broadcast},
    {"bandwidth_limit", l_enet_host_bandwidth_limit},
    {NULL, NULL}
};

// Metatable for ENetPeer
static const luaL_Reg enet_peer_mt[] = {
    {"__gc", enet_peer_gc},
    {"send", l_enet_peer_send},
    {"reset", l_enet_peer_reset},
    {"disconnect", l_enet_peer_disconnect},
    {"disconnect_later", l_enet_peer_disconnect_later},
    {"disconnect_now", l_enet_peer_disconnect_now},
    {"set_data", l_enet_peer_set_data},
    {"get_data", l_enet_peer_get_data},
    {"get_connect_id", l_enet_peer_get_connect_id},
    {"get_incoming_peer_id", l_enet_peer_get_incoming_peer_id},
    {"get_state", l_enet_peer_get_state},
    {"throttle_configure", l_enet_peer_throttle_configure},
    {"ping", l_enet_peer_ping},
    {"timeout", l_enet_peer_timeout},
    {NULL, NULL}
};

// Metatable for ENetPacket
static const luaL_Reg enet_packet_mt[] = {
    {"__gc", enet_packet_gc},
    {"destroy", l_enet_packet_destroy},
    {NULL, NULL}
};

int luaopen_module_enet(lua_State *L) {
    luaL_newmetatable(L, ENET_HOST_MT);
    luaL_setfuncs(L, enet_host_mt, 0);
    lua_pop(L, 1);

    luaL_newmetatable(L, ENET_PEER_MT);
    luaL_setfuncs(L, enet_peer_mt, 0);
    lua_pop(L, 1);

    luaL_newmetatable(L, ENET_PACKET_MT);
    luaL_setfuncs(L, enet_packet_mt, 0);
    lua_pop(L, 1);

    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, ENET_PEER_REGISTRY);

    lua_newtable(L);
    luaL_setfuncs(L, enet_funcs, 0);

    lua_pushinteger(L, ENET_EVENT_TYPE_NONE);
    lua_setfield(L, -2, "EVENT_TYPE_NONE");
    lua_pushinteger(L, ENET_EVENT_TYPE_CONNECT);
    lua_setfield(L, -2, "EVENT_TYPE_CONNECT");
    lua_pushinteger(L, ENET_EVENT_TYPE_DISCONNECT);
    lua_setfield(L, -2, "EVENT_TYPE_DISCONNECT");
    lua_pushinteger(L, ENET_EVENT_TYPE_RECEIVE);
    lua_setfield(L, -2, "EVENT_TYPE_RECEIVE");
    lua_pushinteger(L, ENET_EVENT_TYPE_DISCONNECT_TIMEOUT);
    lua_setfield(L, -2, "EVENT_TYPE_DISCONNECT_TIMEOUT");

    lua_pushinteger(L, ENET_PACKET_FLAG_RELIABLE);
    lua_setfield(L, -2, "PACKET_FLAG_RELIABLE");
    lua_pushinteger(L, ENET_PACKET_FLAG_UNSEQUENCED);
    lua_setfield(L, -2, "PACKET_FLAG_UNSEQUENCED");
    lua_pushinteger(L, ENET_PACKET_FLAG_NO_ALLOCATE);
    lua_setfield(L, -2, "PACKET_FLAG_NO_ALLOCATE");
    lua_pushinteger(L, ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
    lua_setfield(L, -2, "PACKET_FLAG_UNRELIABLE_FRAGMENT");
    lua_pushinteger(L, ENET_PACKET_FLAG_UNTHROTTLED);
    lua_setfield(L, -2, "PACKET_FLAG_UNTHROTTLED");
    lua_pushinteger(L, ENET_PACKET_FLAG_SENT);
    lua_setfield(L, -2, "PACKET_FLAG_SENT");

    lua_pushinteger(L, ENET_VERSION_MAJOR);
    lua_setfield(L, -2, "VERSION_MAJOR");
    lua_pushinteger(L, ENET_VERSION_MINOR);
    lua_setfield(L, -2, "VERSION_MINOR");
    lua_pushinteger(L, ENET_VERSION_PATCH);
    lua_setfield(L, -2, "VERSION_PATCH");

    lua_pushinteger(L, ENET_PEER_STATE_DISCONNECTED);
    lua_setfield(L, -2, "PEER_STATE_DISCONNECTED");
    lua_pushinteger(L, ENET_PEER_STATE_CONNECTING);
    lua_setfield(L, -2, "PEER_STATE_CONNECTING");
    lua_pushinteger(L, ENET_PEER_STATE_ACKNOWLEDGING_CONNECT);
    lua_setfield(L, -2, "PEER_STATE_ACKNOWLEDGING_CONNECT");
    lua_pushinteger(L, ENET_PEER_STATE_CONNECTION_PENDING);
    lua_setfield(L, -2, "PEER_STATE_CONNECTION_PENDING");
    lua_pushinteger(L, ENET_PEER_STATE_CONNECTION_SUCCEEDED);
    lua_setfield(L, -2, "PEER_STATE_CONNECTION_SUCCEEDED");
    lua_pushinteger(L, ENET_PEER_STATE_CONNECTED);
    lua_setfield(L, -2, "PEER_STATE_CONNECTED");
    lua_pushinteger(L, ENET_PEER_STATE_DISCONNECT_LATER);
    lua_setfield(L, -2, "PEER_STATE_DISCONNECT_LATER");
    lua_pushinteger(L, ENET_PEER_STATE_DISCONNECTING);
    lua_setfield(L, -2, "PEER_STATE_DISCONNECTING");
    lua_pushinteger(L, ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT);
    lua_setfield(L, -2, "PEER_STATE_ACKNOWLEDGING_DISCONNECT");
    lua_pushinteger(L, ENET_PEER_STATE_ZOMBIE);
    lua_setfield(L, -2, "PEER_STATE_ZOMBIE");

    return 1;
}

void lua_enet_init(lua_State *L) {
    (void)L;
}