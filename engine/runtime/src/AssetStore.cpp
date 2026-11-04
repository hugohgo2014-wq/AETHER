#include "aether/runtime/AssetStore.hpp"
#include <fstream>
#include <iterator>
#include <utility>
#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif
namespace aether {
namespace {
#if defined(__ANDROID__)
AAssetManager *g_asset_manager = nullptr;
#endif
std::string g_project_root;
bool safeRelativePath(std::string_view path) {
    if (path.empty() || path.front() == '/' || path.find('\\') != std::string_view::npos) return false;
    std::size_t begin = 0;
    while (begin <= path.size()) {
        const auto end = path.find('/', begin);
        const auto part = path.substr(begin, end == std::string_view::npos ? path.size()-begin : end-begin);
        if (part == ".." || part == "." || part.empty()) return false;
        if (end == std::string_view::npos) break;
        begin = end + 1;
    }
    return true;
}
}
void AssetStore::mountAndroid(void *manager) {
#if defined(__ANDROID__)
    g_asset_manager = static_cast<AAssetManager *>(manager);
#else
    (void)manager;
#endif
}
void AssetStore::setProjectRoot(std::string root) { g_project_root = std::move(root); }
const std::string &AssetStore::projectRoot() { return g_project_root; }
std::vector<std::uint8_t> AssetStore::readBinary(std::string_view path) {
    if (safeRelativePath(path) && !g_project_root.empty()) {
        const std::string full = g_project_root + "/" + std::string(path);
        std::ifstream file(full, std::ios::binary);
        if (file) return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
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
    return {};
}
std::string AssetStore::readText(std::string_view path) {
    const auto bytes = readBinary(path);
    return std::string(bytes.begin(), bytes.end());
}
}
