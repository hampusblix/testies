#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <memory>

#include "Shader.hpp"
#include "Terrain.hpp"

class Renderer {
public:
    Renderer(int viewportWidth, int viewportHeight);

    void resize(int width, int height);
    void render(const Terrain& terrain, const glm::mat4& view, const glm::vec3& cameraPos);
    void toggleWireframe();

private:
    int m_width = 0;
    int m_height = 0;

    bool m_wireframe = false;
    GLuint m_terrainTexture = 0;

    std::unique_ptr<Shader> m_shader;

    void createTexture();
};
