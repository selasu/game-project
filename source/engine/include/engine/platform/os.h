#include <optional>
#include <stdint.h>

namespace engine 
{
    struct Context;
    struct Config
    {
        const char* title;
        uint32_t width;
        uint32_t height;
    };

    std::optional<Context*> os_create_context(Config& cfg);
    void os_update_context(Context* cxt);
    void os_delete_context(Context* cxt);
} // engine