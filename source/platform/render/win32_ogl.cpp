#if defined(_WIN32) || defined(_WIN64)

#pragma comment(lib, "opengl32.lib")

#include <windows.h>
#include <gl/gl.h>

#include <stdio.h>

#include "win32_render.h"
#include "../../util/assert.h"

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

typedef BOOL(__stdcall wglChoosePixelFormatARB_t)(HDC, const int*, const FLOAT*, UINT, int*, UINT*);
typedef HGLRC(__stdcall wglCreateContextAttribsARB_t)(HDC, HGLRC, const int*);

void* Win32Alloc(size_t size)
{
    return VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

extern "C" __declspec(dllexport) RENDER_BEGIN_FRAME(win32_begin_frame)
{

}

extern "C" __declspec(dllexport) RENDER_END_FRAME(win32_end_frame)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    SwapBuffers(wglGetCurrentDC());
}

extern "C" __declspec(dllexport) WIN32_LOAD_RENDERER(win32_load_renderer)
{
    wglChoosePixelFormatARB_t* wglChoosePixelFormatARB       = 0;
    wglCreateContextAttribsARB_t* wglCreateContextAttribsARB = 0;

    {
        WNDCLASSEXA wc = {0};
        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.lpfnWndProc   = DefWindowProc;
        wc.lpszClassName = "SelengineWGLLoader";
        wc.hInstance     = GetModuleHandle(0);
        if (!RegisterClassExA(&wc)) DEV_ASSERT(false)

        auto handle = CreateWindowExA(0, wc.lpszClassName, wc.lpszClassName, 0, 0, 0, 0, 0, 0, 0, wc.hInstance, 0);
        if (!handle) DEV_ASSERT(false)
        auto dc = GetDC(handle);

        PIXELFORMATDESCRIPTOR pfd;
        pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion     = 1;
        pfd.dwFlags      = PFD_DRAW_TO_WINDOW | LPD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType   = PFD_TYPE_RGBA;
        pfd.cColorBits   = 32;
        pfd.cDepthBits   = 24;
        pfd.cStencilBits = 8;
        pfd.iLayerType   = PFD_MAIN_PLANE;

        auto format = ChoosePixelFormat(dc, &pfd);
        if (!format || !SetPixelFormat(dc, format, &pfd)) DEV_ASSERT(false)

        auto hglrc = wglCreateContext(dc);
        if (!hglrc || !wglMakeCurrent(dc, hglrc)) DEV_ASSERT(false)

        wglChoosePixelFormatARB = (wglChoosePixelFormatARB_t*)wglGetProcAddress("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t*)wglGetProcAddress("wglCreateContextAttribsARB");

        wglMakeCurrent(dc, 0);
        wglDeleteContext(hglrc);
        ReleaseDC(handle, dc);
        DestroyWindow(handle);
    }

    // NOTE(selina): Proper OpenGL context; more info here: 
    // https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)

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
    wglChoosePixelFormatARB(device_context, format_attr, 0, 1, &format, &formatc);
    if (!formatc) DEV_ASSERT(false)

    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(device_context, format, sizeof(pfd), &pfd);
    if (!SetPixelFormat(device_context, format, &pfd)) {}

    int ogl_attr[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    auto hglrc = wglCreateContextAttribsARB(device_context, 0, ogl_attr);
    if (!hglrc || !wglMakeCurrent(device_context, hglrc)) DEV_ASSERT(false)

    // NOTE(selina): Load OpenGL functions
    auto lib = LoadLibraryA("opengl32.dll");
    auto load_function = [&lib](const char* name) {
        auto f = wglGetProcAddress(name);
        if (!f || f == (void*)0x1 || f == (void*)0x2 || f == (void*)0x3 || f == (void*)-1) f = GetProcAddress(lib, name);
        return f;
    };
    FreeLibrary(lib);

    printf("[OpenGL] %s\n", (char*)glGetString(GL_VERSION));
}

#endif