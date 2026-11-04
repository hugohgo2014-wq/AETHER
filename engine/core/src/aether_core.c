#define _POSIX_C_SOURCE 200809L
#include "aether/aether_core.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#if defined(__ANDROID__)
#include <android/log.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
void aether_core_init(void) { aether_log(AETHER_LOG_INFO, "AETHER core %s initialized", aether_version_string()); }
void aether_core_shutdown(void) { aether_log(AETHER_LOG_INFO, "AETHER core shutdown"); }
const char *aether_version_string(void) { return "0.1.0"; }
void aether_log(AetherLogLevel level, const char *format, ...) {
    if (format == NULL) return;
    char message[1024];
    va_list args; va_start(args, format);
    (void)vsnprintf(message, sizeof(message), format, args);
    va_end(args);
#if defined(__ANDROID__)
    int priority = ANDROID_LOG_INFO;
    if (level == AETHER_LOG_TRACE) priority = ANDROID_LOG_DEBUG;
    else if (level == AETHER_LOG_WARN) priority = ANDROID_LOG_WARN;
    else if (level == AETHER_LOG_ERROR) priority = ANDROID_LOG_ERROR;
    __android_log_write(priority, "AETHER", message);
#else
    const char *tag = "INFO";
    if (level == AETHER_LOG_TRACE) tag = "TRACE";
    else if (level == AETHER_LOG_WARN) tag = "WARN";
    else if (level == AETHER_LOG_ERROR) tag = "ERROR";
    (void)fprintf(stderr, "[AETHER][%s] %s\n", tag, message);
#endif
}
double aether_time_seconds(void) {
#if defined(_WIN32)
    LARGE_INTEGER counter, frequency;
    QueryPerformanceCounter(&counter); QueryPerformanceFrequency(&frequency);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec value;
#if defined(CLOCK_MONOTONIC)
    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0) return 0.0;
#else
    if (timespec_get(&value, TIME_UTC) != TIME_UTC) return 0.0;
#endif
    return (double)value.tv_sec + (double)value.tv_nsec / 1000000000.0;
#endif
}
void *aether_alloc(size_t size) { return size == 0 ? NULL : malloc(size); }
void *aether_realloc(void *memory, size_t size) {
    if (size == 0) { free(memory); return NULL; }
    return realloc(memory, size);
}
void aether_free(void *memory) { free(memory); }
