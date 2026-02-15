#include "PlayerController.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace {
constexpr float kMouseSensitivity = 0.11f;
constexpr float kBaseSpeed = 6.0f;
constexpr float kSprintMultiplier = 1.65f;
constexpr float kJumpSpeed = 6.8f;
constexpr float kGravity = 20.0f;
constexpr float kGroundAccel = 40.0f;
constexpr float kAirAccel = 8.0f;
constexpr float kGroundFriction = 11.0f;
constexpr float kAirFriction = 0.6f;
constexpr float kSlopeLimitDeg = 45.0f;
constexpr float kMaxSnapSpeed = 8.0f;
constexpr float kSnapDistance = 0.3f;
constexpr float kHeadHeight = 1.65f;

float radians(float deg) {
    return deg * 0.01745329252f;
}
}  // namespace

PlayerController::PlayerController() = default;

void PlayerController::applyFriction(float dt, float amount) {
    glm::vec3 horizontal = glm::vec3(m_velocity.x, 0.0f, m_velocity.z);
    const float speed = glm::length(horizontal);
    if (speed < 1e-4f) {
        return;
    }

    const float drop = speed * amount * dt;
    const float newSpeed = std::max(0.0f, speed - drop);
    horizontal *= (newSpeed / speed);
    m_velocity.x = horizontal.x;
    m_velocity.z = horizontal.z;
}

void PlayerController::accelerate(const glm::vec3& wishDir, float wishSpeed, float accel, float dt) {
    const float currentSpeed = glm::dot(glm::vec3(m_velocity.x, 0.0f, m_velocity.z), wishDir);
    const float addSpeed = wishSpeed - currentSpeed;
    if (addSpeed <= 0.0f) {
        return;
    }

    const float accelSpeed = std::min(accel * dt * wishSpeed, addSpeed);
    m_velocity.x += accelSpeed * wishDir.x;
    m_velocity.z += accelSpeed * wishDir.z;
}

void PlayerController::update(const InputState& input, float dt, const Terrain& terrain) {
    m_yaw += input.mouseDeltaX * kMouseSensitivity;
    m_pitch += input.mouseDeltaY * kMouseSensitivity;
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    const glm::vec3 forward = glm::normalize(glm::vec3(
        std::cos(radians(m_yaw)) * std::cos(radians(m_pitch)),
        0.0f,
        std::sin(radians(m_yaw)) * std::cos(radians(m_pitch))));
    const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 wishDir(0.0f);
    if (input.moveForward) wishDir += forward;
    if (input.moveBackward) wishDir -= forward;
    if (input.moveRight) wishDir += right;
    if (input.moveLeft) wishDir -= right;
    if (glm::length(wishDir) > 0.01f) {
        wishDir = glm::normalize(wishDir);
    }

    float targetSpeed = kBaseSpeed;
    if (input.sprintHeld) {
        targetSpeed *= kSprintMultiplier;
    }

    const float horizontalSpeed = glm::length(glm::vec2(m_velocity.x, m_velocity.z));

    if (m_grounded && input.sprintHeld && input.crouchHeld && horizontalSpeed > kBaseSpeed * 1.2f && !m_sliding) {
        m_sliding = true;
        m_slideDirection = horizontalSpeed > 0.01f ? glm::normalize(glm::vec3(m_velocity.x, 0.0f, m_velocity.z)) : forward;
        m_slideTimer = 0.65f;
    }

    if (m_sliding) {
        m_slideTimer -= dt;
        if (m_slideTimer <= 0.0f || !input.crouchHeld) {
            m_sliding = false;
        }
    }

    if (m_grounded) {
        applyFriction(dt, m_sliding ? 2.2f : kGroundFriction);
        if (m_sliding) {
            accelerate(m_slideDirection, targetSpeed * 1.1f, kGroundAccel * 0.45f, dt);
        } else {
            accelerate(wishDir, targetSpeed, kGroundAccel, dt);
        }

        if (input.jumpPressed) {
            m_velocity.y = kJumpSpeed;
            m_grounded = false;
        }
    } else {
        applyFriction(dt, kAirFriction);
        accelerate(wishDir, targetSpeed, kAirAccel, dt);
        m_velocity.y -= kGravity * dt;
    }

    m_position += m_velocity * dt;

    const std::optional<SurfaceHit> surface = terrain.sampleSurface(m_position.x, m_position.z);
    if (!surface.has_value()) {
        m_grounded = false;
        return;
    }

    const SurfaceHit& hit = surface.value();
    const float slopeCos = glm::dot(hit.normal, glm::vec3(0.0f, 1.0f, 0.0f));
    const float slopeAngle = std::acos(std::clamp(slopeCos, -1.0f, 1.0f));
    const bool walkable = slopeAngle < radians(kSlopeLimitDeg);

    const float verticalGap = m_position.y - hit.y;
    const bool shouldSnap = walkable && verticalGap <= kSnapDistance && m_velocity.y <= 0.0f &&
                            (horizontalSpeed < kMaxSnapSpeed || m_grounded);

    if (shouldSnap) {
        m_position.y = hit.y;
        m_velocity.y = 0.0f;
        m_grounded = true;
        m_groundNormal = hit.normal;

        const float vn = glm::dot(m_velocity, m_groundNormal);
        if (vn < 0.0f) {
            m_velocity -= vn * m_groundNormal;
        }
    } else {
        m_grounded = false;
    }

    m_position.x = std::clamp(m_position.x, -28.0f, 28.0f);
    m_position.z = std::clamp(m_position.z, -28.0f, 28.0f);
}

glm::vec3 PlayerController::cameraPosition() const {
    return m_position + glm::vec3(0.0f, kHeadHeight, 0.0f);
}

glm::vec3 PlayerController::viewDirection() const {
    const float cy = std::cos(radians(m_yaw));
    const float sy = std::sin(radians(m_yaw));
    const float cp = std::cos(radians(m_pitch));
    const float sp = std::sin(radians(m_pitch));

    return glm::normalize(glm::vec3(cy * cp, sp, sy * cp));
}

glm::mat4 PlayerController::viewMatrix() const {
    const glm::vec3 eye = cameraPosition();
    return glm::lookAt(eye, eye + viewDirection(), glm::vec3(0.0f, 1.0f, 0.0f));
}
