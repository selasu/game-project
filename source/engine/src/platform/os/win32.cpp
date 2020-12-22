#if defined(_WIN32) || defined(_WIN64)

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "opengl32.lib")

#include "engine/platform/os.h"
#include "engine/platform/render/ogl.h"
#include "engine/util/assert.h"
#include "win32.h"

#include <stdio.h>
#include <algorithm>

// See https://www.opengl.org/registry/specs/ARB/wgl_create_context.txt for all values
#define WGL_CONTEXT_MAJOR_VERSION_ARB    0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB    0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB     0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

// See https://www.opengl.org/registry/specs/ARB/wgl_pixel_format.txt for all values
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB   0x2003
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB  0x2011
#define WGL_PIXEL_TYPE_ARB     0x2013
#define WGL_COLOR_BITS_ARB     0x2014
#define WGL_DEPTH_BITS_ARB     0x2022
#define WGL_STENCIL_BITS_ARB   0x2023

#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_TYPE_RGBA_ARB         0x202B

#define return_os_error {\
    PlatformError e; \
    e.code = GetLastError(); \
    e.line = __LINE__; \
    return e; }

typedef int(__stdcall wglChoosePixelFormatARB_t)(void*, const int*, const float*, unsigned int, int*, unsigned int*);
typedef void*(__stdcall wglCreateContextAttribsARB_t)(void*, void*, const int*);

long_ptr __stdcall win32_callback(void* handle, unsigned int msg, uint_ptr wparam, long_ptr lparam);

namespace engine
{
    struct Context
    {
        void* handle;
        void* device_context;
        void* hglrc;

        std::vector<Event> events;
    };

    std::variant<Context*, PlatformError> os_create_context(Config& cfg)
    {
        wglChoosePixelFormatARB_t* wglChoosePixelFormatARB = nullptr;
        wglCreateContextAttribsARB_t* wglCreateContextAttribsARB = nullptr;

        // Create a temporary window so that we can load the functions wglChoosePixelFormatARB and
        // wglCreateContextAttribsARB, which are needed to create a full OpenGL context
        // More info. here: https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)
        {
            WNDCLASSEX wc = {0};
            wc.size       = sizeof(WNDCLASSEX);
            wc.callback   = DefWindowProc;
            wc.class_name = "TempClass";
            wc.instance   = GetModuleHandle(0);
            if (!RegisterClassEx(&wc)) return_os_error

            auto handle = CreateWindowEx(0, wc.class_name, wc.class_name, 0, 0, 0, 0, 0, 0, 0, wc.instance, 0);
            if (!handle) return_os_error

            auto dc = GetDC(handle);

            PIXELFORMATDESCRIPTOR pfd;
            pfd.size         = sizeof(PIXELFORMATDESCRIPTOR);
            pfd.version      = 1;
            pfd.flags        = PFD_DRAW_TO_WINDOW | LPD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            pfd.pixel_type   = PFD_TYPE_RGBA;
            pfd.colour_bits  = 32;
            pfd.depth_bits   = 24;
            pfd.stencil_bits = 8;
            pfd.layer_type   = PFD_MAIN_PLANE;

            auto format = ChoosePixelFormat(dc, &pfd);
            if (!format || !SetPixelFormat(dc, format, &pfd)) return_os_error

            auto hglrc = wglCreateContext(dc);
            if (!hglrc || !wglMakeCurrent(dc, hglrc)) return_os_error

            wglChoosePixelFormatARB = (wglChoosePixelFormatARB_t*)wglGetProcAddress("wglChoosePixelFormatARB");
            wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t*)wglGetProcAddress("wglCreateContextAttribsARB");

            wglMakeCurrent(dc, 0);
            wglDeleteContext(hglrc);
            ReleaseDC(handle, dc);
            DestroyWindow(handle);
        }

        // Create our normal window now that we have the appropriate WGL functions

        WNDCLASSEX wc = {};
        wc.size       = sizeof(WNDCLASSEX);
        wc.style      = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.callback   = win32_callback;
        wc.instance   = GetModuleHandle(0);
        wc.class_name = cfg.title;

        if (!RegisterClassEx(&wc)) return_os_error

        auto cxt = new Context;
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
        if (!cxt->handle) return_os_error
        cxt->device_context = GetDC(cxt->handle);

        // Create a proper OpenGL context
        // More info. here: https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)

