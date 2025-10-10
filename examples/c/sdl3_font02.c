// 
// required glad 2.0.8
// 
#include <SDL3/SDL.h>
#include <glad/gl.h>  // GLAD header (replaces SDL_opengl.h for OpenGL functions)
#include <stdio.h>
#include <stdlib.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"  // Your font header

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        fprintf(stderr, "Failed to init video! %s\n", SDL_GetError());
        return 1;
    }

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_OPENGL;
    SDL_Window* window = SDL_CreateWindow("SDL3 OpenGL Font Example", (int)(1280 * main_scale), (int)(720 * main_scale), window_flags);
    if (!window) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Create OpenGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_GL_MakeCurrent(window, gl_context);


    int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
    if (version == 0) {
        printf("Failed to initialize GLAD\n");
        // Handle error
        return 1;
    }

    int major = GLAD_VERSION_MAJOR(version);
    int minor = GLAD_VERSION_MINOR(version);
    printf("Loaded OpenGL version %d.%d\n", major, minor);

    // Load OpenGL functions with GLAD using SDL's loader
    // if (!gladLoaderLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
    // if (!gladLoaderLoadGL()) {
    //     printf("Error: Failed to load OpenGL functions with GLAD\n");
    //     SDL_GL_DestroyContext(gl_context);
    //     SDL_DestroyWindow(window);
    //     SDL_Quit();
    //     return -1;
    // }
    printf("OpenGL loaded: version %s\n", glGetString(GL_VERSION));  // Optional: Verify loaded version

    // Load font
    unsigned char ttf_buffer[1 << 20];
    FILE* ff = fopen("resources/Kenney Mini.ttf", "rb");
    if (!ff) {
        printf("Error: Failed to open font file 'Kenney Mini.ttf'\n");
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    fread(ttf_buffer, 1, 1 << 20, ff);
    fclose(ff);

    const int bitmap_w = 512;
    const int bitmap_h = 512;
    unsigned char* bitmap = (unsigned char*)malloc(bitmap_w * bitmap_h);
    stbtt_bakedchar cdata[96]; // ASCII 32..127
    float font_size = 32.0f * main_scale;
    stbtt_BakeFontBitmap(ttf_buffer, 0, font_size, bitmap, bitmap_w, bitmap_h, 32, 96, cdata);

    // Create OpenGL texture
    GLuint font_tex;
    glGenTextures(1, &font_tex);
    glBindTexture(GL_TEXTURE_2D, font_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmap_w, bitmap_h, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    free(bitmap);

    // Shaders
    const char* vs_src =
        "#version 330 core\n"
        "layout(location = 0) in vec2 position;\n"
        "layout(location = 1) in vec2 texCoord;\n"
        "out vec2 TexCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 0.0, 1.0);\n"
        "    TexCoord = texCoord;\n"
        "}\n";

    const char* fs_src =
        "#version 330 core\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D textTexture;\n"
        "uniform vec4 textColor;\n"
        "void main() {\n"
        "    float alpha = texture(textTexture, TexCoord).r;\n"
        "    FragColor = vec4(textColor.rgb, alpha * textColor.a);\n"
        "}\n";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, NULL);
    glCompileShader(vs);
    GLint success;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(vs, 512, NULL, info_log);
        printf("Vertex shader compilation failed: %s\n", info_log);
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(fs, 512, NULL, info_log);
        printf("Fragment shader compilation failed: %s\n", info_log);
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        printf("Program linking failed: %s\n", info_log);
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    // VAO and VBO
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Clear color
    float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

    // Main loop
    bool done = false;
    float vertices[1024 * 4]; // Enough for simple text
    const char* text = "Hello World! Glad 2.0.8";
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        int ww, hh;
        SDL_GetWindowSize(window, &ww, &hh);
        glViewport(0, 0, ww, hh);

        glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render text
        float x = 25.0f;
        float y = 150.0f; // Baseline position from top
        int vert_count = 0;
        for (const char* p = text; *p; p++) {
            if (*p >= 32 && *p < 128) {
                stbtt_aligned_quad q;
                stbtt_GetBakedQuad(cdata, bitmap_w, bitmap_h, *p - 32, &x, &y, &q, 1);

                float nx0 = 2.0f * q.x0 / ww - 1.0f;
                float ny0 = 1.0f - 2.0f * q.y0 / hh;
                float nx1 = 2.0f * q.x1 / ww - 1.0f;
                float ny1 = 1.0f - 2.0f * q.y1 / hh;

                // Triangle 1
                vertices[vert_count++] = nx0; vertices[vert_count++] = ny0; vertices[vert_count++] = q.s0; vertices[vert_count++] = q.t0;
                vertices[vert_count++] = nx1; vertices[vert_count++] = ny0; vertices[vert_count++] = q.s1; vertices[vert_count++] = q.t0;
                vertices[vert_count++] = nx1; vertices[vert_count++] = ny1; vertices[vert_count++] = q.s1; vertices[vert_count++] = q.t1;

                // Triangle 2
                vertices[vert_count++] = nx0; vertices[vert_count++] = ny0; vertices[vert_count++] = q.s0; vertices[vert_count++] = q.t0;
                vertices[vert_count++] = nx1; vertices[vert_count++] = ny1; vertices[vert_count++] = q.s1; vertices[vert_count++] = q.t1;
                vertices[vert_count++] = nx0; vertices[vert_count++] = ny1; vertices[vert_count++] = q.s0; vertices[vert_count++] = q.t1;
            }
        }

        glUseProgram(program);
        glUniform1i(glGetUniformLocation(program, "textTexture"), 0);
        glUniform4f(glGetUniformLocation(program, "textColor"), 1.0f, 1.0f, 1.0f, 1.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font_tex);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vert_count * sizeof(float), vertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, vert_count / 4);
        glBindVertexArray(0);

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    glDeleteProgram(program);
    glDeleteTextures(1, &font_tex);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}