# Third-party software inventory

Dependencies are pinned as Git submodules; upstream copyright/license files remain in each checked-out module. Review each license before updating a pin or redistributing a build.

| Project | Purpose | License |
|---|---|---|
| bgfx.cmake (fetches bgfx, bx, bimg) | Optional multi-backend renderer | CC0 wrapper; bgfx BSD-2-Clause |
| Dear ImGui | Editor docking UI | MIT |
| EnTT | ECS | MIT |
| Box2D | 2D physics | MIT |
| Jolt Physics | 3D physics | MIT |
| miniaudio | Audio | Public Domain or MIT-0 |
| Lua | Embedded scripting | MIT |
| nlohmann/json | Project and scene JSON | MIT |
| stb | Image decoding | Public Domain or MIT |
| meshoptimizer | Mesh optimization | MIT |
| fastgltf | glTF/GLB import | MIT |
| ImGuizmo | 3D transform gizmos | MIT |
| imgui-node-editor | Node-graph editor | MIT |
| Recast Navigation | Navigation meshes/pathfinding | Zlib |
| Tracy | Profiling | 3-clause BSD |

All module commit IDs are fixed in the Git tree. Check each external dependency's LICENSE file when its submodule is initialized.

## Art sources

The first showcase uses procedural GLSL rather than an unreviewed downloaded texture pack. Good CC0 sources for upcoming sample projects:
- Kenney game icons and UI packs: https://kenney.nl/assets/game-icons and https://kenney.nl/support
- Poly Haven PBR textures, HDRIs and models: https://polyhaven.com/license

Check the included per-pack license when importing new artwork. Do not copy Kenney's studio logo or third-party demo renders.
