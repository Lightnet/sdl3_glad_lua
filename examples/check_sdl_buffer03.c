#include <SDL3/SDL.h>
#include <stdio.h>
#include "glad/gl.h"  // GLAD 2.0.8 header (OpenGL 4.6 core profile)

// Helper function to query and print SDL_GL attributes
void print_gl_attributes(const char* stage) {
    int value;
    printf("%s GL context creation:\n", stage);

    if (SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &value) == 0) {
        printf("  SDL_GL_DOUBLEBUFFER: %d\n", value);
    } else {
        printf("  Failed to get SDL_GL_DOUBLEBUFFER: %s\n", SDL_GetError());
    }

    if (SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value) == 0) {
        printf("  SDL_GL_DEPTH_SIZE: %d\n", value);
    } else {
        printf("  Failed to get SDL_GL_DEPTH_SIZE: %s\n", SDL_GetError());
    }

    if (SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &value) == 0) {
        printf("  SDL_GL_STENCIL_SIZE: %d\n", value);
    } else {
        printf("  Failed to get SDL_GL_STENCIL_SIZE: %s\n", SDL_GetError());
    }

    if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &value) == 0) {
        printf("  SDL_GL_MULTISAMPLEBUFFERS: %d\n", value);
    } else {
        printf("  Failed to get SDL_GL_MULTISAMPLEBUFFERS: %s\n", SDL_GetError());
    }

    if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &value) == 0) {
        printf("  SDL_GL_MULTISAMPLESAMPLES: %d\n", value);
    } else {
        printf("  Failed to get SDL_GL_MULTISAMPLESAMPLES: %s\n", SDL_GetError());
    }

    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &value) == 0) {
        printf("  SDL_GL_CONTEXT_FLAGS: %d\n", value);
    } else {
        printf("  Failed to get SDL_GL_CONTEXT_FLAGS: %s\n", SDL_GetError());
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Print attributes before creating GL context/window
    print_gl_attributes("not set variable");

    // Set OpenGL attributes before window creation
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    // Print attributes before creating GL context/window
    print_gl_attributes("Before");

    // Create window with OpenGL context
    SDL_Window* window = SDL_CreateWindow("SDL3 GL Attribute Test", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
    printf("GLAD version: %s\n", GLAD_GENERATOR_VERSION);

    // Print attributes after GL context creation
    print_gl_attributes("After");

    // Enable multisampling in OpenGL (if supported)
    glEnable(GL_MULTISAMPLE);

    // Clear the screen with a color to test rendering
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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