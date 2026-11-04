#pragma once
namespace aether {
class AudioSystem final {
public:
    AudioSystem() = default;
    ~AudioSystem();
    bool initialize();
    void shutdown();
    bool isReady() const noexcept { return ready_; }
private:
    void *engine_storage_ = nullptr;
    bool ready_ = false;
};
}
