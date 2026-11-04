#include "aether/runtime/EngineRuntime.hpp"
#include "aether/runtime/AssetStore.hpp"
#include "aether/aether_core.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
namespace aether {
EngineRuntime&EngineRuntime::instance(){static EngineRuntime runtime;return runtime;}
void EngineRuntime::initialize(void *manager,const std::string &root){
    if(initialized_){(void)loadProject(root);return;}
    aether_core_init();AssetStore::mountAndroid(manager);
    if(!audio_initialized_)audio_initialized_=audio_.initialize();
    plugins_=aether_plugin_registry_create();initialized_=true;(void)loadProject(root);
}
bool EngineRuntime::loadProject(const std::string &root){
    if(!initialized_){aether_log(AETHER_LOG_ERROR,"Runtime not initialized");return false;}
    AssetStore::setProjectRoot(root);project_=ProjectLoader::load("project.json");world_.clear();
    for(const auto&o:project_.objects)(void)world_.createEntity(o.name);
    if(plugins_!=nullptr)aether_plugin_registry_destroy(plugins_);
    plugins_=aether_plugin_registry_create();
    if(plugins_!=nullptr)for(const auto&library:project_.native_libraries){
        const auto result=aether_plugin_registry_load_shared(plugins_,(root+"/"+library).c_str());
        if(result!=AETHER_OK)aether_log(AETHER_LOG_WARN,"Native module %s rejected (code %d)",library.c_str(),(int)result);
    }
    for(const auto&script:project_.script_extensions)(void)scripting_.executeAsset(script);
    aether_log(AETHER_LOG_INFO,"Project '%s': %zu objects / %zu extensions",project_.name.c_str(),world_.entityCount(),extensionCount());
    return true;
}
void EngineRuntime::addPrimitive(const std::string &mesh){
    SceneObject object;object.mesh=(mesh=="sphere"||mesh=="plane")?mesh:"cube";
    const std::size_t index=project_.objects.size();
    object.name=object.mesh+"_"+std::to_string(index+1);
    object.position={static_cast<float>((index%5u))*1.25f-2.5f,0.0f,-static_cast<float>(index/5u)*1.25f};
    if(object.mesh=="sphere")object.color={0.20f,0.72f,0.92f,1.0f};
    else if(object.mesh=="plane"){object.color={0.13f,0.18f,0.25f,1.0f};object.scale={6.0f,1.0f,6.0f};}
    else object.color={0.43f,0.52f,0.98f,1.0f};
    project_.objects.push_back(object);(void)world_.createEntity(object.name);
    try{
        nlohmann::json scene;scene["format"]="aether.scene";scene["version"]=1;
        scene["environment"]={{"clearColor",project_.clear_color},{"grid",project_.show_grid}};
        scene["entities"]=nlohmann::json::array();
        for(const auto&o:project_.objects)scene["entities"].push_back({
            {"name",o.name},{"mesh",o.mesh},{"visible",o.visible},
            {"transform",{{"position",o.position},{"rotation",o.rotation},{"scale",o.scale}}},
            {"material",{{"color",o.color}}}
        });
        const std::filesystem::path relative(project_.start_scene);
        if(relative.is_absolute()||project_.start_scene.find("..")!=std::string::npos)throw std::runtime_error("unsafe scene path");
        const auto full=std::filesystem::path(AssetStore::projectRoot())/relative;
        std::filesystem::create_directories(full.parent_path());
        std::ofstream out(full,std::ios::binary|std::ios::trunc);out<<scene.dump(2);out.close();
        aether_log(AETHER_LOG_INFO,"Created %s and saved scene",object.name.c_str());
    }catch(const std::exception&e){aether_log(AETHER_LOG_ERROR,"Could not save scene: %s",e.what());}
}
void EngineRuntime::update(float dt){if(initialized_&&plugins_!=nullptr)aether_plugin_registry_update(plugins_,dt);}
}
