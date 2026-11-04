#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
namespace aether {
class AssetStore final {
public:
    static void mountAndroid(void *java_asset_manager);
    static std::vector<std::uint8_t> readBinary(std::string_view path);
    static std::string readText(std::string_view path);
};
}
