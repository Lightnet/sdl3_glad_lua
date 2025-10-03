#include <SDL3/SDL.h>
#include <stdio.h>
#include "glad/gl.h"  // GLAD 2.0.8 header
#include <math.h>

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Test SDL_GL_DOUBLEBUFFER before context creation
    int doublebuffer_before;
    if (SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer_before) == 0) {
        printf("Before GL context creation - SDL_GL_DOUBLEBUFFER value: %d\n", doublebuffer_before);
    } else {
        printf("Failed to get SDL_GL_DOUBLEBUFFER before creation: %s\n", SDL_GetError());
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("SDL3 GL Test", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create OpenGL context
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        printf("Failed to create GL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load OpenGL functions using GLAD 2.0.8
    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD!\n");
        SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("OpenGL loaded with GLAD 2.0.8. OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("GLAD version: %s\n", GLAD_GENERATOR_VERSION);

    // Test SDL_GL_DOUBLEBUFFER after context creation
    int doublebuffer_after;
    if (SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer_after) == 0) {
        printf("After GL context creation - SDL_GL_DOUBLEBUFFER value: %d\n", doublebuffer_after);
    } else {
        printf("Warning: Failed to get SDL_GL_DOUBLEBUFFER after creation: %s (continuing, likely SDL3 issue)\n", SDL_GetError());
    }

    // Animation loop to visually confirm double buffering
    bool running = true;
    float t = 0.0f;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Clear with oscillating color
        glClearColor(0.2f + 0.1f * sinf(t), 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);

        t += 0.05f;
        SDL_Delay(16); // ~60 FPS
        if (t > 6.28f) break; // ~3 seconds
    }

    // Clean up
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("Program finished successfully\n");
    return 0;
}