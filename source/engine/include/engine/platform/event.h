#pragma once

#include <stdint.h>

namespace engine
{
    enum EventType
    {
        // Application is currently closing
        // More info.: https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-close
        CLOSE,
        // Application is closed
        // More info.: https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-quit
        QUIT,
        // The window has been resized
        // More info.: https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-size
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