#if defined(_WIN32) || defined(_WIN64)

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#include "engine/platform/os.h"
#include "win32.h"

#include <stdio.h>

namespace engine
{
    struct Context
    {
        void* handle;
        void* device_context;
    };

    std::optional<Context*> os_create_context(Config& cfg)
    {
        auto cxt = new Context;

        WNDCLASSEX wc = {};
        wc.size       = sizeof(WNDCLASSEX);
        wc.style      = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.callback   = DefWindowProc;
        wc.instance   = GetModuleHandle(0);
        wc.class_name = "TestClassName";

        if (!RegisterClassEx(&wc)) { printf("reg %d", GetLastError()); return {}; }

        cxt->handle = CreateWindowEx(
            0,
            wc.class_name,
            cfg.title,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
            NULL,
            NULL,
            wc.instance,
            cxt
        );
        if (!cxt->handle) { printf("cre %d", GetLastError()); return {}; }

        return cxt;
    }

    void os_update_context(Context* cxt)
    {
        MSG msg = {0};
        while (PeekMessage(&msg, cxt->handle, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void os_delete_context(Context* cxt)
    {
        DestroyWindow(cxt->handle);
        
        delete cxt;
        cxt = nullptr;
    }
} // engine

#endif