#include <jni.h>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#include "aether/runtime/EngineRuntime.hpp"
#include "GlesRenderer.hpp"
#include "aether/aether_core.h"
namespace { double last_frame_time=0.0; }
extern "C" JNIEXPORT void JNICALL Java_com_aether_engine_MainActivity_nativeOnSurfaceCreated(JNIEnv *env,jobject,jobject java_assets){
    AAssetManager *manager=AAssetManager_fromJava(env,java_assets);
    aether::EngineRuntime::instance().initialize(manager);
    if(!aether::GlesRenderer::instance().initialize())aether_log(AETHER_LOG_ERROR,"GLES renderer initialization failed");
    last_frame_time=aether_time_seconds();
}
extern "C" JNIEXPORT void JNICALL Java_com_aether_engine_MainActivity_nativeOnSurfaceChanged(JNIEnv *,jobject,jint w,jint h){
    aether::GlesRenderer::instance().resize(w,h);
}
extern "C" JNIEXPORT void JNICALL Java_com_aether_engine_MainActivity_nativeOnDrawFrame(JNIEnv *,jobject){
    double now=aether_time_seconds();float dt=(float)(now-last_frame_time);last_frame_time=now;
    if(dt<0.0f||dt>0.25f)dt=1.0f/60.0f;
    aether::EngineRuntime::instance().update(dt);aether::GlesRenderer::instance().render((float)now);
}
