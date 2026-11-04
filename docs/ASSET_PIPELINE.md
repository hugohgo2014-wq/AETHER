# Project and asset pipeline

AETHER projects are separate directories. The Android app copies new projects into its private workspace and imports ZIP projects through the Android document picker. A valid project has project.json at its root and a startScene path relative to that root.

Recommended layout:

- project.json
- scenes/
- models/
- textures/
- materials/
- audio/
- fonts/
- extensions/scripts/
- extensions/exe/
- extensions/shaders/

The current renderer reads scene geometry descriptions from JSON and supports built-in cube, sphere and plane primitives. glTF/GLB, texture/material loading and shader-pack hot reload are planned but are not part of the current runtime yet.

The extensions/exe folder contains compiled Android native modules (.so), not Windows executable files. Modules must be built for the matching NDK ABI and explicitly listed in project.json.
