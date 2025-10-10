#include <SDL3/SDL.h>
#include <glad/gl.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Vertex shader source (for textured quads)
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "    gl_Position = projection * vec4(aPos, 0.0, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\0";

// Fragment shader source (for texturing)
const char* fragmentShaderSource = "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D textTexture;\n"
    "void main() {\n"
    "    float alpha = texture(textTexture, TexCoord).r;\n"
    // "    FragColor = vec4(1.0, 1.0, 1.0, alpha);\n"
    "    FragColor = vec4(0.5, 0.5, 0.5, 1.0);\n"
    "}\n\0";

// Structure for vertex data: position (vec2) + texture coordinates (vec2)
typedef struct {
    float position[2];
    float texCoord[2];
} Vertex;

// Structure to hold font information
typedef struct {
    stbtt_fontinfo font;
    unsigned char* bitmap;
    int bitmap_w, bitmap_h;
    stbtt_bakedchar cdata[96]; // ASCII 32..126
    GLuint texture;
    float scale;
} Font;

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
    SDL_Window* window = SDL_CreateWindow("Text Rendering - SDL3 + GLAD + stb_truetype",
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

    // Load OpenGL functions with GLAD
    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    printf("OpenGL loaded: %s\n", glGetString(GL_VERSION));

    // Compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("Vertex shader compilation failed: %s\n", infoLog);
        return -1;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("Fragment shader compilation failed: %s\n", infoLog);
        return -1;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Shader program linking failed: %s\n", infoLog);
        return -1;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Load font
    Font font;
    FILE* fontFile = fopen("resources/Kenney Mini.ttf", "rb");
    if (!fontFile) {
        printf("Failed to open font file\n");
        SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Read font file
    fseek(fontFile, 0, SEEK_END);
    long fontSize = ftell(fontFile);
    fseek(fontFile, 0, SEEK_SET);
    unsigned char* fontBuffer = (unsigned char*)malloc(fontSize);
    fread(fontBuffer, 1, fontSize, fontFile);
    fclose(fontFile);

    // Initialize stb_truetype
    stbtt_InitFont(&font.font, fontBuffer, 0);
    font.bitmap_w = 512;
    font.bitmap_h = 512;
    font.bitmap = (unsigned char*)malloc(font.bitmap_w * font.bitmap_h);
    font.scale = stbtt_ScaleForPixelHeight(&font.font, 64.0f);
    stbtt_BakeFontBitmap(fontBuffer, 0, 64.0f, font.bitmap, font.bitmap_w, font.bitmap_h, 32, 96, font.cdata);

    // Create OpenGL texture for font atlas
    glGenTextures(1, &font.texture);
    glBindTexture(GL_TEXTURE_2D, font.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font.bitmap_w, font.bitmap_h, 0, GL_RED, GL_UNSIGNED_BYTE, font.bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Set up orthographic projection matrix
    float projection[16] = {
        2.0f / 800, 0.0f, 0.0f, -1.0f,
        0.0f, -2.0f / 600, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Set up VAO and VBO for dynamic text rendering
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Enable blending for text rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
        glClear(GL_COLOR_BUFFER_BIT);

        // Render text
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, projection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font.texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "textTexture"), 0);
        glBindVertexArray(VAO);

        const char* text = "Hello, World!";
        float x = 300.0f; // Start position (x)
        float y = 300.0f; // Start position (y)
        for (const char* p = text; *p; p++) {
            if (*p >= 32 && *p < 128) {
                stbtt_aligned_quad q;
                stbtt_GetBakedQuad(font.cdata, font.bitmap_w, font.bitmap_h, *p - 32, &x, &y, &q, 1);

                Vertex vertices[6] = {
                    {{q.x0, q.y0}, {q.s0, q.t0}},
                    {{q.x1, q.y0}, {q.s1, q.t0}},
                    {{q.x1, q.y1}, {q.s1, q.t1}},
                    {{q.x0, q.y0}, {q.s0, q.t0}},
                    {{q.x1, q.y1}, {q.s1, q.t1}},
                    {{q.x0, q.y1}, {q.s0, q.t1}}
                };

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    free(fontBuffer);
    free(font.bitmap);
    glDeleteTextures(1, &font.texture);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}