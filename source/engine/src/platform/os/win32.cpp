#if defined(_WIN32) || defined(_WIN64)

#pragma comment(lib, "opengl32.lib")

#include <stdio.h>
#include <algorithm>
#include <windows.h>
#include <dsound.h>

#include "engine/platform/os.h"
#include "engine/platform/render/ogl.h"
#include "engine/util/assert.h"

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

#define QUEUE_SIZE 128

#define array_count(arr) ((sizeof(arr) / sizeof((arr)[0])))

#define DIRECT_SOUND_CREATE(name) HRESULT __stdcall name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(DirectSoundCreate_t);

int16_t* samples   = 0;
float    update_hz = 0.0f;

struct Win32WorkQueueJob
{
    void* data;
    std::function<void(void*)> callback;
};

struct Win32WorkQueue
{
    uint32_t volatile completed;
    uint32_t volatile to_complete;

    uint32_t volatile next_write_index;
    uint32_t volatile next_read_index;

    HANDLE semaphore;

    Win32WorkQueueJob jobs[QUEUE_SIZE];
};

struct Win32SoundInfo
{
    int32_t sample_index;
    int32_t samples_per_second;
    int32_t bytes_per_sample;

    int32_t secondary_buffer_size;

    LPDIRECTSOUNDBUFFER secondary_buffer;

    float sine;
};

typedef BOOL(__stdcall wglChoosePixelFormatARB_t)(HDC, const int*, const FLOAT*, UINT, int*, UINT*);
typedef HGLRC(__stdcall wglCreateContextAttribsARB_t)(HDC, HGLRC, const int*);

LRESULT __stdcall win32_callback(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam);

bool perform_job(Win32WorkQueue* queue)
{
    bool sleep = true;

    auto original_read = queue->next_read_index;
    long next_read     = (original_read + 1) % array_count(queue->jobs);

    if (original_read != queue->next_write_index)
    {
        // Ensure this job is still available
        auto index = InterlockedCompareExchange((long volatile*)&queue->next_read_index, next_read, original_read);
        if (index == original_read)
        {
            // Launch the job and increment the completion count afterwards
            auto job = queue->jobs[index];
            job.callback(job.data);
            InterlockedIncrement((long volatile*)&queue->completed);
        }

        sleep = false;
    }

    return sleep;
}

unsigned long __stdcall win32_thread_proc(void* param)
{
    printf("[win32] Thread #%u: Starting...\n", GetCurrentThreadId());

    auto queue = (Win32WorkQueue*)param;

    while (1)
    {
        if (perform_job(queue))
        {
            // If the read index matches write index, wait for semaphore to free
            WaitForSingleObjectEx(queue->semaphore, INFINITE, 0);
        }
    }
}

void test_job(void* data)
{
    Sleep(1500);
    printf("[win32] Thread #%u: %s\n", GetCurrentThreadId(), (char*)data);
}

void clear_sound_buffer(Win32SoundInfo* sound_output)
{
    void *region1, *region2;
    DWORD region1_size, region2_size;

    if (SUCCEEDED(sound_output->secondary_buffer->Lock(0, sound_output->secondary_buffer_size, &region1, &region1_size, &region2, &region2_size, 0)))
    {
        uint8_t* output = (uint8_t*)region1;
        for (DWORD _ = 0; _ < region1_size; ++_)
        {
            *output++ = 0;
        }

        output = (uint8_t*)region2;
        for (DWORD _ = 0; _ < region2_size; ++_)
        {
            *output++ = 0;
        }

        sound_output->secondary_buffer->Unlock(region1, region1_size, region2, region2_size);
    }
}

void fill_sound_buffer(Win32SoundInfo* sound_output, engine::SoundBuffer* sound_input, DWORD to_lock, DWORD to_write)
{
    void *region1, *region2;
    DWORD region1_size, region2_size;

    if (SUCCEEDED(sound_output->secondary_buffer->Lock(to_lock, to_write, &region1, &region1_size, &region2, &region2_size, 0)))
    {
        int16_t* source = sound_input->samples;

        int16_t* output = (int16_t*)region1;
        for (DWORD sample_index = 0; sample_index < region1_size / sound_output->bytes_per_sample; ++sample_index)
        {
            *output++ = *source++;
            *output++ = *source++;
            ++sound_output->sample_index;
        }

        output = (int16_t*)region2;
        for (DWORD sample_index = 0; sample_index < region2_size / sound_output->bytes_per_sample; ++sample_index)
        {
            *output++ = *source++;
            *output++ = *source++;
            ++sound_output->sample_index;
        }

        sound_output->secondary_buffer->Unlock(region1, region1_size, region2, region2_size);
    }
}

