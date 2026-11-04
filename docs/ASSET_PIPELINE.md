# Asset pipeline

Projects use a manifest and relative resource paths. Android content is packaged below app/src/main/assets/game/. The manifest startScene is relative to the game folder.

Suggested project layout:
- project.json
- scenes/
- textures/
- materials/
- models/
- audio/
- fonts/
- extensions/

The runtime reads bytes through one platform-neutral AssetStore. Android uses AAssetManager; a desktop editor will resolve the same project-relative paths from a folder. The APK should contain cooked/runtime-ready resources. Expensive import and conversion should happen in the desktop editor/build pipeline.

Integration points are staged for stb_image, fastgltf, meshoptimizer and bgfx/bimg.
