#pragma once

struct InputState {
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool jumpHeld = false;
    bool jumpPressed = false;
    bool sprintHeld = false;
    bool crouchHeld = false;
    bool toggleWireframePressed = false;

    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;
};
