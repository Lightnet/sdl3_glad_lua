


# sample test
```lua
local test = require("module_test")
test.set_window({
    width=500,
    height=500
})
test.set_window({
    height=500,
    width=500
})
```

```lua
local function removeFirstMatch(tbl, value)
  for i, v in ipairs(tbl) do
    if v == value then
      table.remove(tbl, i)
      return
    end
  end
end

-- Example usage
local items = { "sword", "shield", "potion", "sword" }
removeFirstMatch(items, "sword")

-- Prints: "shield", "potion", "sword"
for i, v in ipairs(items) do
  print(i, v)
end
```

```lua
local function removeAllMatches(tbl, value)
  for i = #tbl, 1, -1 do
    if tbl[i] == value then
      table.remove(tbl, i)
    end
  end
end

-- Example usage
local items = { "sword", "shield", "potion", "sword" }
removeAllMatches(items, "sword")

-- Prints: "shield", "potion"
for i, v in ipairs(items) do
  print(i, v)
end
```