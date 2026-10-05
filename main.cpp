#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

const int WIDTH  = 800;
const int HEIGHT = 600;

bool isWhiteBackground = false;

const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;

void main() {
    gl_Position = vec4(aPos, 1.0);
    vColor = aColor;
}
)";

const char* fragmentSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

unsigned int makeShader(const char* vShaderCode, const char* fShaderCode) {
    int ok = 0;
    char infoLog[512];

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vShaderCode, nullptr);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(vs, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fShaderCode, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(fs, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
        isWhiteBackground = true;
    } else {
        isWhiteBackground = false;
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW іске қосылмады\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT,
                                          "Компьютерлік графика - 3-апта",
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "Терезе жасалмады.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(1);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "GLAD жүктелмеді\n";
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GPU:    " << glGetString(GL_RENDERER) << "\n";

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,    1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.0f,    0.0f, 0.0f, 1.0f
    };

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const int STRIDE = 6 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    unsigned int shaderProgram = makeShader(vertexSrc, fragmentSrc);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        if (isWhiteBackground) {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            float t = (float)glfwGetTime();
            float r = (std::sin(t * 0.5f) + 1.0f) * 0.5f * 0.3f;
            float g = (std::sin(t * 0.3f) + 1.0f) * 0.5f * 0.3f;
            glClearColor(r, g, 0.35f, 1.0f);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}