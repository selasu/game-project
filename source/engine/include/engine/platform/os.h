#include "engine/platform/event.h"

#include <functional>
#include <stdint.h>
#include <variant>
#include <vector>

namespace engine 
{
    struct Config
    {
        const char* title;
        uint32_t width;
        uint32_t height;

        int32_t bytes_per_sample;
        int32_t samples_per_second;

        int ogl_version_major;
        int ogl_version_minor;

        uint32_t thread_count;
    };

    struct PlatformError 
    {
        uint32_t code;
        int line;
    };

    struct OSFile
    {
        void* content;
        uint32_t size;
    };

    struct SoundBuffer
    {
        int32_t samples_per_second;
        int32_t sample_count;

        int16_t* samples;
    };

    struct Context;

    struct WorkQueue;
    struct WorkQueueJob;
    typedef void(JobCallback)(WorkQueue* queue, void* data);

    std::variant<Context*, PlatformError> os_create_context(Config& cfg);
    void os_update_context(Context* cxt);
    void os_delete_context(Context* cxt);

    void os_add_job(Context* cxt, JobCallback* callback, void* data);
    void os_flush_queue(Context* cxt);

    void os_process_events(Context* cxt, std::function<void(Event)> handler);

    OSFile os_read_file(const char* file_name);
    void os_free_file(void* file);
    bool os_write_file(const char* file_name, uint64_t memory_size, void* memory);
} // engine

void test_job(engine::WorkQueue* queue, void* data);