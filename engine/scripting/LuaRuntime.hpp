#pragma once
#include <string>
struct lua_State;
namespace aether {
class LuaRuntime final {
public:
    LuaRuntime();
    ~LuaRuntime();
    LuaRuntime(const LuaRuntime &) = delete;
    LuaRuntime &operator=(const LuaRuntime &) = delete;
    bool executeAsset(const std::string &asset_path);
private:
    lua_State *state_ = nullptr;
};
}
