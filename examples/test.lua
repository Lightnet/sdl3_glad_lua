-- main.lua
local test = require("module_test")

-- Test the module
print("Initializing module...")
if test.init() then
    print("Module initialized successfully")
end

-- Test number
print("\nTesting number:")
test.set_number(42.5)
print("Number set to 42.5, got: " .. test.get_number())

-- Test string
print("\nTesting string:")
test.set_string("Hello, Lua!")
print("String set to 'Hello, Lua!', got: " .. (test.get_string() or "nil"))

-- Test integer
print("\nTesting integer:")
test.set_int(100)
print("Integer set to 100, got: " .. test.get_int())

-- Test boolean
print("\nTesting boolean:")
test.set_bool(true)
print("Boolean set to true, got: " .. tostring(test.get_bool()))

test.set_bool(false)
print("Boolean set to false, got: " .. tostring(test.get_bool()))

-- Test call_foo
print("\nTesting call_foo:")
test.call_foo()

-- Test error case
print("\nTesting error case:")
test.init() -- Reset to clear state
print("Trying to get number before setting...")
local status, err = pcall(test.get_number)
if not status then
    print("Error caught: " .. err)
end