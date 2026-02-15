# Minimal FPS-Style 3D Engine (Educational)

A tiny modern C++17 OpenGL project that demonstrates the core parts of a basic first-person 3D engine:

- Window + input (GLFW)
- Rendering (OpenGL core profile + GLSL)
- Math (GLM)
- Terrain mesh generation + simple texturing
- FPS-style player movement with gravity/jump/sprint/slide
- Basic static collision + slope-aware ground snapping

## Controls

- `WASD`: move relative to camera
- `Mouse`: look around (yaw/pitch, pitch clamped)
- `Space`: jump
- `Left Shift`: sprint
- `Left Ctrl` while sprinting: trigger slide
- `F1`: toggle wireframe
- `Esc`: quit

## Build

```bash
cmake -S . -B build
cmake --build build -j
```

## Run

```bash
./build/mini_fps_engine
```

## Notes on the movement model

The movement model intentionally stays compact and readable:

- Ground and air acceleration are separate.
- Friction is stronger on ground than in air.
- Jumping applies upward velocity under gravity.
- Grounding is determined by terrain normal and slope limit.
- On walkable slopes and low-to-moderate horizontal speed, the player snaps down to the plane.
- Steeper surfaces are treated as non-walkable.

The implementation is inspired by classic FPS movement ideas (Quake/Source style) in simplified form.
