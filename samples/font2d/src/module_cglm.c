// module_cglm.c
#include "module_cglm.h"
#include <cglm/cglm.h>
#include <lauxlib.h>

// Placeholder: Initialize cglm (no specific initialization needed)
static int cglm_init(lua_State *L) {
    lua_pushboolean(L, 1);
    return 1;
}

static const struct luaL_Reg cglm_lib[] = {
    {"init", cglm_init},
    {NULL, NULL}
};

int luaopen_module_cglm(lua_State *L) {
    luaL_newlib(L, cglm_lib);
    return 1;
}