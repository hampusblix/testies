#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "PlayerController.hpp"
#include "Renderer.hpp"
#include "Terrain.hpp"
#include "Window.hpp"

#include <chrono>
#include <exception>
#include <iostream>

int main() {
    try {
        Window window(1280, 720, "Minimal FPS Engine");

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("Failed to initialize GLEW");
        }

        Renderer renderer(window.width(), window.height());
        Terrain terrain;
        PlayerController player;

        using clock = std::chrono::high_resolution_clock;
        auto previous = clock::now();

        while (!window.shouldClose()) {
            const auto now = clock::now();
            float dt = std::chrono::duration<float>(now - previous).count();
            previous = now;
            dt = std::min(dt, 0.033f);

            window.pollEvents();
            const InputState input = window.consumeInput();

            if (input.toggleWireframePressed) {
                renderer.toggleWireframe();
            }

            int fbWidth = 0;
            int fbHeight = 0;
            glfwGetFramebufferSize(window.handle(), &fbWidth, &fbHeight);
            renderer.resize(fbWidth, fbHeight);

            player.update(input, dt, terrain);
            renderer.render(terrain, player.viewMatrix(), player.cameraPosition());

            window.swapBuffers();
        }
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
