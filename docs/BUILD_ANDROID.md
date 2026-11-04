# Android build

Pinned build matrix: JDK 17, Android Gradle Plugin 8.8.2, Gradle 8.10.2, Android SDK 35 / Build Tools 35.0.0, NDK 27.2.12479018 and CMake 3.22.1.

Clone with submodules and run gradle :app:assembleDebug. The APK is created at app/build/outputs/apk/debug/app-debug.apk. The Android workflow installs the same toolchain and uploads the debug APK as AETHER-Android-debug.

If CMake reports missing source files, initialize Git submodules. Keep AGP, Gradle, NDK and CMake updates coordinated and verify them through Actions before changing the pinned matrix.
