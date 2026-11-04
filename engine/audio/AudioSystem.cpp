#include "AudioSystem.hpp"
#include "aether/aether_core.h"
#include <new>
extern "C" {
#include <miniaudio.h>
}
namespace aether {
AudioSystem::~AudioSystem() { shutdown(); }
bool AudioSystem::initialize() {
    if (ready_) return true;
    auto *engine = new (std::nothrow) ma_engine{};
    if (engine == nullptr) return false;
    const ma_result result = ma_engine_init(nullptr, engine);
    if (result != MA_SUCCESS) {
        aether_log(AETHER_LOG_WARN, "Audio device unavailable (miniaudio code %d)", (int)result);
        delete engine; return false;
    }
    engine_storage_ = engine; ready_ = true;
    aether_log(AETHER_LOG_INFO, "miniaudio engine ready");
    return true;
}
void AudioSystem::shutdown() {
    if (engine_storage_ == nullptr) return;
    auto *engine = static_cast<ma_engine *>(engine_storage_);
    ma_engine_uninit(engine); delete engine; engine_storage_ = nullptr; ready_ = false;
}
}
