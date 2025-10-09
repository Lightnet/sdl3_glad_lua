
# SDL 3.2 api

```

```


```c
SDL_Window *window = SDL_CreateWindow("Lua+SDL3+GL", 800, 600, SDL_WINDOW_OPENGL);
SDL_PropertiesID props = SDL_GetWindowProperties(window);
SDL_SetPointerProperty(props, "lua_state", L)

```