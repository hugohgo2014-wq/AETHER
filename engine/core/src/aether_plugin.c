#include "aether/aether_plugin.h"
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#define AETHER_PLUGIN_LIMIT 64u
typedef struct AetherPluginSlot { const AetherPluginDescriptor *descriptor; void *module; } AetherPluginSlot;
struct AetherPluginRegistry { AetherPluginSlot slots[AETHER_PLUGIN_LIMIT]; size_t count; AetherHostApi host; };
static void host_log(AetherLogLevel level, const char *message) { aether_log(level, "%s", message != NULL ? message : ""); }
static void *host_allocate(size_t size) { return aether_alloc(size); }
static void host_deallocate(void *memory) { aether_free(memory); }
static int descriptor_valid(const AetherPluginDescriptor *d) {
    return d != NULL && d->abi_version == AETHER_PLUGIN_ABI_VERSION &&
           d->id != NULL && d->id[0] != '\0' && d->name != NULL && d->name[0] != '\0';
}
AetherPluginRegistry *aether_plugin_registry_create(void) {
    AetherPluginRegistry *r = (AetherPluginRegistry *)calloc(1, sizeof(*r));
    if (r == NULL) return NULL;
    r->host.abi_version = AETHER_PLUGIN_ABI_VERSION;
    r->host.log = host_log; r->host.allocate = host_allocate; r->host.deallocate = host_deallocate;
    return r;
}
AetherResult aether_plugin_registry_register(AetherPluginRegistry *r, const AetherPluginDescriptor *d) {
    if (r == NULL || !descriptor_valid(d)) return AETHER_ERROR_INVALID_ARGUMENT;
    if (r->count >= AETHER_PLUGIN_LIMIT) return AETHER_ERROR_CAPACITY;
    for (size_t i = 0; i < r->count; ++i)
        if (strcmp(r->slots[i].descriptor->id, d->id) == 0) return AETHER_ERROR_DUPLICATE;
    if (d->on_load != NULL && d->on_load(&r->host) == 0) return AETHER_ERROR_PLUGIN_REJECTED;
    r->slots[r->count].descriptor = d; r->slots[r->count].module = NULL; r->count++;
    aether_log(AETHER_LOG_INFO, "Extension registered: %s (%s)", d->name, d->id);
    return AETHER_OK;
}
AetherResult aether_plugin_registry_load_shared(AetherPluginRegistry *r, const char *path) {
    if (r == NULL || path == NULL || path[0] == '\0') return AETHER_ERROR_INVALID_ARGUMENT;
#if defined(__ANDROID__)
    return AETHER_ERROR_NOT_SUPPORTED;
#else
    void *module = NULL; AetherPluginQueryFn query = NULL;
#if defined(_WIN32)
    HMODULE handle = LoadLibraryA(path);
    if (handle == NULL) return AETHER_ERROR_IO;
    module = (void *)handle;
    query = (AetherPluginQueryFn)(void *)GetProcAddress(handle, AETHER_PLUGIN_QUERY_SYMBOL);
#else
    module = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (module == NULL) { aether_log(AETHER_LOG_ERROR, "dlopen failed: %s", dlerror()); return AETHER_ERROR_IO; }
    query = (AetherPluginQueryFn)dlsym(module, AETHER_PLUGIN_QUERY_SYMBOL);
#endif
    if (query == NULL) {
#if defined(_WIN32)
        FreeLibrary((HMODULE)module);
#else
        dlclose(module);
#endif
        return AETHER_ERROR_ABI_MISMATCH;
    }
    AetherResult result = aether_plugin_registry_register(r, query(AETHER_PLUGIN_ABI_VERSION));
    if (result != AETHER_OK) {
#if defined(_WIN32)
        FreeLibrary((HMODULE)module);
#else
        dlclose(module);
#endif
        return result;
    }
    r->slots[r->count - 1].module = module;
    return AETHER_OK;
#endif
}
void aether_plugin_registry_update(AetherPluginRegistry *r, float dt) {
    if (r == NULL) return;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.25f) dt = 0.25f;
    for (size_t i = 0; i < r->count; ++i)
        if (r->slots[i].descriptor->on_update != NULL)
            r->slots[i].descriptor->on_update(&r->host, dt);
}
size_t aether_plugin_registry_count(const AetherPluginRegistry *r) { return r != NULL ? r->count : 0u; }
void aether_plugin_registry_destroy(AetherPluginRegistry *r) {
    if (r == NULL) return;
    while (r->count > 0u) {
        AetherPluginSlot *slot = &r->slots[--r->count];
        if (slot->descriptor != NULL && slot->descriptor->on_unload != NULL)
            slot->descriptor->on_unload(&r->host);
        if (slot->module != NULL) {
#if defined(_WIN32)
            FreeLibrary((HMODULE)slot->module);
#else
            dlclose(slot->module);
#endif
        }
    }
    free(r);
}
