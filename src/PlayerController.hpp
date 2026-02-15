#pragma once

#include <glm/glm.hpp>

#include "InputState.hpp"
#include "Terrain.hpp"

class PlayerController {
public:
    PlayerController();

    void update(const InputState& input, float dt, const Terrain& terrain);

    glm::vec3 cameraPosition() const;
    glm::vec3 viewDirection() const;
    glm::mat4 viewMatrix() const;

    bool isGrounded() const { return m_grounded; }

private:
    glm::vec3 m_position{0.0f, 2.0f, 0.0f};
    glm::vec3 m_velocity{0.0f};

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;

    bool m_grounded = false;
    glm::vec3 m_groundNormal{0.0f, 1.0f, 0.0f};

    bool m_sliding = false;
    glm::vec3 m_slideDirection{0.0f};
    float m_slideTimer = 0.0f;

    void applyFriction(float dt, float amount);
    void accelerate(const glm::vec3& wishDir, float wishSpeed, float accel, float dt);
};
