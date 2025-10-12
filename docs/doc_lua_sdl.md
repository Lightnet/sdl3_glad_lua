
# SDL Lua Module Functions

## Function: sdl.init

Description: Initializes SDL with the specified subsystems (e.g., video, events). Returns a boolean indicating success and an error message on failure.

Parameters:
- subsystems (integer): Bitmask of SDL subsystems to initialize (e.g., sdl.INIT_VIDEO, sdl.INIT_EVENTS, or sdl.INIT_NONE).

Returns:
- success (boolean): true if initialization succeeded, false otherwise.
- err_msg (string, optional): Error message if initialization failed.

Example:

lua
```lua
local ok, err = sdl.init(sdl.INIT_VIDEO + sdl.INIT_EVENTS)
if not ok then
    print("Failed to initialize SDL: " .. err)
end
```

---

## Function: sdl.init_window

Description: Creates an SDL window with the specified title, dimensions, and flags. Sets default OpenGL attributes if SDL_WINDOW_OPENGL is used. Returns the window handle or an error message on failure.

Parameters:
- title (string): Window title.
- width (integer): Window width in pixels.
- height (integer): Window height in pixels.
- flags (integer): Window flags (e.g., sdl.WINDOW_OPENGL, sdl.WINDOW_RESIZABLE, sdl.WINDOW_HIGH_PIXEL_DENSITY).

Returns:
- window (lightuserdata): SDL window handle, or nil on failure.
- err_msg (string, optional): Error message if window creation failed.

Example:

lua
```lua
local window, err = sdl.init_window("My Game", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
if not window then
    print("Failed to create window: " .. err)
end
```

---

## Function: sdl.poll_events

Description: Polls for SDL events and returns a table of events. Each event is a table with a type field and additional fields based on the event type (e.g., key code, mouse coordinates, window size).

Parameters: None.

Returns:
- events (table): Array of event tables, each with:
    - type (integer): Event type (e.g., sdl.EVENT_QUIT, sdl.EVENT_KEY_DOWN).
    - For EVENT_KEY_DOWN: key (integer) for the key code.
    - For EVENT_MOUSE_BUTTON_DOWN: button (integer), x (integer), y (integer).
    - For EVENT_WINDOW_RESIZED: width (integer), height (integer).

Example:

lua

```lua
local events = sdl.poll_events()
for i, event in ipairs(events) do
    if event.type == sdl.EVENT_QUIT then
        print("Window closed")
    elseif event.type == sdl.EVENT_KEY_DOWN then
        print("Key pressed: " .. event.key)
    end
end
```

---

## Function: sdl.poll_events_ig

Description: Similar to sdl.poll_events, but also processes events for ImGui integration. Returns the same event table structure.

Parameters: None.

Returns:
- events (table): Same as sdl.poll_events.

Example:

lua

```lua
local events = sdl.poll_events_ig()
for i, event in ipairs(events) do
    if event.type == sdl.EVENT_MOUSE_BUTTON_DOWN then
        print("Mouse clicked at: " .. event.x .. ", " .. event.y)
    end
end
```

---

## Function: sdl.quit

Description: Destroys the window (if stored in the global sdl_window) and shuts down SDL, cleaning up resources.

Parameters: None.

Returns: None.

Example:

lua
```lua
sdl.quit() -- Clean up SDL and destroy window
```

---

## Function: sdl.gl_reset_attribute

Description: Resets all OpenGL context attributes to their default values.

Parameters: None.

Returns:
- success (boolean): Always true.

Example:

lua
```lua
sdl.gl_reset_attribute() -- Reset OpenGL attributes before setting new ones
```

---

## Function: sdl.gl_set_attribute

Description: Sets an OpenGL context attribute (e.g., red size, double buffering). Returns a boolean indicating success and an error message on failure.

Parameters:
- attr (integer): OpenGL attribute (e.g., sdl.GL_RED_SIZE, sdl.GL_DOUBLEBUFFER).
- value (integer): Value to set for the attribute.

Returns:
- success (boolean): true if the attribute was set, false otherwise.
- err_msg (string, optional): Error message if setting failed.

Example:

lua
```lua
local ok, err = sdl.gl_set_attribute(sdl.GL_DOUBLEBUFFER, 1)
if not ok then
    print("Failed to set GL attribute: " .. err)
end
```

---

## Function: sdl.gl_get_attribute

Description: Retrieves the value of an OpenGL context attribute. Returns the value or an error message on failure.

Parameters:
- attr (integer): OpenGL attribute to query (e.g., sdl.GL_RED_SIZE).

Returns:
- value (integer): Attribute value, or nil on failure.
- err_msg (string, optional): Error message if retrieval failed.

Example:

lua
```lua
local value, err = sdl.gl_get_attribute(sdl.GL_DEPTH_SIZE)
if value then
    print("Depth size: " .. value)
else
    print("Error: " .. err)
end
```

---

## Function: sdl.gl_swap_window

Description: Swaps the OpenGL buffers for the specified window, updating the display.

Parameters:
- window (lightuserdata): SDL window handle.

Returns:
- nil, err_msg (string): If the window is invalid.

Example:

lua
```lua
sdl.gl_swap_window(window) -- Swap buffers to update the display
```

---

## Function: sdl.get_primary_display

Description: Returns the ID of the primary display.

Parameters: None.

Returns:
- display_id (integer): ID of the primary display, or nil on failure.
- err_msg (string, optional): Error message if retrieval failed.

Example:

