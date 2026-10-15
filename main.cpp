#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>
#include <vector>

const int WIDTH  = 800;
const int HEIGHT = 600;

bool isWhiteBackground = false;
float speed = 1.0f;
float aspect = (float)WIDTH / (float)HEIGHT;

const float GROUND_Y   = -0.8f;
const float GRAVITY    = 4.0f;
const float JUMP_SPEED = 2.6f;
const float BOUNCE_T   = 2.0f * JUMP_SPEED / GRAVITY;

const float BALL_R      = 0.12f;
const float BALL_X      = 0.0f;
const float SQUARE_HALF = 0.08f;

const int CIRCLE_SEGMENTS = 48;

const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform vec2 uOffset;
uniform float uScale;
uniform float uAspect;
uniform vec3 uTint;

out vec3 vColor;

void main() {
    vec2 p = aPos.xy * uScale + uOffset;
    gl_Position = vec4(p.x / uAspect, p.y, aPos.z, 1.0);
    vColor = aColor * uTint;
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
    if (height > 0) {
        aspect = (float)width / (float)height;
    }
}

void processInput(GLFWwindow* window, float dt) {
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

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        speed += 1.0f * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        speed -= 1.0f * dt;
        if (speed < 0.1f) speed = 0.1f;
    }
}

void makeMesh(const float* vertices, size_t verticesSize,
              const unsigned int* indices, size_t indicesSize,
              unsigned int& vao, unsigned int& vbo, unsigned int& ebo) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

    const int STRIDE = 6 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

float ballHeight(float t) {
    float tau = std::fmod(t, BOUNCE_T);
    return JUMP_SPEED * tau - 0.5f * GRAVITY * tau * tau;
}

float squareX(float t) {
    float start = aspect + SQUARE_HALF;
    float length = 2.0f * start;
    float phase = std::fmod(t, BOUNCE_T) / BOUNCE_T;
    return start - length * phase;
}

bool isTouching(float ballX, float ballY, float sqX, float sqY) {
    float nx = std::fmax(sqX - SQUARE_HALF, std::fmin(ballX, sqX + SQUARE_HALF));
    float ny = std::fmax(sqY - SQUARE_HALF, std::fmin(ballY, sqY + SQUARE_HALF));
    float dx = ballX - nx;
    float dy = ballY - ny;
    return dx * dx + dy * dy < BALL_R * BALL_R;
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
                                          "Компьютерлік графика - 6-апта",
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

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    onResize(window, fbW, fbH);

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GPU:    " << glGetString(GL_RENDERER) << "\n";
    std::cout << "W/S: жылдамдық, Space: ақ фон, Esc: шығу\n";

    std::vector<float> circleVertices;
    circleVertices.insert(circleVertices.end(), { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });
    for (int i = 0; i < CIRCLE_SEGMENTS; ++i) {
        float a = 2.0f * 3.14159265f * i / CIRCLE_SEGMENTS;
        float x = std::cos(a) * BALL_R;
        float y = std::sin(a) * BALL_R;
        float r = 0.5f + 0.5f * std::cos(a);
        float g = 0.5f + 0.5f * std::cos(a + 2.094f);
        float b = 0.5f + 0.5f * std::cos(a + 4.189f);
        circleVertices.insert(circleVertices.end(), { x, y, 0.0f, r, g, b });
    }

    std::vector<unsigned int> circleIndices;
    for (int i = 1; i <= CIRCLE_SEGMENTS; ++i) {
        int next = (i % CIRCLE_SEGMENTS) + 1;
        circleIndices.insert(circleIndices.end(), { 0u, (unsigned int)i, (unsigned int)next });
    }

    unsigned int circleVao, circleVbo, circleEbo;
    makeMesh(circleVertices.data(), circleVertices.size() * sizeof(float),
             circleIndices.data(), circleIndices.size() * sizeof(unsigned int),
             circleVao, circleVbo, circleEbo);

    float h = SQUARE_HALF;
    float squareVertices[] = {
         h,  h, 0.0f,    1.0f, 0.0f, 0.0f,
         h, -h, 0.0f,    0.0f, 1.0f, 0.0f,
        -h, -h, 0.0f,    0.0f, 0.0f, 1.0f,
        -h,  h, 0.0f,    1.0f, 1.0f, 0.0f
    };

    unsigned int squareIndices[] = {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int squareVao, squareVbo, squareEbo;
    makeMesh(squareVertices, sizeof(squareVertices), squareIndices, sizeof(squareIndices),
             squareVao, squareVbo, squareEbo);

    float groundVertices[] = {
         1.0f,  0.0f, 0.0f,    0.45f, 0.45f, 0.45f,
         1.0f, -1.0f, 0.0f,    0.25f, 0.25f, 0.25f,
        -1.0f, -1.0f, 0.0f,    0.25f, 0.25f, 0.25f,
        -1.0f,  0.0f, 0.0f,    0.45f, 0.45f, 0.45f
    };

    unsigned int groundVao, groundVbo, groundEbo;
    makeMesh(groundVertices, sizeof(groundVertices), squareIndices, sizeof(squareIndices),
             groundVao, groundVbo, groundEbo);

    unsigned int shaderProgram = makeShader(vertexSrc, fragmentSrc);

    int locOffset = glGetUniformLocation(shaderProgram, "uOffset");
    int locScale  = glGetUniformLocation(shaderProgram, "uScale");
    int locAspect = glGetUniformLocation(shaderProgram, "uAspect");
    int locTint   = glGetUniformLocation(shaderProgram, "uTint");

    float lastFrame = (float)glfwGetTime();
    float simTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        float dt = now - lastFrame;
        lastFrame = now;

        processInput(window, dt);

        simTime += dt * speed;

        float ballY = GROUND_Y + BALL_R + ballHeight(simTime);
        float sqX = squareX(simTime);
        float sqY = GROUND_Y + SQUARE_HALF;

        bool touching = isTouching(BALL_X, ballY, sqX, sqY);
        if (touching) {
            std::cout << "Соқтығысу!\n";
        }

        if (isWhiteBackground) {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            glClearColor(0.12f, 0.14f, 0.25f, 1.0f);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform1f(locAspect, aspect);

        glUniform3f(locTint, 1.0f, 1.0f, 1.0f);
        glUniform2f(locOffset, 0.0f, GROUND_Y);
        glUniform1f(locScale, 1.0f);
        glBindVertexArray(groundVao);
        glUniform1f(locAspect, 1.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glUniform1f(locAspect, aspect);

        if (touching) {
            glUniform3f(locTint, 1.0f, 0.2f, 0.2f);
        }

        glUniform2f(locOffset, sqX, sqY);
        glBindVertexArray(squareVao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glUniform2f(locOffset, BALL_X, ballY);
        glBindVertexArray(circleVao);
        glDrawElements(GL_TRIANGLES, (GLsizei)circleIndices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &circleVao);
    glDeleteBuffers(1, &circleVbo);
    glDeleteBuffers(1, &circleEbo);

    glDeleteVertexArrays(1, &squareVao);
    glDeleteBuffers(1, &squareVbo);
    glDeleteBuffers(1, &squareEbo);

    glDeleteVertexArrays(1, &groundVao);
    glDeleteBuffers(1, &groundVbo);
    glDeleteBuffers(1, &groundEbo);

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
