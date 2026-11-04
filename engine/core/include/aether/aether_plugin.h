#ifndef AETHER_PLUGIN_H
#define AETHER_PLUGIN_H
#include "aether_core.h"
#ifdef __cplusplus
extern "C" {
#endif
#define AETHER_PLUGIN_ABI_VERSION 1u
#define AETHER_PLUGIN_QUERY_SYMBOL "aether_plugin_query"
typedef enum AetherResult {
    AETHER_OK = 0, AETHER_ERROR_INVALID_ARGUMENT = 1,
    AETHER_ERROR_ABI_MISMATCH = 2, AETHER_ERROR_PLUGIN_REJECTED = 3,
    AETHER_ERROR_DUPLICATE = 4, AETHER_ERROR_CAPACITY = 5,
    AETHER_ERROR_NOT_SUPPORTED = 6, AETHER_ERROR_IO = 7
} AetherResult;
typedef struct AetherHostApi {
    uint32_t abi_version;
    void (*log)(AetherLogLevel level, const char *message);
    void *(*allocate)(size_t size);
    void (*deallocate)(void *memory);
} AetherHostApi;
typedef struct AetherPluginDescriptor {
    uint32_t abi_version;
    const char *id;
    const char *name;
    uint32_t version;
    int (*on_load)(const AetherHostApi *host);
    void (*on_update)(const AetherHostApi *host, float delta_seconds);
    void (*on_unload)(const AetherHostApi *host);
} AetherPluginDescriptor;
typedef const AetherPluginDescriptor *(*AetherPluginQueryFn)(uint32_t host_abi_version);
typedef struct AetherPluginRegistry AetherPluginRegistry;
AETHER_API AetherPluginRegistry *aether_plugin_registry_create(void);
AETHER_API void aether_plugin_registry_destroy(AetherPluginRegistry *registry);
AETHER_API AetherResult aether_plugin_registry_register(AetherPluginRegistry *registry, const AetherPluginDescriptor *descriptor);
AETHER_API AetherResult aether_plugin_registry_load_shared(AetherPluginRegistry *registry, const char *library_path);
AETHER_API void aether_plugin_registry_update(AetherPluginRegistry *registry, float delta_seconds);
AETHER_API size_t aether_plugin_registry_count(const AetherPluginRegistry *registry);
#ifdef __cplusplus
}
#endif
#endif
