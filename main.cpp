#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

const float W = 800, H = 450, GROUND = 370, PX = 170, PI = 3.14159265f;
const float GRAVITY = 2600, JUMP_V = 760, JUMP2_V = 640, BAR_BOTTOM = GROUND - 58;
const float THIGH = 24, SHIN = 24, TORSO = 34, HEAD = 11, UPPER_ARM = 18, FOREARM = 17, LIMB_W = 5;

enum { BOX, BAR, GAP };
struct Obstacle { int type; float x, w, h; };
struct Pose { float hipDrop, lean, legs[2][2], arms[2][2]; };

std::vector<Obstacle> obs;
float py, pvy, speed, dist, nextGap, phase;
int jumps, best, uRect, uAngle, uKind, uColor;
bool grounded, fall, slide, dead;

float rnd() { return rand() / (float)RAND_MAX; }

void draw(int kind, float x, float y, float w, float h, float ang, float r, float g, float b) {
    glUniform4f(uRect, x, y, w, h);
    glUniform1f(uAngle, ang);
    glUniform1i(uKind, kind);
    glUniform3f(uColor, r, g, b);
    if (kind == 2) glDrawArrays(GL_TRIANGLE_FAN, 0, 34);
    else           glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
void rect(float x, float y, float w, float h, float r, float g, float b) { draw(0, x, y, w, h, 0, r, g, b); }
void circle(float x, float y, float rad, const float* c) { draw(2, x, y, rad, rad, 0, c[0], c[1], c[2]); }

void limb(float x, float y, float ang, float len, float width, const float* c, float& ex, float& ey) {
    draw(1, x, y, width, len, -ang, c[0], c[1], c[2]);
    ex = x + std::sin(ang) * len;
    ey = y + std::cos(ang) * len;
    circle(x, y, width * 0.5f, c);
    circle(ex, ey, width * 0.5f, c);
}

void reset() {
    obs.clear();
    py = pvy = dist = phase = 0; speed = 330; nextGap = 600; jumps = 0;
    grounded = true; fall = slide = dead = false;
}

void jump() {
    if (dead) { reset(); return; }
    if (fall) return;
    if (grounded) { pvy = JUMP_V; grounded = false; jumps = 1; }
    else if (jumps < 2) { pvy = JUMP2_V; jumps = 2; }
}

void spawn() {
    float lvl = std::min(1.0f, dist / 6000), r = rnd();
    Obstacle o = { BOX, W + 40, 0, 0 };
    if (r < 0.4f)       { o.w = 28 + rnd() * 26; o.h = 28 + rnd() * (26 + 20 * lvl); }
    else if (r < 0.72f) { o.type = BAR; o.w = 60 + rnd() * 60; }
    else                { o.type = GAP; o.w = 60 + rnd() * (40 + 40 * lvl); }
    obs.push_back(o);
    nextGap = speed * 2 * JUMP_V / GRAVITY + o.w + 120 + rnd() * 260 * (1.2f - lvl * 0.5f);
}

bool overGap() {
    for (const Obstacle& o : obs)
        if (o.type == GAP && PX - 6 > o.x && PX + 6 < o.x + o.w) return true;
    return false;
}

void die() {
    dead = true;
    best = std::max(best, (int)(dist / 10));
}

void update(float dt, bool down) {
    if (dead) return;
    speed += 7 * dt;
    dist += speed * dt;
    phase += dt * speed / 34;
    slide = down && grounded;

    if (!grounded) {
        pvy -= GRAVITY * dt * (down && !fall ? 2.4f : 1.0f);
        py += pvy * dt;
        if (!fall && py <= 0) {
            if (overGap()) fall = true;
            else { py = pvy = 0; grounded = true; jumps = 0; }
        }
        if (py < -140) { die(); return; }
    } else if (overGap()) {
        grounded = false; fall = true; pvy = 0;
    }

    for (Obstacle& o : obs) o.x -= speed * dt;
    obs.erase(std::remove_if(obs.begin(), obs.end(),
              [](const Obstacle& o) { return o.x + o.w < -20; }), obs.end());
    if (obs.empty() || obs.back().x <= W + 40 - nextGap) spawn();

    if (fall) return;
    float bottom = GROUND - py, top = bottom - (slide ? 34 : 96);
    float left = PX - (slide ? 22 : 10), right = PX + (slide ? 26 : 12);
    for (const Obstacle& o : obs) {
        if (!(right > o.x && left < o.x + o.w)) continue;
        if ((o.type == BOX && bottom > GROUND - o.h) || (o.type == BAR && top < BAR_BOTTOM)) {
            die();
            return;
        }
    }
}

Pose getPose() {
    float p = phase;
    if (dead && !fall)
        return { 0, -0.5f, { { 0.5f, -0.2f }, { -0.4f, -0.9f } }, { { 2.4f, 2.8f }, { -2.2f, -2.6f } } };
    if (slide)
        return { 28, -1.3f, { { 1.5f, 1.6f }, { 1.2f, 2.0f } }, { { -0.9f, -0.4f }, { 0.4f, 1.4f } } };
    if (!grounded) {
        float flip = jumps == 2 ? pvy / JUMP2_V : 0.0f;
        return { 0, 0.25f, { { 1.0f, -0.4f }, { -0.2f, -1.4f } }, { { -0.5f + flip * 0.3f, 1.3f }, { 1.6f, 2.6f } } };
    }
    float legA = 0.75f * std::sin(p), legB = 0.75f * std::sin(p + PI);
    float kneeA = legA - (0.25f + 1.25f * std::max(0.0f, std::sin(p + 1.3f)));
    float kneeB = legB - (0.25f + 1.25f * std::max(0.0f, std::sin(p + PI + 1.3f)));
    float armA = -0.85f * std::sin(p), armB = -0.85f * std::sin(p + PI);
    return { 3 * std::fabs(std::cos(p)), 0.2f, { { legA, kneeA }, { legB, kneeB } },
             { { armA, armA + 1.3f }, { armB, armB + 1.3f } } };
}

void drawStickman() {
    Pose ps = getPose();
    float hipX = PX;
    float hipY = GROUND - py - THIGH - SHIN + ps.hipDrop + (slide ? 0.0f : 2.0f);
    const float bk[3] = { 0.54f, 0.58f, 0.77f }, fr[3] = { 0.95f, 0.96f, 1.0f }, eye[3] = { 0.12f, 0.14f, 0.25f };

    float shX = hipX + std::sin(ps.lean) * (TORSO - 4);
    float shY = hipY - std::cos(ps.lean) * (TORSO - 4);
    float ex, ey, kx, ky;

    limb(hipX, hipY, ps.legs[1][0], THIGH, LIMB_W, bk, kx, ky);
    limb(kx, ky, ps.legs[1][1], SHIN, LIMB_W, bk, ex, ey);
    limb(shX, shY, ps.arms[1][0], UPPER_ARM, LIMB_W, bk, kx, ky);
    limb(kx, ky, ps.arms[1][1], FOREARM, LIMB_W, bk, ex, ey);

    float neckX, neckY;
    limb(hipX, hipY, PI - ps.lean, TORSO, LIMB_W + 1, fr, neckX, neckY);
    float headX = neckX + std::sin(ps.lean) * (HEAD + 2);
    float headY = neckY - std::cos(ps.lean) * (HEAD + 2);
    circle(headX, headY, HEAD, fr);
    circle(headX + 4 * std::cos(ps.lean), headY - 2 + 4 * std::sin(ps.lean), 2, eye);

    limb(hipX, hipY, ps.legs[0][0], THIGH, LIMB_W, fr, kx, ky);
    limb(kx, ky, ps.legs[0][1], SHIN, LIMB_W, fr, ex, ey);
    limb(shX, shY, ps.arms[0][0], UPPER_ARM, LIMB_W, fr, kx, ky);
    limb(kx, ky, ps.arms[0][1], FOREARM, LIMB_W, fr, ex, ey);
}

void drawScene() {
    rect(0, GROUND, W, H - GROUND, 0.23f, 0.26f, 0.44f);
    for (const Obstacle& o : obs) {
        if (o.type == GAP) rect(o.x, GROUND, o.w, H - GROUND, 0.04f, 0.05f, 0.11f);
        if (o.type == BOX) rect(o.x, GROUND - o.h, o.w, o.h, 0.95f, 0.55f, 0.3f);
        if (o.type == BAR) {
            rect(o.x + o.w / 2 - 3, 0, 6, BAR_BOTTOM, 0.36f, 0.4f, 0.6f);
            rect(o.x, BAR_BOTTOM - 22, o.w, 22, 0.9f, 0.3f, 0.35f);
        }
    }
    if (!(fall && py < -120)) drawStickman();
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(960, 540, "STICK RUN", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGL(glfwGetProcAddress);

    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action != GLFW_PRESS) return;
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
        if (key == GLFW_KEY_SPACE || key == GLFW_KEY_UP || key == GLFW_KEY_W) jump();
    });

    const char* vs = R"(#version 330 core
