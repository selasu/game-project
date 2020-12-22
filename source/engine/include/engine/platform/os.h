#include "engine/platform/event.h"

#include <functional>
#include <stdint.h>
#include <variant>
#include <vector>

namespace engine 
{
    struct Context;
    struct Config
    {
        const char* title;
        uint32_t width;
        uint32_t height;

        int ogl_version_major;
        int ogl_version_minor;
    };

    struct PlatformError 
    {
        uint32_t code;
        int line;
    };

    std::variant<Context*, PlatformError> os_create_context(Config& cfg);
    void os_update_context(Context* cxt);
    void os_delete_context(Context* cxt);

    void os_process_events(Context* cxt, std::function<void(Event)> handler);
} // engine