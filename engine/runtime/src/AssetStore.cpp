#include "aether/runtime/AssetStore.hpp"
#include <fstream>
#include <iterator>
#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif
namespace aether {
namespace {
#if defined(__ANDROID__)
AAssetManager *g_asset_manager = nullptr;
#endif
}
void AssetStore::mountAndroid(void *manager) {
#if defined(__ANDROID__)
    g_asset_manager = static_cast<AAssetManager *>(manager);
#else
    (void)manager;
#endif
}
std::vector<std::uint8_t> AssetStore::readBinary(std::string_view path) {
#if defined(__ANDROID__)
    if (g_asset_manager != nullptr) {
        const std::string name(path);
        AAsset *asset = AAssetManager_open(g_asset_manager, name.c_str(), AASSET_MODE_BUFFER);
        if (asset == nullptr) return {};
        const auto length = AAsset_getLength64(asset);
        if (length < 0 || static_cast<unsigned long long>(length) > 256ull * 1024ull * 1024ull) {
            AAsset_close(asset); return {};
        }
        std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
        std::size_t offset = 0;
        while (offset < bytes.size()) {
            const int amount = AAsset_read(asset, bytes.data() + offset, bytes.size() - offset);
            if (amount <= 0) { AAsset_close(asset); return {}; }
            offset += static_cast<std::size_t>(amount);
        }
        AAsset_close(asset); return bytes;
    }
#endif
    std::ifstream file(std::string(path), std::ios::binary);
    if (!file) return {};
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}
std::string AssetStore::readText(std::string_view path) {
    const auto bytes = readBinary(path); return std::string(bytes.begin(), bytes.end());
}
}