        int format_attr[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB,     32,
            WGL_DEPTH_BITS_ARB,     24,
            WGL_STENCIL_BITS_ARB,   8,
            0
        };

        int format;
        unsigned int formatc;
        wglChoosePixelFormatARB(cxt->device_context, format_attr, 0, 1, &format, &formatc);
        if (!formatc) return_os_error

        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(cxt->device_context, format, sizeof(pfd), &pfd);
        if (!SetPixelFormat(cxt->device_context, format, &pfd)) {}

        int ogl_attr[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, cfg.ogl_version_major,
            WGL_CONTEXT_MINOR_VERSION_ARB, cfg.ogl_version_minor,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0,
        };

        cxt->hglrc = wglCreateContextAttribsARB(cxt->device_context, 0, ogl_attr);
        if (!cxt->hglrc || !wglMakeCurrent(cxt->device_context, cxt->hglrc)) return_os_error

        void* mod = LoadLibrary("opengl32.dll");
        load_opengl_functions(wglGetProcAddress);
        FreeLibrary(mod);

        return cxt;
    }

    void os_update_context(Context* cxt)
    {
        DEV_ASSERT(cxt);

        // Assumes all events have been handled by user
        cxt->events.clear();

        SwapBuffers(cxt->device_context);

        // Process system messages for new events
        MSG msg = {0};
        while (PeekMessage(&msg, cxt->handle, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void os_delete_context(Context* cxt)
    {
        DEV_ASSERT(cxt);

        wglMakeCurrent(cxt->device_context, 0);
        wglDeleteContext(cxt->hglrc);
        ReleaseDC(cxt->handle, cxt->device_context);
        DestroyWindow(cxt->handle);
        
        delete cxt;
        cxt = nullptr;
    }

    void os_process_events(Context* cxt, std::function<void(Event)> handler)
    {
        DEV_ASSERT(cxt);

        std::for_each(cxt->events.begin(), cxt->events.end(), handler);
    }
} // engine

long_ptr __stdcall win32_callback(void* handle, unsigned int msg, uint_ptr wparam, long_ptr lparam)
{
    using namespace engine;

    // Attempt to load the context; if it can't be found, then we can't do any operations with
    // most of the WM messages, so we simply pass back to the default procedure on failure
    Context* cxt = nullptr;
    if (msg != WM_CREATE)
    {
        auto ptr = GetWindowLongPtr(handle, GWLP_USERDATA);
        cxt = reinterpret_cast<Context*>(ptr);

        if (!cxt)
        {
            return DefWindowProc(handle, msg, wparam, lparam);
        }
    }

    // NOTE(Selina): Differences between WM_CLOSE, WM_DESTROY, WM_QUIT:
    // https://stackoverflow.com/a/3155879 - 22/12/2020

    long_ptr result = 0;
    switch(msg)
    {
    case WM_CREATE: 
    {
        // Load the context from the parameters and save it into a long pointer we can call later
        auto cs = reinterpret_cast<CREATESTRUCT*>(lparam);
        cxt = reinterpret_cast<Context*>(cs->create_params);
        SetWindowLongPtr(handle, GWLP_USERDATA, (long_ptr)cxt);
    } break;

    case WM_CLOSE:
    {
        // Application going to close event
        Event e;
        e.event_type = EventType::CLOSE;

        cxt->events.push_back(e);

        // Since we aren't actually handling this event yet, just call destroy window can be
        // processed. Removing this means the application won't close.
        DestroyWindow(handle);
        
        // TODO(Selina): Add a way for window destruction to be called via function. Maybe
        // this should be relegated to os_delete_context? Would just mean thinking where it goes. - 22/12/2020
    } break;
    
    case WM_DESTROY:
    {
        // Application closing event
        PostQuitMessage(0);
    } break;

    case WM_QUIT:
    {
        // Application closed event
        Event e;
        e.event_type   = EventType::QUIT;
        e.quit_message = (uint32_t)wparam;

        cxt->events.push_back(e);
    } break;

    case WM_SIZE:
    {
        // Window resized event
        Event e;
        e.event_type = EventType::RESIZE;
        e.width      = lparam & 0xFFFF;
        e.height     = (lparam >> 16) & 0xFFFF;

        cxt->events.push_back(e);
    };

    default:
        // Send all unhandled messages through the default procecdure
        result = DefWindowProc(handle, msg, wparam, lparam);
    }
    return result;
}

#endif