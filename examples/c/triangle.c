#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Vertex shader source (simple passthrough with color)
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 vertexColor;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "    vertexColor = aColor;\n"
    "}\0";

// Fragment shader source (output vertex color)
const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 vertexColor;\n"
    "void main() {\n"
    "    FragColor = vec4(vertexColor, 1.0);\n"
    "}\n\0";

// Structure for vertex data: position (vec3) + color (vec3)
typedef struct {
    float position[3];
    float color[3];
} Vertex;

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return -1;
    }

    // Request OpenGL 3.3 core context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // Create window (800x600, OpenGL-enabled)
    SDL_Window* window = SDL_CreateWindow("Simple Triangle - SDL3 + GLAD 2.0",
                                          800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Create OpenGL context
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        printf("GL context creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Load OpenGL functions with GLAD (using SDL's proc address getter)
    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Verify OpenGL version
    printf("OpenGL loaded: %s\n", glGetString(GL_VERSION));

    // Vertex data: three vertices forming a triangle (positions + colors)
    Vertex vertices[] = {
        { {-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f} },  // Bottom-left (red)
        { { 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f} },  // Bottom-right (green)
        { { 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f} }   // Top (blue)
    };

    // Compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check compile errors (omitted for brevity; add in production)

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check compile errors (omitted for brevity; add in production)

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check link errors (omitted for brevity; add in production)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Enable depth testing (optional for 2D)
    glEnable(GL_DEPTH_TEST);

    // Main loop
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}