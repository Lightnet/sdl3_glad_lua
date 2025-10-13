#include "module_enet.h"
#include <lauxlib.h>
#include <lualib.h>
// #include <ENet/ENet.h>  // Assuming ENet is installed and linked properly
#include <enet.h>
#include <string.h>
#include <stdlib.h>

// Metatable names
#define ENET_HOST_MT "ENetHost"
#define ENET_PEER_MT "ENetPeer"
#define ENET_PACKET_MT "ENetPacket"

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
    if (address) free(address);  // Clean up if we allocated it
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
    }
    return 1;
}

// enet.host_service(host, timeout)
static int l_enet_host_service(lua_State *L) {
    ENetHost **host = (ENetHost**)luaL_checkudata(L, 1, ENET_HOST_MT);
    if (!host || !*host) {
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
            ENetPeer **ud = push_enet_peer(L);
            *ud = event.peer;
            lua_setfield(L, -2, "peer");
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

// enet.packet_create(data, flags)
static int l_enet_packet_create(lua_State *L) {
    size_t data_len;
    const char *data = luaL_checklstring(L, 1, &data_len);
    enet_uint32 flags = (enet_uint32)luaL_optinteger(L, 2, ENET_PACKET_FLAG_RELIABLE);

    ENetPacket *packet = enet_packet_create(data, data_len, flags);
    if (packet == NULL) {
        lua_pushnil(L);
    } else {
        ENetPacket **ud = push_enet_packet(L);
        *ud = packet;
        // Note: Packets are reference-counted by ENet; no need for manual registry ref unless custom GC logic requires it
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
    if (packet && *packet && (*packet)->data) {
        lua_pushlstring(L, (const char*)(*packet)->data, (*packet)->dataLength);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// enet.peer_send(peer, channelID, packet)
static int l_enet_peer_send(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    enet_uint8 channelID = (enet_uint8)luaL_checkinteger(L, 2);
    ENetPacket **packet = (ENetPacket**)luaL_checkudata(L, 3, ENET_PACKET_MT);

    if (!peer || !*peer || !packet || !*packet) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int result = enet_peer_send(*peer, channelID, *packet);
    if (result >= 0) {
        // Ownership transferred to ENet; clear userdata to avoid double-free
        *packet = NULL;
    }
    lua_pushinteger(L, result);
    return 1;
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

static int enet_peer_gc(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (peer && *peer) {
        if ((*peer)->data) {
            free((*peer)->data); // Free allocated string
            (*peer)->data = NULL;
        }
        *peer = NULL; // Peers are destroyed by host
    }
    return 0;
}

// enet.peer_reset(peer)
static int l_enet_peer_reset(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        lua_pushnil(L);
        return 1;
    }
    enet_peer_reset(*peer);
    *peer = NULL;  // Nullify to prevent accidental reuse
    return 0;
}

// enet.peer_disconnect(peer, [data])
static int l_enet_peer_disconnect(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        lua_pushnil(L);
        return 1;
    }
    enet_uint32 data = (enet_uint32)luaL_optinteger(L, 2, 0);
    enet_peer_disconnect(*peer, data);
    return 0;
}

// enet.host_broadcast(host, channelID, packet)
static int l_enet_host_broadcast(lua_State *L) {
    ENetHost **host = (ENetHost**)luaL_checkudata(L, 1, ENET_HOST_MT);
    enet_uint8 channelID = (enet_uint8)luaL_checkinteger(L, 2);
    ENetPacket **packet = (ENetPacket**)luaL_checkudata(L, 3, ENET_PACKET_MT);

    if (!host || !*host || !packet || !*packet) {
        lua_pushinteger(L, -1);
        return 1;
    }

    enet_host_broadcast(*host, channelID, *packet);
    *packet = NULL; // Ownership transferred to ENet
    lua_pushinteger(L, 0);
    return 1;
}

// enet.peer_disconnect_later(peer, [data])
static int l_enet_peer_disconnect_later(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        lua_pushnil(L);
        return 1;
    }
    enet_uint32 data = (enet_uint32)luaL_optinteger(L, 2, 0);
    enet_peer_disconnect_later(*peer, data);
    return 0;
}

// enet.peer_set_data(peer, data)
static int l_enet_peer_set_data(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer) {
        lua_pushnil(L);
        return 1;
    }
    // Free any existing data (assuming it's a string allocated by us)
    if ((*peer)->data) {
        free((*peer)->data);
        (*peer)->data = NULL;
    }
    if (!lua_isnil(L, 2)) {
        const char *data = luaL_checkstring(L, 2);
        (*peer)->data = strdup(data); // Allocate and copy string
    }
    return 0;
}

// enet.peer_get_data(peer) -> string or nil
static int l_enet_peer_get_data(lua_State *L) {
    ENetPeer **peer = (ENetPeer**)luaL_checkudata(L, 1, ENET_PEER_MT);
    if (!peer || !*peer || !(*peer)->data) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushstring(L, (const char*)(*peer)->data);
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

// Lua: enet.peer_get_connect_id(peer) -> integer
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


// Module-level functions
static const luaL_Reg enet_funcs[] = {
    {"initialize", l_enet_initialize},
    {"deinitialize", l_enet_deinitialize},
    {"host_create", l_enet_host_create},
    {"host_destroy", l_enet_host_destroy},
    {"host_connect", l_enet_host_connect},
    {"host_service", l_enet_host_service},
    {"host_flush", l_enet_host_flush}, // Add host_flush
    {"packet_create", l_enet_packet_create},
    {"packet_destroy", l_enet_packet_destroy},
    {"packet_data", l_enet_packet_data},
    {"peer_send", l_enet_peer_send},
    {"peer_reset", l_enet_peer_reset},
    {"peer_disconnect", l_enet_peer_disconnect},
    {"peer_set_data", l_enet_peer_set_data},   // Add set_data
    {"peer_get_data", l_enet_peer_get_data},   // Add get_data
    {"peer_get_connect_id", l_enet_peer_get_connect_id}, // Add get_connect_id

    {"host_broadcast", l_enet_host_broadcast},         // Add broadcast

    {"disconnect_later", l_enet_peer_disconnect_later}, // Add disconnect_later
    {NULL, NULL}
};

// Metatable for ENetHost
static const luaL_Reg enet_host_mt[] = {
    {"__gc", enet_host_gc},
    {"destroy", l_enet_host_destroy},  // Optional: Expose destroy as method
    {"host_flush", l_enet_host_flush}, // Add host_flush
    {"broadcast", l_enet_host_broadcast}, // Add broadcast method
    {NULL, NULL}
};

// Metatable for ENetPeer
static const luaL_Reg enet_peer_mt[] = {
    {"__gc", enet_peer_gc},
    {"send", l_enet_peer_send},  // Optional: Expose send as 
    {"reset", l_enet_peer_reset},         // Add reset method
    {"set_data", l_enet_peer_set_data},   // Add set_data
    {"get_data", l_enet_peer_get_data},   // Add get_data
    {"peer_get_connect_id", l_enet_peer_get_connect_id}, // Add get_connect_id
    {"disconnect", l_enet_peer_disconnect}, // Add disconnect method
    {"disconnect_later", l_enet_peer_disconnect_later}, // Add disconnect_later
    {NULL, NULL}
};

// Metatable for ENetPacket
static const luaL_Reg enet_packet_mt[] = {
    {"__gc", enet_packet_gc},
    {"destroy", l_enet_packet_destroy},  // Optional: Expose destroy as method
    {NULL, NULL}
};

int luaopen_module_enet(lua_State *L) {
    // Create and register metatables
    luaL_newmetatable(L, ENET_HOST_MT);
    luaL_setfuncs(L, enet_host_mt, 0);
    lua_pop(L, 1);

    luaL_newmetatable(L, ENET_PEER_MT);
    luaL_setfuncs(L, enet_peer_mt, 0);
    lua_pop(L, 1);

    luaL_newmetatable(L, ENET_PACKET_MT);
    luaL_setfuncs(L, enet_packet_mt, 0);
    lua_pop(L, 1);

    // Create the module table
    lua_newtable(L);
    luaL_setfuncs(L, enet_funcs, 0);

    // Add ENet event type constants
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

    // Add common packet flags
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

    //enet version
    lua_pushinteger(L, ENET_VERSION_MAJOR);
    lua_setfield(L, -2, "VERSION_MAJOR");
    lua_pushinteger(L, ENET_VERSION_MINOR);
    lua_setfield(L, -2, "VERSION_MINOR");
    lua_pushinteger(L, ENET_VERSION_PATCH);
    lua_setfield(L, -2, "VERSION_PATCH");

    return 1;
}

// Optional init function (if needed elsewhere; not called in main.c)
void lua_enet_init(lua_State *L) {
    // No global state needed; everything is per-call
    (void)L;
}