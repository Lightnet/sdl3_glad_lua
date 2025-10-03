#include <SDL3/SDL.h>
#include <stdio.h>

// GLAD headers - assuming glad/gl.h and gl.c are generated for version 2.0.8
// and included in the project. Generate from https://glad.dav1d.de/ with OpenGL 4.6 core profile or appropriate version.
#include "glad/gl.h"  // Include the generated glad header

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Set OpenGL attributes before window creation
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Test SDL_GL_DOUBLEBUFFER before creating any GL context/window
    int doublebuffer_before;
    if (SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer_before) == 0) {
        printf("Before GL context creation - SDL_GL_DOUBLEBUFFER value: %d\n", doublebuffer_before);
    } else {
        printf("Failed to get SDL_GL_DOUBLEBUFFER before creation: %s\n", SDL_GetError());
    }

    // Set the attribute for double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Create window with OpenGL context
    SDL_Window* window = SDL_CreateWindow("SDL3 GL Test", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create OpenGL context
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == NULL) {
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
    printf("GLAD version: %s\n", GLAD_GENERATOR_VERSION);  // Verify GLAD version (defined in glad.h)

    // Test SDL_GL_DOUBLEBUFFER after GL context creation
    int doublebuffer_after;
    if (SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer_after) == 0) {
        printf("After GL context creation - SDL_GL_DOUBLEBUFFER value: %d\n", doublebuffer_after);
    } else {
        printf("Failed to get SDL_GL_DOUBLEBUFFER after creation: %s\n", SDL_GetError());
    }

    // Example: Clear the screen with a color to test rendering (requires double buffer for smooth swap)
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Swap buffers to display (tests double buffering)
    SDL_GL_SwapWindow(window);

    // Wait for 3 seconds
    SDL_Delay(3000);

    // Clean up
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}