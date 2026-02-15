#include "Window.hpp"

#include <GL/glew.h>

#include <stdexcept>

Window::Window(int width, int height, const char* title) : m_width(width), m_height(height) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

InputState Window::consumeInput() {
    InputState input;

    input.moveForward = glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS;
    input.moveBackward = glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS;
    input.moveLeft = glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS;
    input.moveRight = glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS;
    input.jumpHeld = glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS;
    input.sprintHeld = glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    input.crouchHeld = glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

    const bool jumpDown = input.jumpHeld;
    input.jumpPressed = jumpDown && !m_jumpWasDown;
    m_jumpWasDown = jumpDown;

    const bool wireframeDown = glfwGetKey(m_window, GLFW_KEY_F1) == GLFW_PRESS;
    input.toggleWireframePressed = wireframeDown && !m_wireframeWasDown;
    m_wireframeWasDown = wireframeDown;

    double x = 0.0;
    double y = 0.0;
    glfwGetCursorPos(m_window, &x, &y);

    static bool firstMouse = true;
    static double prevX = x;
    static double prevY = y;

    if (firstMouse) {
        prevX = x;
        prevY = y;
        firstMouse = false;
    }

    input.mouseDeltaX = static_cast<float>(x - prevX);
    input.mouseDeltaY = static_cast<float>(prevY - y);

    prevX = x;
    prevY = y;

    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }

    return input;
}

void Window::framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}
