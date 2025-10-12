-- Load required modules
local sdl = require("module_sdl")
local gl = require("module_gl") 
local imgui = require("module_imgui")
local enet = require("module_enet")



-- Initialize SDL with video and events subsystems
local success, err = sdl.init(sdl.INIT_VIDEO + sdl.INIT_EVENTS)
if not success then
    print("SDL init failed: " .. err)
    return
end

-- Create window with OpenGL and resizable flags
local window, err = sdl.init_window("sdl3 lua", 800, 600, sdl.WINDOW_OPENGL + sdl.WINDOW_RESIZABLE)
if not window then
    lua_util.log("Failed to create window: " .. err)
    sdl.quit()
    return
end

-- Initialize OpenGL
local gl_context, success, err = gl.init(window)
if not success then
    lua_util.log("Failed to initialize OpenGL: " .. err)
    gl.destroy()
    sdl.quit()
    return
end
print("success: " .. tostring(success))
print("gl_context: " .. tostring(gl_context))

-- Access stored gl context
-- local gl_context = gl.get_gl_context()
-- if not gl_context then
--     print("Failed to get GL context: ", select(2, gl.get_gl_context()))
--     return
-- end
local server_status = "None"
-- Initialize ENet
enet.initialize()

-- Create server host (no specific address = bind to all interfaces)
local host = enet.host_create({host = "127.0.0.1", port = 12345}, 32, 0, 0, 0)
if not host then
    error("Failed to create server host")
end
server_status = "On"




-- Initialize ImGui with sdl_window and gl_context
success, err = imgui.init(window, gl_context)
if not success then
    print("ImGui init failed: " .. err)
    gl.destroy()
    sdl.quit()
    return
end

-- Main loop
local running = true
while running do
    -- Poll SDL events
    local events = sdl.poll_events_ig() -- Use poll_events_ig to process ImGui inputs
    for i, event in ipairs(events) do
        if event.type == sdl.EVENT_QUIT then
            running = false
        elseif event.type == sdl.EVENT_WINDOW_RESIZED then
            gl.viewport(0, 0, event.width, event.height)
        end
    end


    -- local event = enet.host_service(host, 1000)  -- 1 second timeout
    local event = enet.host_service(host, 0)  -- 1 second timeout
    if type(event) == "table" and event.type then
        if event.type == enet.EVENT_TYPE_CONNECT then
            print("Client connected from:", event.peer)
        elseif event.type == enet.EVENT_TYPE_DISCONNECT then
            print("Client disconnected:", event.peer)
        elseif event.type == enet.EVENT_TYPE_RECEIVE then
            local data = enet.packet_data(event.packet)
            print("Received from", event.peer, ":", data)
            
            -- Echo back the message
            local echo_packet = enet.packet_create(data, enet.PACKET_FLAG_RELIABLE)
            enet.peer_send(event.peer, event.channelID, echo_packet)
            
            -- Destroy the received packet (ENet owns it after service)
            enet.packet_destroy(event.packet)
        end
    elseif event == 0 then
        -- No event, continue
    else
        print("Service error:", event)
        -- break
        running = false
    end

    -- Start ImGui frame
    imgui.new_frame() -- start drawing imgui
    -- create widgets here

    -- Create a simple ImGui window
    local open = imgui.ig_begin("Test Window", true)
    if open then
        imgui.ig_text("Hello, ImGui from Lua!")
        imgui.ig_text("Server:")
        imgui.ig_text(server_status)

        if imgui.ig_button("ping") then
            print("Button clicked!")
        end


        -- if imgui.ig_button("Click Me") then
        --     print("Button clicked!")
        -- end

        -- if imgui.ig_button("Window ID") then
        --     local id = sdl.get_window_id(window)
        --     local display_id = sdl.get_primary_display(window)
        --     print("window id:", id)
        --     print("display_id:", display_id)
        -- end

        -- if imgui.ig_button("get_current_gl_context") then
        --     local context = sdl.get_current_gl_context()
        --     print("context:", context)
        -- end

        imgui.ig_end()
    end
    imgui.render() -- end drawing imgui

    -- Clear the screen
    gl.clear_color(0.2, 0.3, 0.3, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT)

    -- Render ImGui
    imgui.render_draw_data() -- draw gl imgui 

    -- Swap window
    sdl.gl_swap_window(window)
end

-- Cleanup

enet.host_destroy(host)
enet.deinitialize()

imgui.shutdown()
gl.destroy()
sdl.quit()