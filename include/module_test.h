// module_test.h
#ifndef MODULE_TEST_H
#define MODULE_TEST_H

#include <lua.h>

int luaopen_module_test(lua_State *L);

#endif

/*
-- main.lua
local test = require("module_test")

-- Test the module
print("Initializing module...")
if test.init() then
    print("Module initialized successfully")
end

print("Getting number: " .. test.get_number())

-- This would cause an error if uncommented
-- print(test.get_number()) -- Error: Module not initialized
*/