namespace engine
{   
    struct Context
    {
        HWND  handle;
        HDC   device_context;
        HGLRC hglrc;
        
        std::vector<Event> events;

        Win32WorkQueue queue;
        Win32SoundInfo sound_info;
    };

    PlatformData* os_create_context(Config& cfg)
    {
        auto context = new Context;

        Win32WorkQueue queue = {};
        queue.semaphore = CreateSemaphoreEx(0, 0, cfg.thread_count, 0, 0, SEMAPHORE_ALL_ACCESS);
        context->queue = queue;

        for (uint32_t i = 0; i < cfg.thread_count; ++i)
        {
            unsigned long thread_id;
            auto handle = CreateThread(0, 0, win32_thread_proc, &context->queue, 0, &thread_id);
            CloseHandle(handle);
        }

        os_add_job(context, test_job, "In os_create_context 1");
        os_add_job(context, test_job, "In os_create_context 2");
        os_add_job(context, test_job, "In os_create_context 3");
        os_add_job(context, test_job, "In os_create_context 4");
        os_add_job(context, test_job, "In os_create_context 5");

        wglChoosePixelFormatARB_t* wglChoosePixelFormatARB = nullptr;
        wglCreateContextAttribsARB_t* wglCreateContextAttribsARB = nullptr;

        // Create a temporary window so that we can load the functions wglChoosePixelFormatARB and
        // wglCreateContextAttribsARB, which are needed to create a full OpenGL context
        // More info. here: https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)
        {
            WNDCLASSEXA wc = {0};
            wc.cbSize        = sizeof(WNDCLASSEX);
            wc.lpfnWndProc   = DefWindowProc;
            wc.lpszClassName = "SelengineWGLLoader";
            wc.hInstance     = GetModuleHandle(0);
            if (!RegisterClassExA(&wc)) 
            {

            }

            auto handle = CreateWindowExA(0, wc.lpszClassName, wc.lpszClassName, 0, 0, 0, 0, 0, 0, 0, wc.hInstance, 0);
            if (!handle) 
            {

            }

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
            if (!format || !SetPixelFormat(dc, format, &pfd)) 
            {

            }

            auto hglrc = wglCreateContext(dc);
            if (!hglrc || !wglMakeCurrent(dc, hglrc)) 
            {

            }

            wglChoosePixelFormatARB = (wglChoosePixelFormatARB_t*)wglGetProcAddress("wglChoosePixelFormatARB");
            wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t*)wglGetProcAddress("wglCreateContextAttribsARB");

            wglMakeCurrent(dc, 0);
            wglDeleteContext(hglrc);
            ReleaseDC(handle, dc);
            DestroyWindow(handle);
        }

        // Create our normal window now that we have the appropriate WGL functions

        WNDCLASSEXA wc = {};
        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = win32_callback;
        wc.lpszClassName = cfg.title;
        wc.hInstance     = GetModuleHandle(0);

        if (!RegisterClassExA(&wc)) 
        {

        }

        context->handle = CreateWindowExA(0, wc.lpszClassName, cfg.title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, wc.hInstance, context);
        if (!context->handle) 
        {

        }
        context->device_context = GetDC(context->handle);

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
        wglChoosePixelFormatARB(context->device_context, format_attr, 0, 1, &format, &formatc);
        if (!formatc) 
        {

        }

        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(context->device_context, format, sizeof(pfd), &pfd);
        if (!SetPixelFormat(context->device_context, format, &pfd)) {}

        int ogl_attr[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, cfg.ogl_version_major,
            WGL_CONTEXT_MINOR_VERSION_ARB, cfg.ogl_version_minor,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0,
        };

        context->hglrc = wglCreateContextAttribsARB(context->device_context, 0, ogl_attr);
        if (!context->hglrc || !wglMakeCurrent(context->device_context, context->hglrc)) 
        {

        }

        // Load OpenGL functions

        auto mod = LoadLibraryA("opengl32.dll");
        auto load_function = [&mod](const char* proc_name) {
            auto f = wglGetProcAddress(proc_name);
            if (!f || f == (void*)0x1 || f == (void*)0x2 || f == (void*)0x3 || f == (void*)-1)
            {
                f = GetProcAddress(mod, proc_name);
            }
            return f;
        };
        auto res = load_opengl_functions(load_function);
        FreeLibrary(mod);

        if (!res) 
        {

        }

        // Query the refresh rate
        int32_t refresh_rate = 60;

