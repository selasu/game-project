#include "event.h"

#include <functional>
#include <stdint.h>

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

    struct PlatformData
    {
        SoundBuffer sound_buffer;
        Context*    context;

        float time_elapsed;
    };

    PlatformData* os_create_context(Config& cfg);
    void os_update_context(PlatformData* platform_data);
    void os_delete_context(PlatformData* platform_data);

    void* os_get_memory(uint32_t size);

    uint32_t os_poll_error(void);

    void os_add_job(Context* context, std::function<void(void*)> callback, void* data);
    void os_flush_queue(Context* context);

    void os_process_events(Context* context, std::function<void(Event)> handler);

    OSFile os_read_file(const char* file_name);
    void os_free_file(void* file);
    bool os_write_file(const char* file_name, uint64_t memory_size, void* memory);
} // engine