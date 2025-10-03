cmake_minimum_required(VERSION 3.22)
project(sdl3_glad_lua)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set C standard (since you're using .c files)
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_STANDARD 99)
# set(CMAKE_C_STANDARD_REQUIRED ON)

# Include FetchContent for downloading dependencies
include(FetchContent)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Fetch raylib
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.22
    GIT_SHALLOW TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(sdl3)

# Fetch cimgui
FetchContent_Declare(
    cimgui
    GIT_REPOSITORY https://github.com/cimgui/cimgui.git
    GIT_TAG master 
    # GIT_TAG 1.92.1
    EXCLUDE_FROM_ALL #this disable cmake build? since customize for sdl3 and opengl3.
    GIT_SHALLOW TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(cimgui)

#================================================
# cimgui custom
#================================================

# Define cimgui sources
set(IMGUI_SOURCES
    ${cimgui_SOURCE_DIR}/cimgui.cpp
    ${cimgui_SOURCE_DIR}/imgui/imgui.cpp
    ${cimgui_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${cimgui_SOURCE_DIR}/imgui/imgui_widgets.cpp
    ${cimgui_SOURCE_DIR}/imgui/imgui_demo.cpp
    ${cimgui_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${cimgui_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.cpp
    ${cimgui_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

# Build cimgui as a static library with a custom name
add_library(custom_cimgui STATIC ${IMGUI_SOURCES})

# Set include directories for custom_cimgui
target_include_directories(custom_cimgui PUBLIC
    ${cimgui_SOURCE_DIR}                    # cimgui
    ${cimgui_SOURCE_DIR}/imgui              # cimgui/imgui
    ${cimgui_SOURCE_DIR}/imgui/backends     # imgui/backends
    ${SDL3_SOURCE_DIR}/include              # SDL 3.2
)

# Set compile definitions for custom_cimgui (no CIMGUI_DEFINE_ENUMS_AND_STRUCTS)
target_compile_definitions(custom_cimgui PUBLIC
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
    IMGUI_IMPL_API=extern\ \"C\"
)

# Link custom_cimgui with GLFW and OpenGL
# need this to compile for library else error on compile
target_link_libraries(custom_cimgui PUBLIC 
    OpenGL::GL
    SDL3::SDL3
)

#================================================
# Lua
#================================================
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG v5.4.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(lua)

# Lua doesn't have a CMake build, so we manually compile it
# List Lua source files (core and libraries, excluding lua.c and luac.c)
set(LUA_SRC
    ${lua_SOURCE_DIR}/lapi.c
    ${lua_SOURCE_DIR}/lauxlib.c
    ${lua_SOURCE_DIR}/lbaselib.c
    ${lua_SOURCE_DIR}/lcode.c
    ${lua_SOURCE_DIR}/lcorolib.c
    ${lua_SOURCE_DIR}/lctype.c
    ${lua_SOURCE_DIR}/ldblib.c
    ${lua_SOURCE_DIR}/ldebug.c
    ${lua_SOURCE_DIR}/ldo.c
    ${lua_SOURCE_DIR}/ldump.c
    ${lua_SOURCE_DIR}/lfunc.c
    ${lua_SOURCE_DIR}/lgc.c
    ${lua_SOURCE_DIR}/linit.c
    ${lua_SOURCE_DIR}/liolib.c
    ${lua_SOURCE_DIR}/llex.c
    ${lua_SOURCE_DIR}/lmathlib.c
    ${lua_SOURCE_DIR}/lmem.c
    ${lua_SOURCE_DIR}/loadlib.c
    ${lua_SOURCE_DIR}/lobject.c
    ${lua_SOURCE_DIR}/lopcodes.c
    ${lua_SOURCE_DIR}/loslib.c
    ${lua_SOURCE_DIR}/lparser.c
    ${lua_SOURCE_DIR}/lstate.c
    ${lua_SOURCE_DIR}/lstring.c
    ${lua_SOURCE_DIR}/lstrlib.c
    ${lua_SOURCE_DIR}/ltable.c
    ${lua_SOURCE_DIR}/ltablib.c
    ${lua_SOURCE_DIR}/ltm.c
    ${lua_SOURCE_DIR}/lundump.c
    ${lua_SOURCE_DIR}/lvm.c
    ${lua_SOURCE_DIR}/lzio.c
    ${lua_SOURCE_DIR}/lutf8lib.c
)

# Create a static Lua library
add_library(lua STATIC ${LUA_SRC})
target_include_directories(lua PUBLIC ${lua_SOURCE_DIR})

#================================================
# ENET
#================================================
FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/zpl-c/enet.git
    GIT_TAG v2.6.5
    GIT_SHALLOW TRUE
)
set(ENET_TEST  OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(enet)

#================================================
# APP
#================================================

# Application NAME
set(APP_NAME sdl3_lua)

# Source files (add more if needed)
set(SRC_FILES
    # Add other .c files here if necessary
    src/module_sdl.c
    src/module_lua.c
    src/module_gl.c
    src/module_imgui.c
    src/module_enet.c
    glad/src/gl.c
)

message(STATUS "cimgui_SOURCE_DIR: >> ${cimgui_SOURCE_DIR}")

# Create executable
add_executable(${APP_NAME}
    ${SRC_FILES}
    # glad/src/gl.c
    src/main.c
    # examples/sdl3_gl_check.c
    # src/check_sdl_buffer.c
    # src/check_sdl_buffer03.c
    # src/triangle.c
    # src/server.c
    # src/sdl3_cimgui.c
)

# SDL3 
target_link_libraries(${APP_NAME} PUBLIC SDL3::SDL3) # SDL 3.2.22
target_link_libraries(${APP_NAME} PRIVATE custom_cimgui) # custom cimgui for sdl3 and opengl3, 1.92.1 or master
target_link_libraries(${APP_NAME} PRIVATE lua) # lua 5.4.7
# CMAKE_SOURCE_DIR > current folder
# CMAKE_CURRENT_SOURCE_DIR  > top folder
# Include directories
target_include_directories(${APP_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/glad/include            # glad
    ${CMAKE_SOURCE_DIR}/include                 # root project
    ${cimgui_SOURCE_DIR}                        # cimgui
    ${cimgui_SOURCE_DIR}/imgui                  # imgui
    ${cimgui_SOURCE_DIR}/imgui/backends         # imgui/backends
    ${SDL3_SOURCE_DIR}/include                  # SDL 3.2.22
    ${lua_SOURCE_DIR}/include                   # lua
    ${enet_SOURCE_DIR}/include                  # enet
)

# for c #define
target_compile_definitions(
	${APP_NAME}
	PUBLIC
    #IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
	CIMGUI_DEFINE_ENUMS_AND_STRUCTS=1
	CIMGUI_USE_SDL3=1
	CIMGUI_USE_OPENGL3=1
    ENET_IMPLEMENTATION=1                   #enet
)

# Platform-specific settings for Windows (MinGW/MSYS2)
if (WIN32)
    # Link necessary Windows libraries for raylib
    target_link_libraries(${APP_NAME} PRIVATE
        opengl32                                        # opengl
        gdi32                                           # Windows Graphics Device Interface
        winmm                                           # Windows Multimedia API 
        ws2_32                                          # Winsock 2
    )

    # Static linking for libgcc and libstdc++
    target_link_options(${APP_NAME} PRIVATE
        -static-libgcc                                  # GNU Compiler Collection
        -static-libstdc++                               # Uncomment if C++ code is used # need for imgui
        -static                                         # Avoid full static linking to prevent issues with system libraries
    )
endif()

# configure_file("script.lua" "${CMAKE_BINARY_DIR}/script.lua" COPYONLY)


# set(APP_EX triangle)

# set(SRC_EX_FILES
#     # Add other .c files here if necessary
#     glad/src/gl.c
# )

# add_executable(${APP_EX}
#     ${SRC_EX_FILES}
#     glad/src/gl.c
#     src/triangle.c
# )
# target_link_libraries(${APP_EX} PUBLIC 
#     SDL3::SDL3                      # SDL 3.2.22
#     custom_cimgui                   # custom cimgui for sdl3 and opengl3, 1.92.1 or master
#     lua                             # 5.4
# )

# target_include_directories(${APP_EX} PUBLIC
#     ${CMAKE_SOURCE_DIR}/glad/include            # glad
#     ${CMAKE_SOURCE_DIR}/include                 # root project
#     ${cimgui_SOURCE_DIR}                        # cimgui
#     ${cimgui_SOURCE_DIR}/imgui                  # imgui
#     ${cimgui_SOURCE_DIR}/imgui/backends         # imgui/backends
#     ${SDL3_SOURCE_DIR}/include                  # SDL 3.2.22
#     ${lua_SOURCE_DIR}/include                   # lua
#     ${enet_SOURCE_DIR}/include                  # enet
# )

# target_compile_definitions(
# 	${APP_EX}
# 	PUBLIC
#     #IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
# 	CIMGUI_DEFINE_ENUMS_AND_STRUCTS=1
# 	CIMGUI_USE_SDL3=1
# 	CIMGUI_USE_OPENGL3=1
#     ENET_IMPLEMENTATION=1                   #enet
# )

# if (WIN32)
#     # Link necessary Windows libraries for raylib
#     target_link_libraries(${APP_EX} PRIVATE
#         opengl32                                # opengl
#         gdi32                                   # Windows Graphics Device Interface 
#         winmm                                   # Windows Multimedia API 
#         ws2_32                                  # Winsock 2
#     )

#     # Static linking for libgcc and libstdc++
#     target_link_options(${APP_EX} PRIVATE
#         -static-libgcc                          # GNU Compiler Collection
#         -static-libstdc++                       # Uncomment if C++ code is used
#         -static                                 # Avoid full static linking to prevent issues with system libraries
#     )
# endif()
