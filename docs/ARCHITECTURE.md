# Architecture

The C17 core is independent from C++ and Android UI. It owns logging, a monotonic clock facade, memory helpers and the versioned extension registry. C++ systems consume that API.

The C++20 runtime owns ECS/world state (EnTT), project and scene parsing (nlohmann/json), scripting (Lua), audio (miniaudio), image decoding (stb) and the rendering interface.

On Android, Java hosts a GLSurfaceView/EGL context. JNI passes the Java AssetManager to the native runtime. AssetStore reads packaged bytes using AAssetManager_open; no WebView, HTML or project extraction is involved. The demo project is under app/src/main/assets/game/.

The first render path is OpenGL ES 3.0 and includes a procedurally generated sphere, animated star field, directional lighting, a specular term, atmospheric rim and tone mapping. bgfx is pinned as an optional renderer backend for a future multi-API render graph.

Next architecture layers: stable entity IDs, transform hierarchy, fixed-step simulation, resource handles, async streaming, PBR materials, shadow maps, post-processing, instancing/LOD, job scheduling, world partitioning, navigation, animation and editor tooling.