uniform vec4 uRect;
uniform float uAngle;
uniform int uKind;
void main() {
    vec2 c = vec2(gl_VertexID & 1, gl_VertexID >> 1);
    if (uKind == 1) c.x -= 0.5;
    if (uKind == 2) {
        float a = float(gl_VertexID - 1) * 6.2831853 / 32.0;
        c = gl_VertexID == 0 ? vec2(0.0) : vec2(cos(a), sin(a));
    }
    vec2 p = c * uRect.zw;
    float cs = cos(uAngle), sn = sin(uAngle);
    p = vec2(p.x * cs - p.y * sn, p.x * sn + p.y * cs) + uRect.xy;
    gl_Position = vec4(p.x / 400.0 - 1.0, 1.0 - p.y / 225.0, 0.0, 1.0);
})";
    const char* fs = R"(#version 330 core
uniform vec3 uColor;
out vec4 o;
void main() { o = vec4(uColor, 1.0); })";

    unsigned int v = glCreateShader(GL_VERTEX_SHADER), f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(v, 1, &vs, nullptr); glCompileShader(v);
    glShaderSource(f, 1, &fs, nullptr); glCompileShader(f);
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, v); glAttachShader(prog, f); glLinkProgram(prog);
    glUseProgram(prog);
    uRect = glGetUniformLocation(prog, "uRect");
    uAngle = glGetUniformLocation(prog, "uAngle");
    uKind = glGetUniformLocation(prog, "uKind");
    uColor = glGetUniformLocation(prog, "uColor");

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    reset();
    double last = glfwGetTime();
    int lastKey = -1;
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = std::min(0.033f, (float)(now - last));
        last = now;

        bool down = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ||
                    glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        update(dt, down);

        int meters = (int)(dist / 10), key = meters * 2 + dead;
        if (key != lastKey) {
            lastKey = key;
            char buf[128];
            std::snprintf(buf, sizeof(buf), dead ? "GAME OVER - %d m | Best: %d m | SPACE to retry"
                                                 : "STICK RUN | %d m | Best: %d m", meters, best);
            glfwSetWindowTitle(window, buf);
        }

        glClearColor(0.12f, 0.14f, 0.26f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        drawScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}