#pragma once

#include <stdint.h>

namespace engine
{
    enum EventType
    {
        CLOSE,
        QUIT,
        RESIZE
    };

    struct Event
    {
        EventType event_type;

        union
        { 
            struct
            { // QUIT message
                uint32_t quit_message;
            };

            struct
            { // RESIZE message
                uint32_t height;
                uint32_t width;
            };
        };
    };
} // engine