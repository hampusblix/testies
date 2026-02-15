#include "Renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <array>

Renderer::Renderer(int viewportWidth, int viewportHeight) : m_width(viewportWidth), m_height(viewportHeight) {
    const char* vertexShader = R"(
        #version 410 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aUV;

        out vec3 vNormal;
        out vec3 vWorldPos;
        out vec2 vUV;

        uniform mat4 uView;
        uniform mat4 uProj;

        void main() {
            vec4 world = vec4(aPos, 1.0);
            vWorldPos = world.xyz;
            vNormal = aNormal;
            vUV = aUV;
            gl_Position = uProj * uView * world;
        }
    )";

    const char* fragmentShader = R"(
        #version 410 core
        in vec3 vNormal;
        in vec3 vWorldPos;
        in vec2 vUV;

        out vec4 FragColor;

        uniform sampler2D uGrassTex;
        uniform vec3 uLightDir;
        uniform vec3 uCameraPos;

        void main() {
            vec3 normal = normalize(vNormal);
            vec3 lightDir = normalize(-uLightDir);
            float lambert = max(dot(normal, lightDir), 0.18);

            vec3 albedo = texture(uGrassTex, vUV).rgb;
            vec3 diffuse = albedo * lambert;

            vec3 viewDir = normalize(uCameraPos - vWorldPos);
            vec3 halfDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(normal, halfDir), 0.0), 16.0) * 0.08;

            FragColor = vec4(diffuse + spec, 1.0);
        }
    )";

    m_shader = std::make_unique<Shader>(vertexShader, fragmentShader);
    createTexture();

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, m_width, m_height);
}

void Renderer::createTexture() {
    std::array<unsigned char, 16 * 16 * 3> tex{};

    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            const int i = (y * 16 + x) * 3;
            const bool check = ((x / 2 + y / 2) % 2) == 0;
            tex[i + 0] = static_cast<unsigned char>(check ? 72 : 48);
            tex[i + 1] = static_cast<unsigned char>(check ? 150 : 120);
            tex[i + 2] = static_cast<unsigned char>(check ? 66 : 44);
        }
    }

    glGenTextures(1, &m_terrainTexture);
    glBindTexture(GL_TEXTURE_2D, m_terrainTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.data());
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Renderer::resize(int width, int height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
}

void Renderer::render(const Terrain& terrain, const glm::mat4& view, const glm::vec3& cameraPos) {
    glPolygonMode(GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL);

    glClearColor(0.54f, 0.72f, 0.96f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::mat4 proj = glm::perspective(glm::radians(75.0f), static_cast<float>(m_width) / static_cast<float>(m_height), 0.1f, 500.0f);

    m_shader->use();
    m_shader->setMat4("uView", view);
    m_shader->setMat4("uProj", proj);
    m_shader->setVec3("uLightDir", glm::normalize(glm::vec3(-0.25f, -1.0f, -0.35f)));
    m_shader->setVec3("uCameraPos", cameraPos);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_terrainTexture);
    m_shader->setInt("uGrassTex", 0);

    terrain.draw();
}

void Renderer::toggleWireframe() {
    m_wireframe = !m_wireframe;
}
