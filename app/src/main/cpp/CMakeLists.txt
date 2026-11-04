cmake_minimum_required(VERSION 3.22.1)
project(aether_android LANGUAGES C CXX)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
get_filename_component(AETHER_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
add_subdirectory("${AETHER_ROOT}/engine/core" "${CMAKE_CURRENT_BINARY_DIR}/aether_core")
set(EXT "${AETHER_ROOT}/external")
foreach(required entt/src/entt/entt.hpp json/single_include/nlohmann/json.hpp lua/onelua.c miniaudio/miniaudio.h stb/stb_image.h)
    if(NOT EXISTS "${EXT}/${required}")
        message(FATAL_ERROR "Missing dependency ${required}. Initialize Git submodules before building.")
    endif()
endforeach()
add_library(aether_lua STATIC "${EXT}/lua/onelua.c")
target_include_directories(aether_lua PUBLIC "${EXT}/lua")
target_compile_definitions(aether_lua PRIVATE MAKE_LIB)
set_target_properties(aether_lua PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON)
add_library(aether_sample_extension SHARED "${AETHER_ROOT}/extensions/exe/sample_extension.cpp")
target_include_directories(aether_sample_extension PRIVATE "${AETHER_ROOT}/engine/core/include")
set_target_properties(aether_sample_extension PROPERTIES CXX_VISIBILITY_PRESET default VISIBILITY_INLINES_HIDDEN OFF)
add_library(aether SHARED
    jni_bridge.cpp
    "${AETHER_ROOT}/engine/runtime/src/AssetStore.cpp"
    "${AETHER_ROOT}/engine/runtime/src/ProjectLoader.cpp"
    "${AETHER_ROOT}/engine/runtime/src/World.cpp"
    "${AETHER_ROOT}/engine/runtime/src/EngineRuntime.cpp"
    "${AETHER_ROOT}/engine/renderer/GlesRenderer.cpp"
    "${AETHER_ROOT}/engine/audio/AudioSystem.cpp"
    "${AETHER_ROOT}/engine/audio/miniaudio_impl.c"
    "${AETHER_ROOT}/engine/scripting/LuaRuntime.cpp"
    "${AETHER_ROOT}/engine/third_party/stb_image_impl.cpp"
)
target_include_directories(aether PRIVATE
    "${AETHER_ROOT}/engine/core/include"
    "${AETHER_ROOT}/engine/runtime/include"
    "${AETHER_ROOT}/engine/audio"
    "${AETHER_ROOT}/engine/scripting"
    "${AETHER_ROOT}/engine/renderer"
    "${EXT}/entt/src"
    "${EXT}/json/single_include"
    "${EXT}/lua"
    "${EXT}/miniaudio"
    "${EXT}/stb"
)
target_link_options(aether PRIVATE "-Wl,--no-as-needed")
target_link_libraries(aether PRIVATE aether_core aether_lua aether_sample_extension android log GLESv3 ${CMAKE_DL_LIBS} m)
target_compile_options(aether PRIVATE -Wall -Wextra -Wno-unused-parameter)
set_target_properties(aether PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)
option(AETHER_WITH_BGFX "Enable optional bgfx renderer backend" OFF)
option(AETHER_WITH_BOX2D "Enable optional Box2D physics library" OFF)
option(AETHER_WITH_JOLT "Enable optional Jolt 3D physics library" OFF)
if(AETHER_WITH_BGFX)
    set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BGFX_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory("${EXT}/bgfx.cmake" "${CMAKE_CURRENT_BINARY_DIR}/bgfx")
    target_link_libraries(aether PRIVATE bgfx)
endif()
if(AETHER_WITH_BOX2D)
    set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
    add_subdirectory("${EXT}/box2d" "${CMAKE_CURRENT_BINARY_DIR}/box2d")
    target_link_libraries(aether PRIVATE box2d)
endif()
if(AETHER_WITH_JOLT)
    set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
    set(OVERRIDE_CXX_FLAGS OFF CACHE BOOL "" FORCE)
    set(USE_SSE4_1 OFF CACHE BOOL "" FORCE)
    set(USE_SSE4_2 OFF CACHE BOOL "" FORCE)
    set(USE_AVX OFF CACHE BOOL "" FORCE)
    set(USE_AVX2 OFF CACHE BOOL "" FORCE)
    add_subdirectory("${EXT}/JoltPhysics/Build" "${CMAKE_CURRENT_BINARY_DIR}/jolt")
    target_link_libraries(aether PRIVATE Jolt)
endif()
