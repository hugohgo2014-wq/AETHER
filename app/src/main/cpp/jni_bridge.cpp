#include <jni.h>
#include <android/asset_manager_jni.h>
#include "aether/runtime/EngineRuntime.hpp"
#include "GlesRenderer.hpp"
#include "aether/aether_core.h"
#include <string>
#include <cstdio>
namespace {
double last_frame_time=0.0;
float fps_value=0.0f;
bool debug_tools=false;
std::string toString(JNIEnv *env,jstring value){
    if(value==nullptr)return {};
    const char *chars=env->GetStringUTFChars(value,nullptr);
    if(chars==nullptr)return {};
    std::string result(chars);env->ReleaseStringUTFChars(value,chars);return result;
}
}
extern "C" JNIEXPORT void JNICALL Java_com_aether_engine_MainActivity_nativeOnSurfaceCreated(JNIEnv *env,jobject,jobject assets,jstring root){
    AAssetManager *manager=AAssetManager_fromJava(env,assets);
    aether::EngineRuntime::instance().initialize(manager,toString(env,root));
    if(!aether::GlesRenderer::instance().initialize())aether_log(AETHER_LOG_ERROR,"GLES renderer initialization failed");
    last_frame_time=aether_time_seconds();
}
extern "C" JNIEXPORT void JNICALL Java_com_aether_engine_MainActivity_nativeOnSurfaceChanged(JNIEnv *,jobject,jint w,jint h){
    aether::GlesRenderer::instance().resize(w,h);
}
extern "C" JNIEXPORT void JNICALL Java_com_aether_engine_MainActivity_nativeOnDrawFrame(JNIEnv *,jobject){
    const double now=aether_time_seconds();float dt=(float)(now-last_frame_time);last_frame_time=now;
    if(dt<=0.0f||dt>0.25f)dt=1.0f/60.0f;
    fps_value=fps_value==0.0f?1.0f/dt:fps_value*0.90f+(1.0f/dt)*0.10f;
    auto &runtime=aether::EngineRuntime::instance();runtime.update(dt);
    aether::GlesRenderer::instance().render(runtime.project(),(float)now,debug_tools);
}
extern "C" JNIEXPORT void JNICALL Java_com_aether_engine_MainActivity_nativeLoadProject(JNIEnv *env,jobject,jstring root){
    const bool ok=aether::EngineRuntime::instance().loadProject(toString(env,root));
    if(!ok)aether_log(AETHER_LOG_ERROR,"Project reload failed");
}
extern "C" JNIEXPORT void JNICALL Java_com_aether_engine_MainActivity_nativeSetDebug(JNIEnv *,jobject,jboolean enabled){
    debug_tools=enabled==JNI_TRUE;
}
extern "C" JNIEXPORT void JNICALL Java_com_aether_engine_MainActivity_nativeAddPrimitive(JNIEnv *env,jobject,jstring mesh){
    aether::EngineRuntime::instance().addPrimitive(toString(env,mesh));
}
extern "C" JNIEXPORT jstring JNICALL Java_com_aether_engine_MainActivity_nativeGetStats(JNIEnv *env,jobject){
    char buffer[256];
    const auto &runtime=aether::EngineRuntime::instance();
    std::snprintf(buffer,sizeof(buffer),"FPS %.1f   |   Objetos %zu   |   Draw calls %d   |   Plugins %zu",
        fps_value,runtime.world().entityCount(),aether::GlesRenderer::instance().drawCalls(),runtime.extensionCount());
    return env->NewStringUTF(buffer);
}
