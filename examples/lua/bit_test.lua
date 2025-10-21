
-- local flags = 1
-- local value = 42

-- local packed = string.char(
--     (flags & 0xFF),     -- First byte: flags (8 bits)
--     (value & 0xFF),     -- Second byte: value (low byte)
--     (value >> 8)        -- Third byte: value (high byte)
-- )

-- print("pack:" , packed)

-- -- Example for coordinates ranging roughly from -524,288 to 524,287
-- -- (Each coordinate fits within 20 bits)
-- local max_coord = 524287

-- -- Coordinates
-- local x = 1234
-- local y = 5678
-- local z = 9101

-- -- Pack the coordinates into a single 64-bit integer
-- -- << 20: shift y 20 bits to the left
-- -- << 40: shift z 40 bits to the left
-- local packed = (x & 0xFFFFF) | ((y & 0xFFFFF) << 20) | ((z & 0xFFFFF) << 40)

-- print("Packed value:", packed)

-- -- Unpack the coordinates
-- local unpacked_x = packed & 0xFFFFF
-- local unpacked_y = (packed >> 20) & 0xFFFFF
-- local unpacked_z = (packed >> 40) & 0xFFFFF

-- print(string.format("Unpacked: x=%d, y=%d, z=%d", unpacked_x, unpacked_y, unpacked_z))

-- -- To handle negative numbers, you can add a zigzag encoding step
-- local function zig_zag_encode(n)
--     return (n << 1) ~ (n >> 63)
-- end

-- local function zig_zag_decode(n)
--     return (n >> 1) ~ (-(n & 1))
-- end

-- local packed_zigzag = zig_zag_encode(x) | (zig_zag_encode(y) << 20) | (zig_zag_encode(z) << 40)
-- print("\nPacked with zigzag:", packed_zigzag)


-- Assume coordinates range from -32768 to 32767
local x = -100
local y = 250
local z = 512

-- Pack the three coordinates into a binary string
local packed_string = string.pack("<sss", x, y, z)

print("Packed string length:", #packed_string)
-- Unpack the coordinates from the binary string
local unpacked_x, unpacked_y, unpacked_z = string.unpack("<sss", packed_string)

print(string.format("Unpacked: x=%d, y=%d, z=%d", unpacked_x, unpacked_y, unpacked_z))