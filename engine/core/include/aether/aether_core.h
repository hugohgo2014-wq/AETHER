#ifndef AETHER_CORE_H
#define AETHER_CORE_H
#include <stddef.h>
#include <stdint.h>
#if defined(_WIN32) && defined(AETHER_CORE_SHARED)
# if defined(AETHER_CORE_BUILD)
#  define AETHER_API __declspec(dllexport)
# else
#  define AETHER_API __declspec(dllimport)
# endif
#elif defined(__GNUC__)
# define AETHER_API __attribute__((visibility("default")))
#else
# define AETHER_API
#endif
#ifdef __cplusplus
extern "C" {
#endif
#define AETHER_VERSION_MAJOR 0
#define AETHER_VERSION_MINOR 1
#define AETHER_VERSION_PATCH 0
typedef enum AetherLogLevel {
    AETHER_LOG_TRACE = 0, AETHER_LOG_INFO = 1,
    AETHER_LOG_WARN = 2, AETHER_LOG_ERROR = 3
} AetherLogLevel;
AETHER_API void aether_core_init(void);
AETHER_API void aether_core_shutdown(void);
AETHER_API void aether_log(AetherLogLevel level, const char *format, ...);
AETHER_API double aether_time_seconds(void);
AETHER_API void *aether_alloc(size_t size);
AETHER_API void *aether_realloc(void *memory, size_t size);
AETHER_API void aether_free(void *memory);
AETHER_API const char *aether_version_string(void);
#ifdef __cplusplus
}
#endif
#endif
