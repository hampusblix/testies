#include "Terrain.hpp"

#include <cmath>
#include <cstddef>

namespace {
float heightField(float x, float z) {
    return 0.5f * std::sin(0.22f * x) + 0.4f * std::cos(0.19f * z);
}

bool pointInTriangle2D(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c,
                       glm::vec3& barycentric) {
    const glm::vec2 v0 = b - a;
    const glm::vec2 v1 = c - a;
    const glm::vec2 v2 = p - a;

    const float d00 = glm::dot(v0, v0);
    const float d01 = glm::dot(v0, v1);
    const float d11 = glm::dot(v1, v1);
    const float d20 = glm::dot(v2, v0);
    const float d21 = glm::dot(v2, v1);

    const float denom = d00 * d11 - d01 * d01;
    if (std::abs(denom) < 1e-6f) {
        return false;
    }

    const float v = (d11 * d20 - d01 * d21) / denom;
    const float w = (d00 * d21 - d01 * d20) / denom;
    const float u = 1.0f - v - w;

    barycentric = glm::vec3(u, v, w);
    return u >= -0.0001f && v >= -0.0001f && w >= -0.0001f;
}
}  // namespace

Terrain::Terrain() {
    buildMesh();
    upload();
}

Terrain::~Terrain() {
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Terrain::buildMesh() {
    constexpr int grid = 24;
    constexpr float spacing = 2.5f;
    const float half = (grid * spacing) * 0.5f;

    m_vertices.reserve((grid + 1) * (grid + 1));

    for (int z = 0; z <= grid; ++z) {
        for (int x = 0; x <= grid; ++x) {
            const float worldX = x * spacing - half;
            const float worldZ = z * spacing - half;
            const float worldY = heightField(worldX, worldZ);

            Vertex v{};
            v.position = glm::vec3(worldX, worldY, worldZ);
            v.uv = glm::vec2(x / 4.0f, z / 4.0f);
            m_vertices.push_back(v);
        }
    }

    auto indexOf = [grid](int x, int z) {
        return static_cast<unsigned int>(z * (grid + 1) + x);
    };

    for (int z = 0; z < grid; ++z) {
        for (int x = 0; x < grid; ++x) {
            const unsigned int i0 = indexOf(x, z);
            const unsigned int i1 = indexOf(x + 1, z);
            const unsigned int i2 = indexOf(x, z + 1);
            const unsigned int i3 = indexOf(x + 1, z + 1);

            m_indices.push_back(i0);
            m_indices.push_back(i2);
            m_indices.push_back(i1);

            m_indices.push_back(i1);
            m_indices.push_back(i2);
            m_indices.push_back(i3);
        }
    }

    for (auto& v : m_vertices) {
        v.normal = glm::vec3(0.0f);
    }

    for (size_t i = 0; i < m_indices.size(); i += 3) {
        const auto ia = m_indices[i + 0];
        const auto ib = m_indices[i + 1];
        const auto ic = m_indices[i + 2];

        const glm::vec3 e1 = m_vertices[ib].position - m_vertices[ia].position;
        const glm::vec3 e2 = m_vertices[ic].position - m_vertices[ia].position;
        const glm::vec3 n = glm::normalize(glm::cross(e1, e2));

        m_vertices[ia].normal += n;
        m_vertices[ib].normal += n;
        m_vertices[ic].normal += n;
    }

    for (auto& v : m_vertices) {
        v.normal = glm::normalize(v.normal);
    }
}

void Terrain::upload() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_vertices.size() * sizeof(Vertex)), m_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_indices.size() * sizeof(unsigned int)), m_indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));

    glBindVertexArray(0);
}

void Terrain::draw() const {
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

std::optional<SurfaceHit> Terrain::sampleSurface(float x, float z) const {
    const glm::vec2 p(x, z);
    for (size_t i = 0; i < m_indices.size(); i += 3) {
        const Vertex& a = m_vertices[m_indices[i + 0]];
        const Vertex& b = m_vertices[m_indices[i + 1]];
        const Vertex& c = m_vertices[m_indices[i + 2]];

        glm::vec3 bary;
        if (!pointInTriangle2D(p, glm::vec2(a.position.x, a.position.z), glm::vec2(b.position.x, b.position.z),
                               glm::vec2(c.position.x, c.position.z), bary)) {
            continue;
        }

        SurfaceHit hit;
        hit.y = bary.x * a.position.y + bary.y * b.position.y + bary.z * c.position.y;
        hit.normal = glm::normalize(bary.x * a.normal + bary.y * b.normal + bary.z * c.normal);
        return hit;
    }

    return std::nullopt;
}
