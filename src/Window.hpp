#pragma once

#include <GLFW/glfw3.h>

#include "InputState.hpp"

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();

    GLFWwindow* handle() const { return m_window; }
    int width() const { return m_width; }
    int height() const { return m_height; }

    InputState consumeInput();

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;

    bool m_jumpWasDown = false;
    bool m_wireframeWasDown = false;

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};