lua
```lua
local display_id, err = sdl.get_primary_display()
if display_id then
    print("Primary display ID: " .. display_id)
else
    print("Error: " .. err)
end
```

---

## Function: sdl.get_display_content_scale

Description: Returns the content scale (e.g., for high-DPI displays) for the specified display.

Parameters:
- display_id (integer): Display ID to query.

Returns:
- scale (number): Content scale factor, or nil on failure.
- err_msg (string, optional): Error message if retrieval failed.

Example:

lua
```lua
local scale, err = sdl.get_display_content_scale(display_id)
if scale then
    print("Display scale: " .. scale)
else
    print("Error: " .. err)
end
```

---

## Function: sdl.set_window_position

Description: Sets the position of the specified window.

Parameters:
- window (lightuserdata): SDL window handle.
- x (integer): X-coordinate (e.g., sdl.WINDOWPOS_CENTERED).
- y (integer): Y-coordinate (e.g., sdl.WINDOWPOS_CENTERED).

Returns:
- success (boolean): true if successful, false otherwise.
- err_msg (string, optional): Error message if setting failed.

Example:

lua
```lua
local ok, err = sdl.set_window_position(window, sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED)
if not ok then
    print("Failed to set window position: " .. err)
end
```

---

## Function: sdl.show_window

Description: Shows the specified window.

Parameters:
- window (lightuserdata): SDL window handle.

Returns:
- nil, err_msg (string): If the window is invalid.

Example:

lua
```lua
sdl.show_window(window) -- Show the window
```

---

## Function: sdl.get_window_flags

Description: Returns the flags of the specified window.

Parameters:
- window (lightuserdata): SDL window handle.

Returns:
- flags (integer): Window flags (e.g., sdl.WINDOW_MINIMIZED), or nil on failure.
- err_msg (string, optional): Error message if retrieval failed.

Example:

lua
```lua
local flags, err = sdl.get_window_flags(window)
if flags then
    print("Window flags: " .. flags)
else
    print("Error: " .. err)
end
```

---

## Function: sdl.get_window_size

Description: Returns the size of the specified window.Parameters:
- window (lightuserdata): SDL window handle.

Returns:
- width (integer): Window width in pixels, or nil on failure.
- height (integer): Window height in pixels, or nil on failure.
- err_msg (string, optional): Error message if retrieval failed.

Example:

lua
```lua
local width, height, err = sdl.get_window_size(window)
if width then
    print("Window size: " .. width .. "x" .. height)
else
    print("Error: " .. err)
end
```

---

## Function: sdl.get_window_id

Description: Returns the ID of the specified window.


Parameters:
- window (lightuserdata): SDL window handle.

Returns:
- window_id (integer): Window ID, or nil on failure.
- err_msg (string, optional): Error message if retrieval failed.

Example:

lua
```lua
local window_id, err = sdl.get_window_id(window)
if window_id then
    print("Window ID: " .. window_id)
else
    print("Error: " .. err)
end
```

---

## Function: sdl.delay

Description: Pauses execution for the specified number of milliseconds.

Parameters:
- ms (integer): Delay duration in milliseconds.

Returns: None.Example:

lua
```lua
sdl.delay(100) -- Pause for 100ms
```

---

## Function: sdl.get_current_gl_context

Description: Returns the current OpenGL context.

Parameters: None.

Returns:
- context (lightuserdata): OpenGL context handle, or nil if none exists.

Example:

lua
```lua
local context = sdl.get_current_gl_context()
if context then
    print("Got OpenGL context")
else
    print("No OpenGL context")
end
```

---

# Constants

The module exposes SDL constants directly in the sdl table, including:

- Event Types: EVENT_QUIT, EVENT_WINDOW_RESIZED, EVENT_KEY_DOWN, EVENT_MOUSE_BUTTON_DOWN.
- Window Flags: WINDOW_OPENGL, WINDOW_RESIZABLE, WINDOW_HIGH_PIXEL_DENSITY, WINDOW_MINIMIZED.
- Window Position: WINDOWPOS_CENTERED.
- OpenGL Attributes: GL_RED_SIZE, GL_GREEN_SIZE, GL_BLUE_SIZE, GL_ALPHA_SIZE, GL_DOUBLEBUFFER, GL_DEPTH_SIZE, GL_STENCIL_SIZE, GL_CONTEXT_PROFILE_MASK, GL_CONTEXT_MAJOR_VERSION, GL_CONTEXT_MINOR_VERSION, GL_CONTEXT_PROFILE_CORE, GL_CONTEXT_PROFILE_COMPATIBILITY, GL_CONTEXT_PROFILE_ES, GL_MULTISAMPLEBUFFERS, GL_MULTISAMPLESAMPLES, GL_ACCELERATED_VISUAL, GL_CONTEXT_FLAGS, GL_CONTEXT_DEBUG_FLAG, GL_CONTEXT_FORWARD_COMPATIBLE_FLAG.
- Subsystem Flags: INIT_VIDEO, INIT_EVENTS, INIT_NONE.

Example Usage:

lua
```lua
print(sdl.EVENT_QUIT) -- Prints the SDL_EVENT_QUIT constant value
local flags = sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE
```

---

## Notes
- The module assumes SDL3 and optionally ImGui (cimgui.h) are available.
- Functions handling windows (e.g., sdl.gl_swap_window, sdl.set_window_position) expect a valid window handle as light userdata, typically returned by sdl.init_window.
- Error handling is consistent, with most functions returning nil or false plus an error message on failure.
- The sdl.quit function assumes the window is stored in the global sdl_window, which may require careful Lua-side management.