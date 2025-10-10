#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Vertex shader with MVP transformation
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "void main() {\n"
    "    gl_Position = mvp * vec4(aPos, 1.0);\n"
    "    vertexColor = aColor;\n"
    "}\0";

// Fragment shader
const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 vertexColor;\n"
    "void main() {\n"
    "    FragColor = vec4(vertexColor, 1.0);\n"
    "}\n\0";

// Vertex structure
typedef struct {
    float position[3];
    float color[3];
} Vertex;

// Check OpenGL errors
void check_gl_error(const char* context) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL error at %s: %d\n", context, err);
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return -1;
    }

    // Request OpenGL 3.3 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // Create window
    SDL_Window* window = SDL_CreateWindow("3D Cube - SDL3 + GLAD + CGLM",
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

    // Load GLAD
    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    printf("OpenGL loaded: %s\n", glGetString(GL_VERSION));
    check_gl_error("GLAD initialization");

    // Set viewport
    glViewport(0, 0, 800, 600);
    check_gl_error("Set viewport");

    // Cube vertices
    Vertex vertices[] = {
        // Front face
        { {-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}}, // 0: Bottom-left-front (red)
        { { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}}, // 1: Bottom-right-front (green)
        { { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}, // 2: Top-right-front (blue)
        { {-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}}, // 3: Top-left-front (yellow)
        // Back face
        { {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}}, // 4: Bottom-left-back (magenta)
        { { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}}, // 5: Bottom-right-back (cyan)
        { { 0.5f,  0.5f, -0.5f}, {1.0f, 0.5f, 0.0f}}, // 6: Top-right-back (orange)
        { {-0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}}  // 7: Top-left-back (gray)
    };

    // Cube indices
    unsigned int indices[] = {
        0, 1, 2,  2, 3, 0,  // Front
        5, 4, 7,  7, 6, 5,  // Back
        4, 0, 3,  3, 7, 4,  // Left
        1, 5, 6,  6, 2, 1,  // Right
        3, 2, 6,  6, 7, 3,  // Top
        4, 5, 1,  1, 0, 4   // Bottom
    };

    // Compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("Vertex shader compilation failed: %s\n", infoLog);
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("Fragment shader compilation failed: %s\n", infoLog);
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Shader program linking failed: %s\n", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    check_gl_error("Shader setup");

    // Set up VAO, VBO, EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    check_gl_error("VAO/VBO setup");

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    check_gl_error("Enable depth test");

    // Enable wireframe mode for debugging (comment out for filled rendering)
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    check_gl_error("Set polygon mode");

    // Get MVP uniform location
    unsigned int mvpLoc = glGetUniformLocation(shaderProgram, "mvp");
    if (mvpLoc == -1) {
        printf("Failed to get MVP uniform location\n");
    }

    // Main loop
    bool running = true;
    SDL_Event event;
    float angle = 0.0f;
    float anglez = 0.0f;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            // Handle window resize
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                glViewport(0, 0, w, h);
                check_gl_error("Resize viewport");
            }
        }

        // Update rotation
        angle += 0.01f;
        anglez += 0.01f;

        // Set up matrices using CGLM
        mat4 model, view, proj, mvp;
        glm_mat4_identity(model);
        glm_rotate_y(model, angle, model); // Rotate around Y-axis
        glm_rotate_z(model, anglez, model); // Rotate around Y-axis
        glm_mat4_identity(view);
        // glm_translate(view, (vec3){0.0f, 0.0f, -5.0f}); // Move camera back
        glm_translate(view, (vec3){0.0f, 0.0f, -3.0f}); // Move camera back
        glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, 100.0f, proj); // 45° FOV

        // Compute MVP
        mat4 temp;
        glm_mat4_mul(view, model, temp);
        glm_mat4_mul(proj, temp, mvp);

        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        check_gl_error("Clear screen");

        // Draw cube
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, (float*)mvp);
        check_gl_error("Set MVP uniform");
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        check_gl_error("Draw elements");
        glBindVertexArray(0);

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}