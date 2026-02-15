#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <optional>
#include <vector>

struct SurfaceHit {
    float y = 0.0f;
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
};

class Terrain {
public:
    Terrain();
    ~Terrain();

    Terrain(const Terrain&) = delete;
    Terrain& operator=(const Terrain&) = delete;

    void draw() const;
    [[nodiscard]] std::optional<SurfaceHit> sampleSurface(float x, float z) const;

private:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;

    void buildMesh();
    void upload();
};
