#if defined(_WIN32) || defined(_WIN64)

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "opengl32.lib")

#include "engine/platform/os.h"
#include "engine/platform/render/ogl.h"
#include "win32.h"

#include <stdio.h>

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

#define OPENGL_VERSION_MAJOR 3
#define OPENGL_VERSION_MINOR 3

#define return_os_error(line_no) {\
    PlatformError e; \
    e.code = GetLastError(); \
    e.line = line_no; \
    return e; }

typedef int(__stdcall wglChoosePixelFormatARB_t)(void*, const int*, const float*, unsigned int, int*, unsigned int*);
typedef void*(__stdcall wglCreateContextAttribsARB_t)(void*, void*, const int*);

namespace engine
{
    struct Context
    {
        void* handle;
        void* device_context;
        void* hglrc;
    };

    std::variant<Context*, PlatformError> os_create_context(Config& cfg)
    {
        wglChoosePixelFormatARB_t* wglChoosePixelFormatARB = nullptr;
        wglCreateContextAttribsARB_t* wglCreateContextAttribsARB = nullptr;

        {
            WNDCLASSEX wc = {0};
            wc.size       = sizeof(WNDCLASSEX);
            wc.callback   = DefWindowProc;
            wc.class_name = "TempClass";
            wc.instance   = GetModuleHandle(0);
            if (!RegisterClassEx(&wc)) return_os_error(__LINE__);

            auto handle = CreateWindowEx(0, wc.class_name, wc.class_name, 0, 0, 0, 0, 0, 0, 0, wc.instance, 0);
            if (!handle) return_os_error(__LINE__);

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
            if (!format || !SetPixelFormat(dc, format, &pfd)) return_os_error(__LINE__);

            auto hglrc = wglCreateContext(dc);
            if (!hglrc || !wglMakeCurrent(dc, hglrc)) return_os_error(__LINE__);

            wglChoosePixelFormatARB = (wglChoosePixelFormatARB_t*)wglGetProcAddress("wglChoosePixelFormatARB");
            wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t*)wglGetProcAddress("wglCreateContextAttribsARB");

            wglMakeCurrent(dc, 0);
            wglDeleteContext(hglrc);
            ReleaseDC(handle, dc);
            DestroyWindow(handle);
        }

        auto cxt = new Context;

        WNDCLASSEX wc = {};
        wc.size       = sizeof(WNDCLASSEX);
        wc.style      = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.callback   = DefWindowProc;
        wc.instance   = GetModuleHandle(0);
        wc.class_name = cfg.title;

        if (!RegisterClassEx(&wc)) return_os_error(__LINE__);

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
        if (!cxt->handle) return_os_error(__LINE__);
        cxt->device_context = GetDC(cxt->handle);

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
        if (!formatc) return_os_error(__LINE__);

        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(cxt->device_context, format, sizeof(pfd), &pfd);
        if (!SetPixelFormat(cxt->device_context, format, &pfd)) {}

        int ogl_attr[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, OPENGL_VERSION_MAJOR,
            WGL_CONTEXT_MINOR_VERSION_ARB, OPENGL_VERSION_MINOR,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0,
        };

        cxt->hglrc = wglCreateContextAttribsARB(cxt->device_context, 0, ogl_attr);
        if (!cxt->hglrc || !wglMakeCurrent(cxt->device_context, cxt->hglrc)) return_os_error(__LINE__);

        void* mod = LoadLibrary("opengl32.dll");
        load_opengl_functions(wglGetProcAddress);
        FreeLibrary(mod);

        return cxt;
    }

    void os_update_context(Context* cxt)
    {
        SwapBuffers(cxt->device_context);

        MSG msg = {0};
        while (PeekMessage(&msg, cxt->handle, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void os_delete_context(Context* cxt)
    {
        wglMakeCurrent(cxt->device_context, 0);
        wglDeleteContext(cxt->hglrc);
        ReleaseDC(cxt->handle, cxt->device_context);
        DestroyWindow(cxt->handle);
        
        delete cxt;
        cxt = nullptr;
    }
} // engine

#endif