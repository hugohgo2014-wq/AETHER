#include "LuaRuntime.hpp"
#include "aether/runtime/AssetStore.hpp"
#include "aether/aether_core.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
namespace aether {
LuaRuntime::LuaRuntime() { state_ = luaL_newstate(); if (state_ != nullptr) luaL_openlibs(state_); }
LuaRuntime::~LuaRuntime() { if (state_ != nullptr) lua_close(state_); }
bool LuaRuntime::executeAsset(const std::string &path) {
    if (state_ == nullptr) return false;
    const std::string script = AssetStore::readText(path);
    if (script.empty()) { aether_log(AETHER_LOG_WARN, "Lua extension not found: %s", path.c_str()); return false; }
    int result = luaL_loadbuffer(state_, script.data(), script.size(), path.c_str());
    if (result == LUA_OK) result = lua_pcall(state_, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        const char *message = lua_tostring(state_, -1);
        aether_log(AETHER_LOG_ERROR, "Lua error: %s", message != nullptr ? message : "unknown");
        lua_pop(state_, 1); return false;
    }
    aether_log(AETHER_LOG_INFO, "Lua extension loaded: %s", path.c_str()); return true;
}
}
