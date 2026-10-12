#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>
#include <string>

const int WIDTH  = 800;
const int HEIGHT = 600;

bool isWhiteBackground = false;
float speed = 1.5f;

int mode = 1;

bool wireframe = false;

const GLenum primitives[] = { GL_TRIANGLES, GL_LINE_LOOP, GL_LINE_STRIP, GL_POINTS };
const char*  primitiveNames[] = { "GL_TRIANGLES", "GL_LINE_LOOP", "GL_LINE_STRIP", "GL_POINTS" };
int primitiveIndex = 0;

const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform vec2 uOffset;
uniform float uScale;

out vec3 vColor;

void main() {
    gl_Position = vec4(aPos.xy * uScale + uOffset, aPos.z, 1.0);
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

void onKey(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_TAB) {
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        std::cout << "Wireframe: " << (wireframe ? "ON" : "OFF") << "\n";
    }

    if (key == GLFW_KEY_P) {
        primitiveIndex = (primitiveIndex + 1) % 4;
        std::cout << "Примитив: " << primitiveNames[primitiveIndex] << "\n";
    }

    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4) {
        mode = key - GLFW_KEY_0;
        std::cout << "Режим: " << mode << "\n";
    }

    (void)window;
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
        speed += 2.0f * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        speed -= 2.0f * dt;
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
                                          "Компьютерлік графика - 5-апта",
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "Терезе жасалмады.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSetKeyCallback(window, onKey);
    glfwSwapInterval(1);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "GLAD жүктелмеді\n";
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GPU:    " << glGetString(GL_RENDERER) << "\n";
    std::cout << "1-4: режим, Tab: wireframe, P: примитив, W/S: жылдамдық\n";

    float vertices[] = {
         0.3f,  0.3f, 0.0f,    1.0f, 0.0f, 0.0f,
         0.3f, -0.3f, 0.0f,    0.0f, 1.0f, 0.0f,
        -0.3f, -0.3f, 0.0f,    0.0f, 0.0f, 1.0f,
        -0.3f,  0.3f, 0.0f,    1.0f, 1.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int vao, vbo, ebo;
    makeMesh(vertices, sizeof(vertices), indices, sizeof(indices), vao, vbo, ebo);

    unsigned int brokenIndices[] = {
        0, 1, 2
    };

    unsigned int vaoBroken, vboBroken, eboBroken;
    makeMesh(vertices, sizeof(vertices), brokenIndices, sizeof(brokenIndices),
             vaoBroken, vboBroken, eboBroken);

    const float PI = 3.14159265f;
    const float R  = 0.3f;
    const float angles[5] = { -90.0f, -18.0f, 54.0f, 126.0f, 198.0f };
    const float colors[5][3] = {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 0.0f },
        { 1.0f, 0.0f, 1.0f }
    };

    float pentagonVertices[5 * 6];
    for (int i = 0; i < 5; ++i) {
        float a = angles[i] * PI / 180.0f;
        pentagonVertices[i * 6 + 0] = std::cos(a) * R;
        pentagonVertices[i * 6 + 1] = std::sin(a) * R;
        pentagonVertices[i * 6 + 2] = 0.0f;
        pentagonVertices[i * 6 + 3] = colors[i][0];
        pentagonVertices[i * 6 + 4] = colors[i][1];
        pentagonVertices[i * 6 + 5] = colors[i][2];
    }

    unsigned int pentagonIndices[] = {
        0, 1, 2,
        0, 2, 3,
        0, 3, 4
    };

    unsigned int vaoPent, vboPent, eboPent;
    makeMesh(pentagonVertices, sizeof(pentagonVertices),
             pentagonIndices, sizeof(pentagonIndices),
             vaoPent, vboPent, eboPent);

    unsigned int shaderProgram = makeShader(vertexSrc, fragmentSrc);

    int locOffset = glGetUniformLocation(shaderProgram, "uOffset");
    int locScale  = glGetUniformLocation(shaderProgram, "uScale");

    glPointSize(10.0f);

    float lastFrame = (float)glfwGetTime();
    float angle = 0.0f;
    int lastMode = -1;
    int lastPrimitive = -1;

    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        float dt = now - lastFrame;
        lastFrame = now;

        processInput(window, dt);

        if (mode != lastMode || primitiveIndex != lastPrimitive) {
            const char* modeNames[] = { "",
                "Базалық: төртбұрыш (EBO)",
                "1-тапсырма: индекстер {0,1,2}",
                "2-тапсырма: бесбұрыш",
                "3-тапсырма: екі объект" };
            std::string title = std::string("5-апта | ") + modeNames[mode] +
                                " | " + primitiveNames[primitiveIndex];
            glfwSetWindowTitle(window, title.c_str());
            lastMode = mode;
            lastPrimitive = primitiveIndex;
        }

        angle += speed * dt;

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

        float scaleVal = 0.75f + 0.25f * std::sin((float)glfwGetTime() * 3.0f);
        glUniform1f(locScale, scaleVal);

        GLenum prim = primitives[primitiveIndex];

        if (mode == 1) {
            glUniform2f(locOffset, std::cos(angle) * 0.4f, std::sin(angle) * 0.4f);
            glBindVertexArray(vao);
            glDrawElements(prim, 6, GL_UNSIGNED_INT, 0);
        }
        else if (mode == 2) {
            glUniform2f(locOffset, std::cos(angle) * 0.4f, std::sin(angle) * 0.4f);
            glBindVertexArray(vaoBroken);
            glDrawElements(prim, 3, GL_UNSIGNED_INT, 0);
        }
        else if (mode == 3) {
            glUniform2f(locOffset, std::cos(angle) * 0.4f, std::sin(angle) * 0.4f);
            glBindVertexArray(vaoPent);
            glDrawElements(prim, 9, GL_UNSIGNED_INT, 0);
        }
        else if (mode == 4) {
            const float r = 0.15f;
            glBindVertexArray(vao);

            glUniform2f(locOffset, -0.4f + std::cos(angle) * r,  std::sin(angle) * r);
            glDrawElements(prim, 6, GL_UNSIGNED_INT, 0);

            glUniform2f(locOffset,  0.4f + std::cos(angle) * r, -std::sin(angle) * r);
            glDrawElements(prim, 6, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glDeleteVertexArrays(1, &vaoBroken);
    glDeleteBuffers(1, &vboBroken);
    glDeleteBuffers(1, &eboBroken);

    glDeleteVertexArrays(1, &vaoPent);
    glDeleteBuffers(1, &vboPent);
    glDeleteBuffers(1, &eboPent);

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