        DEVMODE dev_mode = {};
        dev_mode.dmSize = sizeof(DEVMODE);
        if (EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &dev_mode))
        {
            refresh_rate = (int32_t)dev_mode.dmDisplayFrequency;
        }

        update_hz = (float)refresh_rate / 2.0f;

        Win32SoundInfo sound_info = {};
        sound_info.samples_per_second    = cfg.samples_per_second;
        sound_info.bytes_per_sample      = cfg.bytes_per_sample;
        sound_info.secondary_buffer_size = sound_info.samples_per_second * sound_info.bytes_per_sample;
        context->sound_info = sound_info;

        auto dsound_lib = LoadLibraryA("dsound.dll");
        if (dsound_lib)
        {
            auto DirectSoundCreate = (DirectSoundCreate_t*)GetProcAddress(dsound_lib, "DirectSoundCreate");
            LPDIRECTSOUND dsound;

            if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &dsound, 0)))
            {
                WAVEFORMATEX wave_format = {};
                wave_format.wFormatTag      = WAVE_FORMAT_PCM;
                wave_format.nChannels       = 2;
                wave_format.nSamplesPerSec  = sound_info.samples_per_second;
                wave_format.wBitsPerSample  = 16;
                wave_format.nBlockAlign     = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
                wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;

                if (SUCCEEDED(dsound->SetCooperativeLevel(context->handle, DSSCL_PRIORITY)))
                {
                    LPDIRECTSOUNDBUFFER buffer;
                    DSBUFFERDESC buffer_desc = {};
                    buffer_desc.dwSize  = sizeof(DSBUFFERDESC);
                    buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

                    if (SUCCEEDED(dsound->CreateSoundBuffer(&buffer_desc, &buffer, 0)))
                    {
                        if (SUCCEEDED(buffer->SetFormat(&wave_format)))
                        {
                            printf("[win32] Created Primary Buffer\n");
                        }
                    }
                }

                DSBUFFERDESC buffer_desc = {};
                buffer_desc.dwSize        = sizeof(DSBUFFERDESC);
                buffer_desc.dwFlags       = DSBCAPS_GETCURRENTPOSITION2;
                buffer_desc.dwBufferBytes = sound_info.secondary_buffer_size;
                buffer_desc.lpwfxFormat   = &wave_format;

                if (SUCCEEDED(dsound->CreateSoundBuffer(&buffer_desc, &context->sound_info.secondary_buffer, 0)))
                {
                    printf("[win32] Created Secondary Buffer\n");
                }
            }
        }

        clear_sound_buffer(&context->sound_info);
        context->sound_info.secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);

        samples = (int16_t*)VirtualAlloc(0, context->sound_info.secondary_buffer_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

        auto platform_data = new PlatformData;
        platform_data->context = context;
        platform_data->sound_buffer.samples = samples;

        return platform_data;
    }

    void os_update_context(PlatformData* platform_data)
    {
        DEV_ASSERT(platform_data && platform_data->context);

        SwapBuffers(platform_data->context->device_context);

        auto sound_info = &platform_data->context->sound_info;
        DWORD play_cursor, write_cursor, to_lock, to_write;
        bool sound_is_valid = false;
        if (sound_info->secondary_buffer->GetCurrentPosition(&play_cursor, &write_cursor) == DS_OK)
        {
            DWORD target_cursor = (play_cursor + (sound_info->samples_per_second / 15 * sound_info->bytes_per_sample)) % sound_info->secondary_buffer_size;
            
            to_lock = (sound_info->sample_index * sound_info->bytes_per_sample) % sound_info->secondary_buffer_size;

            if (to_lock > target_cursor)
            {
                to_write = sound_info->secondary_buffer_size - to_lock;
                to_write += target_cursor;
            }
            else
            {
                to_write = target_cursor - to_lock;
            }

            sound_is_valid = true;
        }

        if (sound_is_valid)
        {
            fill_sound_buffer(sound_info, &platform_data->sound_buffer, to_lock, to_write);
        }

        //SoundBuffer sound_buffer = {};
        platform_data->sound_buffer.samples_per_second = sound_info->samples_per_second;
        platform_data->sound_buffer.sample_count = to_write / sound_info->bytes_per_sample;
        platform_data->sound_buffer.samples = samples;

        // Assumes all events have been handled by user
        platform_data->context->events.clear();
        MSG msg = {0};
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void os_delete_context(PlatformData* platform_data)
    {
        DEV_ASSERT(platform_data && platform_data->context);

        wglDeleteContext(platform_data->context->hglrc);
        ReleaseDC(platform_data->context->handle, platform_data->context->device_context);
        DestroyWindow(platform_data->context->handle);
        
        delete platform_data->context;
        delete platform_data;
        platform_data = nullptr;
    }

    uint32_t os_poll_error(void)
    {
        return (uint32_t)GetLastError();
    }

    void os_process_events(Context* context, std::function<void(Event)> handler)
    {
        DEV_ASSERT(context);

        std::for_each(context->events.begin(), context->events.end(), handler);
    }

    void os_add_job(Context* context, std::function<void(void*)> callback, void* data)
    {
        auto queue = &context->queue;

        uint32_t new_write_index = (queue->next_write_index + 1) % array_count(queue->jobs);
        DEV_ASSERT(new_write_index != queue->next_read_index);

        auto job = queue->jobs + queue->next_write_index;
        job->callback = callback;
        job->data     = data;

        ++queue->to_complete;

        _WriteBarrier();
        queue->next_write_index = new_write_index;
        ReleaseSemaphore(queue->semaphore, 1, 0);
    }

    void os_flush_queue(Context* context)
    {
        auto queue = &context->queue;
        while (queue->completed < queue->to_complete)
        {
            perform_job(queue);
        }

        queue->completed   = 0;
        queue->to_complete = 0;
    }

    OSFile os_read_file(const char* file_name)
    {
        printf("[win32] Attempting to read [%s]\n", file_name);
        OSFile file;

        void* handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        if (handle != INVALID_HANDLE_VALUE)
        {
            LARGE_INTEGER fsize;
            if (GetFileSizeEx(handle, &fsize))
            {
                file.content = VirtualAlloc(0, fsize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
                if (file.content)
                {
                    unsigned long bytes_read;
                    if (ReadFile(handle, file.content, (DWORD)fsize.QuadPart, &bytes_read, 0) && fsize.QuadPart == bytes_read)
                    {
                        file.size = (uint32_t)fsize.QuadPart;
                    }
                    else
                    {
                        VirtualFree(file.content, 0, MEM_RELEASE);
                        file.content = nullptr;
                    }
                }
            }

            CloseHandle(handle);
        }

        return file;
    }

    void os_free_file(OSFile file)
    {
        if (file.content)
        {
            VirtualFree(file.content, 0, MEM_RELEASE);
            file.content = nullptr;
        }
    }

    bool os_write_file(const char* file_name, uint64_t memory_size, void* memory)
    {
        bool result = false;

        void* handle = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
        if (handle != INVALID_HANDLE_VALUE)
        {
            unsigned long bytes_written;
            if (WriteFile(handle, memory, (DWORD)memory_size, &bytes_written, 0))
            {
                result = bytes_written == memory_size;
            }

            CloseHandle(handle);
        }

        return result;
    }
} // engine

LRESULT __stdcall win32_callback(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
    using namespace engine;

    // Attempt to load the context; if it can't be found, then we can't do any operations with
    // most of the WM messages, so we simply pass back to the default procedure on failure
    Context* context = nullptr;
    if (msg != WM_CREATE)
    {
        auto ptr = GetWindowLongPtr(handle, GWLP_USERDATA);
        context = reinterpret_cast<Context*>(ptr);

        if (!context)
        {
            return DefWindowProc(handle, msg, wparam, lparam);
        }
    }

    // NOTE(Selina): Differences between WM_CLOSE, WM_DESTROY, WM_QUIT:
    // https://stackoverflow.com/a/3155879 - 22/12/2020

    LRESULT result = 0;
    switch(msg)
    {
    case WM_CREATE: 
    {
        // Load the context from the parameters and save it into a long pointer we can call later
        auto cs = reinterpret_cast<CREATESTRUCT*>(lparam);
        context = reinterpret_cast<Context*>(cs->lpCreateParams);
        SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)context);
    } break;

    case WM_CLOSE:
    {
        // Application going to close event
        Event e;
        e.event_type = EventType::CLOSE;

        context->events.push_back(e);

        // Since we aren't actually handling this event yet, just call destroy window can be
        // processed. Removing this means the application won't close.
        DestroyWindow(handle);
        
        // TODO(Selina): Add a way for window destruction to be called via function. Maybe
        // this should be relegated to os_delete_context? Would just mean thinking where it goes. - 22/12/2020
    } break;
    
    case WM_DESTROY:
    {
        // Application closing event
        Event e;
        e.event_type   = EventType::QUIT;
        e.quit_message = (uint32_t)wparam;
        context->events.push_back(e);

        PostQuitMessage(0);
    } break;

    case WM_SIZE:
    {
        // Window resized event
        Event e;
        e.event_type = EventType::RESIZE;
        e.width      = lparam & 0xFFFF;
        e.height     = (lparam >> 16) & 0xFFFF;

        context->events.push_back(e);
    };

    default:
        // Send all unhandled messages through the default procecdure
        result = DefWindowProc(handle, msg, wparam, lparam);
    }
    return result;
}

#endif