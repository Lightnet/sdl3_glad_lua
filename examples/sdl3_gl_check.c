#include <SDL3/SDL.h>
#include <glad/gl.h>  // GLAD 2.0
#include <stdio.h>

int main(int argc, char **argv) {
    // Initialize SDL with video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);



    // Check all SDL_GLAttr attributes
    const struct { SDL_GLAttr attr; const char *name; } attrs_to_check[] = {
        { SDL_GL_RED_SIZE, "SDL_GL_RED_SIZE" },
        { SDL_GL_GREEN_SIZE, "SDL_GL_GREEN_SIZE" },
        { SDL_GL_BLUE_SIZE, "SDL_GL_BLUE_SIZE" },
        { SDL_GL_ALPHA_SIZE, "SDL_GL_ALPHA_SIZE" },
        { SDL_GL_BUFFER_SIZE, "SDL_GL_BUFFER_SIZE" },
        { SDL_GL_DOUBLEBUFFER, "SDL_GL_DOUBLEBUFFER" },
        { SDL_GL_DEPTH_SIZE, "SDL_GL_DEPTH_SIZE" },
        { SDL_GL_STENCIL_SIZE, "SDL_GL_STENCIL_SIZE" },
        { SDL_GL_ACCUM_RED_SIZE, "SDL_GL_ACCUM_RED_SIZE" },
        { SDL_GL_ACCUM_GREEN_SIZE, "SDL_GL_ACCUM_GREEN_SIZE" },
        { SDL_GL_ACCUM_BLUE_SIZE, "SDL_GL_ACCUM_BLUE_SIZE" },
        { SDL_GL_ACCUM_ALPHA_SIZE, "SDL_GL_ACCUM_ALPHA_SIZE" },
        { SDL_GL_STEREO, "SDL_GL_STEREO" },
        { SDL_GL_MULTISAMPLEBUFFERS, "SDL_GL_MULTISAMPLEBUFFERS" },
        { SDL_GL_MULTISAMPLESAMPLES, "SDL_GL_MULTISAMPLESAMPLES" },
        { SDL_GL_ACCELERATED_VISUAL, "SDL_GL_ACCELERATED_VISUAL" },
        { SDL_GL_CONTEXT_MAJOR_VERSION, "SDL_GL_CONTEXT_MAJOR_VERSION" },
        { SDL_GL_CONTEXT_MINOR_VERSION, "SDL_GL_CONTEXT_MINOR_VERSION" },
        { SDL_GL_CONTEXT_FLAGS, "SDL_GL_CONTEXT_FLAGS" },
        { SDL_GL_CONTEXT_PROFILE_MASK, "SDL_GL_CONTEXT_PROFILE_MASK" },
        { SDL_GL_SHARE_WITH_CURRENT_CONTEXT, "SDL_GL_SHARE_WITH_CURRENT_CONTEXT" },
        { SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, "SDL_GL_FRAMEBUFFER_SRGB_CAPABLE" },
        { SDL_GL_CONTEXT_RELEASE_BEHAVIOR, "SDL_GL_CONTEXT_RELEASE_BEHAVIOR" },
        { SDL_GL_CONTEXT_RESET_NOTIFICATION, "SDL_GL_CONTEXT_RESET_NOTIFICATION" },
        { SDL_GL_CONTEXT_NO_ERROR, "SDL_GL_CONTEXT_NO_ERROR" },
    };

    printf("\nAll SDL_GL Attributes:\n");
    for (int i = 0; i < sizeof(attrs_to_check) / sizeof(attrs_to_check[0]); i++) {
        int value;
        if (SDL_GL_GetAttribute(attrs_to_check[i].attr, &value) < 0) {
            printf("Failed to get %s: %s\n", attrs_to_check[i].name, SDL_GetError());
        } else {
            printf("%s: %d\n", attrs_to_check[i].name, value);
        }
    }

    // Reset GL attributes to ensure a clean state
    SDL_GL_ResetAttributes();



    // Set OpenGL attributes
    const struct { SDL_GLAttr attr; const char *name; int value; } attrs[] = {
        { SDL_GL_RED_SIZE, "SDL_GL_RED_SIZE", 8 },
        { SDL_GL_GREEN_SIZE, "SDL_GL_GREEN_SIZE", 8 },
        { SDL_GL_BLUE_SIZE, "SDL_GL_BLUE_SIZE", 8 },
        { SDL_GL_ALPHA_SIZE, "SDL_GL_ALPHA_SIZE", 8 },
        { SDL_GL_DOUBLEBUFFER, "SDL_GL_DOUBLEBUFFER", 1 },
        { SDL_GL_DEPTH_SIZE, "SDL_GL_DEPTH_SIZE", 24 },
        { SDL_GL_CONTEXT_PROFILE_MASK, "SDL_GL_CONTEXT_PROFILE_MASK", SDL_GL_CONTEXT_PROFILE_CORE },
        { SDL_GL_CONTEXT_MAJOR_VERSION, "SDL_GL_CONTEXT_MAJOR_VERSION", 3 },
        { SDL_GL_CONTEXT_MINOR_VERSION, "SDL_GL_CONTEXT_MINOR_VERSION", 3 },
    };

    // Set and verify each attribute
    for (int i = 0; i < sizeof(attrs) / sizeof(attrs[0]); i++) {
        if (SDL_GL_SetAttribute(attrs[i].attr, attrs[i].value) < 0) {
            printf("Failed to set %s: %s\n", attrs[i].name, SDL_GetError());
        } else {
            int set_value;
            if (SDL_GL_GetAttribute(attrs[i].attr, &set_value) < 0) {
                printf("Failed to get %s: %s\n", attrs[i].name, SDL_GetError());
            } else {
                printf("Set %s to %d\n", attrs[i].name, set_value);
                if (set_value != attrs[i].value) {
                    printf("Warning: %s set to %d, expected %d\n", 
                           attrs[i].name, set_value, attrs[i].value);
                }
            }
        }
    }

    // Create OpenGL window
    SDL_Window *window = SDL_CreateWindow("SDL3 GL Test", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create OpenGL context
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Set swap interval (VSync)
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Failed to set VSync: %s\n", SDL_GetError());
    }

    // Print OpenGL version
    const unsigned char *gl_version = glGetString(0x1F02); // GL_VERSION
    const unsigned char *gl_renderer = glGetString(0x1F01); // GL_RENDERER
    if (gl_version && gl_renderer) {
        printf("OpenGL loaded: %s %s\n", gl_version, gl_renderer);
    } else {
        printf("Failed to get OpenGL version/renderer\n");
    }

    // Cleanup
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}