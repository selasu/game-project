#if defined(_WIN32) || defined(_WIN64)

#pragma comment(lib, "winmm.lib")

#include <functional>
#include <windows.h>
#include <dsound.h>

#include "../types.h"
#include "../game.h"
#include "../util/assert.h"
#include "../render/win32_render.h"

#define QUEUE_SIZE 128
#define THREAD_COUNT 8
#define MAX_PATH_LENGTH 512

// TODO(selina): Put this in another file
#define array_count(arr) ((sizeof(arr) / sizeof((arr)[0])))
#define zero_array(size, ptr) zero_size((size) * sizeof((ptr)[0]), (ptr))
void zero_size(size_t size, void* ptr)
{
    u8* byte = (u8*)ptr;
    while (--size) *byte++ = 0;
}

#define DIRECT_SOUND_CREATE(name) HRESULT __stdcall name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(DirectSoundCreate_t);

static bool running = true;

struct WorkQueueJob
{
    void* data;
    work_queue_callback_t* callback;
};

struct WorkQueue
{
    u32 volatile completed;
    u32 volatile to_complete;

    u32 volatile next_write_index;
    u32 volatile next_read_index;

    HANDLE semaphore;

    WorkQueueJob jobs[QUEUE_SIZE];
};

struct Win32SoundInfo
{
    u32 sample_index;
    u32 samples_per_second;
    u32 bytes_per_sample;

    u32 buffer_size;

    LPDIRECTSOUNDBUFFER buffer;
};

struct Win32GameCode
{
    game_update_and_render_t* update_and_render;
    game_get_sound_samples_t* get_sound_samples;
};

struct Win32Code
{
    HMODULE  dll;
    FILETIME last_write_time;

    char* dll_path;
    char* temp_path;
    char* lock_path;

    u32    function_count;
    char** function_names;
    void** functions;

    bool is_valid;
};

PLATFORM_ALLOCATE_MEMORY(win32_allocate_memory)
{
    return VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

PLATFORM_ADD_JOB(win32_add_job)
{
    u32 new_write_index = (queue->next_write_index + 1) % array_count(queue->jobs);
    ASSERT(new_write_index != queue->next_read_index);

    auto job = queue->jobs + queue->next_write_index;
    job->callback = callback;
    job->data     = data;

    ++queue->to_complete;

    queue->next_write_index = new_write_index;
    ReleaseSemaphore(queue->semaphore, 1, 0);
}

PLATFORM_LOAD_FILE(win32_read_file)
{
    void* data = 0;

    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    ASSERT(file != INVALID_HANDLE_VALUE);

    DWORD fsize = GetFileSize(file, 0);
    data = win32_allocate_memory(fsize);

    DWORD read;
    ReadFile(file, data, fsize, &read, 0);
    ASSERT(read == fsize);

    return data;
}

LRESULT __stdcall win32_callback(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // NOTE(selina): Differences between WM_CLOSE, WM_DESTROY, WM_QUIT:
    // https://stackoverflow.com/a/3155879 - 22/12/2020

    LRESULT result = 0;
    switch(msg)
    {
    case WM_CLOSE:
    {
        DestroyWindow(handle);
    } break;
    
    case WM_DESTROY:
    {
        running = false;
        PostQuitMessage(0);
    } break;

    default:
        result = DefWindowProc(handle, msg, wparam, lparam);
    }

    return result;
}

bool win32_perform_job(WorkQueue* queue)
{
    bool sleep = true;

    auto original_read = queue->next_read_index;
    long next_read     = (original_read + 1) % array_count(queue->jobs);

    if (original_read != queue->next_write_index)
    {
        auto index = InterlockedCompareExchange((long volatile*)&queue->next_read_index, next_read, original_read);
        if (index == original_read)
        {
            auto job = queue->jobs[index];
            job.callback(queue, job.data);
            InterlockedIncrement((long volatile*)&queue->completed);
        }

        sleep = false;
    }

    return sleep;
}

unsigned long __stdcall win32_thread_proc(void* param)
{
    printf("[win32] Thread #%u: Starting...\n", GetCurrentThreadId());

    auto queue = (WorkQueue*)param;

    while (1)
    {
        if (win32_perform_job(queue))
        {
            WaitForSingleObjectEx(queue->semaphore, INFINITE, 0);
        }
    }
}

void win32_clear_sound_buffer(Win32SoundInfo* sound_output)
{
    void *region1, *region2;
    DWORD region1_size, region2_size;

    if (SUCCEEDED(sound_output->buffer->Lock(0, sound_output->buffer_size, &region1, &region1_size, &region2, &region2_size, 0)))
    {
        uint8_t* output = (uint8_t*)region1;
        for (DWORD _ = 0; _ < region1_size / sound_output->bytes_per_sample; ++_) *output++ = 0;

        output = (uint8_t*)region2;
        for (DWORD _ = 0; _ < region2_size / sound_output->bytes_per_sample; ++_) *output++ = 0;

        sound_output->buffer->Unlock(region1, region1_size, region2, region2_size);
    }
}

void win32_fill_sound_buffer(Win32SoundInfo* sound_output, SoundBuffer* sound_input, DWORD to_lock, DWORD to_write)
{
    void *region1, *region2;
    DWORD region1_size, region2_size;

    if (SUCCEEDED(sound_output->buffer->Lock(to_lock, to_write, &region1, &region1_size, &region2, &region2_size, 0)))
    {
        i16* source = sound_input->samples;

        i16* output = (i16*)region1;
        for (DWORD sindex = 0; sindex < region1_size / sound_output->bytes_per_sample; ++sindex)
        {
            *output++ = *source++;
            *output++ = *source++;

            ++sound_output->sample_index;
        }

        output = (i16*)region2;
        for (DWORD sindex = 0; sindex < region2_size / sound_output->bytes_per_sample; ++sindex)
        {
            *output++ = *source++;
            *output++ = *source++;

            ++sound_output->sample_index;
        }
        sound_output->buffer->Unlock(region1, region1_size, region2, region2_size);
    }
}

FILETIME win32_get_last_write_time(char* filename)
{
    FILETIME last_write_time = {};

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &data))
    {
        last_write_time = data.ftLastWriteTime;
    }

    return last_write_time;
}

void win32_unload_code(Win32Code* code)
{
    if (code->dll)
    {
        FreeLibrary(code->dll);
        code->dll = 0;
    }

    code->is_valid = false;
    zero_array(code->function_count, code->functions);
}

void win32_load_code(Win32Code* code)
{
    WIN32_FILE_ATTRIBUTE_DATA ignored;
    if (!GetFileAttributesExA(code->lock_path, GetFileExInfoStandard, &ignored))
    {
        code->last_write_time = win32_get_last_write_time(code->dll_path);
        CopyFileA(code->dll_path, code->temp_path, FALSE);

        code->dll = LoadLibraryA(code->temp_path);
        if (code->dll)
        {
            code->is_valid = true;

            for (u32 findex = 0; findex < code->function_count && code->is_valid; ++findex)
            {
                void* f = GetProcAddress(code->dll, code->function_names[findex]);
                if (f)
                {
                    code->functions[findex] = f;
                }
                else
                {
                    code->is_valid = false;
                }
            }
        }
    }

    if (!code->is_valid)
    {
        win32_unload_code(code);
    }
}

bool win32_code_changed(Win32Code* code)
{
    FILETIME new_write_time = win32_get_last_write_time(code->dll_path);
    return CompareFileTime(&new_write_time, &code->last_write_time) != 0;
}

bool win32_try_update_code(Win32Code* code)
{
    bool changed = false;

    if (win32_code_changed(code))
    {
        win32_unload_code(code);
        for (u32 attempt = 0; !code->is_valid && attempt < 100; ++attempt)
        {
            win32_load_code(code);
            Sleep(100);
        }

        changed = true;
    }

    return changed;
}

void cat_strings(size_t a_size, char* a, size_t b_size, char* b, size_t dest_count, char* dest)
{
    for (u32 cindex = 0; cindex < a_size; ++cindex) *dest++ = *a++;
    for (u32 cindex = 0; cindex < b_size; ++cindex) *dest++ = *b++;
    *dest++ = 0;
}

i32 str_length(char* s)
{
    i32 count = 0;
    while (*s++) ++count;
    return count;
}

int main(int argc, char* argv[])
{
    char exe_path[MAX_PATH_LENGTH];
    GetModuleFileNameA(0, exe_path, sizeof(exe_path));
    char* exe_name = exe_path;
    for (char* c = exe_path; *c; ++c)
    {
        if (*c == '\\') exe_name = c + 1;
    }

    char game_file[MAX_PATH_LENGTH];
    cat_strings(exe_name - exe_path, exe_path, str_length("sel_game.dll"), "sel_game.dll", sizeof(game_file), game_file);

    char tgame_file[MAX_PATH_LENGTH];
    cat_strings(exe_name - exe_path, exe_path, str_length("sel_game_temp.dll"), "sel_game_temp.dll", sizeof(tgame_file), tgame_file);

    char ogl_file[MAX_PATH_LENGTH];
    cat_strings(exe_name - exe_path, exe_path, str_length("sel_opengl.dll"), "sel_opengl.dll", sizeof(ogl_file), ogl_file);

    char togl_file[MAX_PATH_LENGTH];
    cat_strings(exe_name - exe_path, exe_path, str_length("sel_opengl_temp.dll"), "sel_opengl_temp.dll", sizeof(togl_file), togl_file);

    char lock_file[MAX_PATH_LENGTH];
    cat_strings(exe_name - exe_path, exe_path, str_length("lock.tmp"), "lock.tmp", sizeof(lock_file), lock_file);

    LARGE_INTEGER pf;
    QueryPerformanceFrequency(&pf);
    float performance_frequency = (float)pf.QuadPart;

    bool granular = timeBeginPeriod(1) == TIMERR_NOERROR;

    WorkQueue queue = {};
    queue.semaphore = CreateSemaphoreEx(0, 0, THREAD_COUNT, 0, 0, SEMAPHORE_ALL_ACCESS);

    for (u32 i = 0; i < THREAD_COUNT; ++i)
    {
        unsigned long thread_id;
        auto handle = CreateThread(0, 0, win32_thread_proc, &queue, 0, &thread_id);
        CloseHandle(handle);
    }

    auto test_job = [](WorkQueue* queue, void* data) {    
        Sleep(1500);
        printf("[win32] Thread #%u: %s\n", GetCurrentThreadId(), (char*)data);
    };

    win32_add_job(&queue, test_job, "Test #1");
    win32_add_job(&queue, test_job, "Test #2");
    win32_add_job(&queue, test_job, "Test #3");
    win32_add_job(&queue, test_job, "Test #4");
    win32_add_job(&queue, test_job, "Test #5");

    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = win32_callback;
    wc.lpszClassName = "SelengineWindowClass";
    wc.hInstance     = GetModuleHandle(0);
    ASSERT(RegisterClassExA(&wc));

    auto handle = CreateWindowExA(0, wc.lpszClassName, "Test Title", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, wc.hInstance, 0);
    ASSERT(handle);

    auto device_context = GetDC(handle);

    // NOTE(selina): Load game game_code
    Win32GameCode game  = {};
    Win32Code game_code = {};

    game_code.dll_path  = game_file;
    game_code.temp_path = tgame_file;
    game_code.lock_path = lock_file;
    
    game_code.function_count = array_count(game_functions);
    game_code.function_names = game_functions;
    game_code.functions      = (void**)&game;

    win32_load_code(&game_code);

    // NOTE(selina): Load renderer code
    Win32RenderCode renderer = {};
    Win32Code renderer_code  = {};

    renderer_code.dll_path  = ogl_file;
    renderer_code.temp_path = togl_file;
    renderer_code.lock_path = lock_file;

    renderer_code.function_count = array_count(win32_render_functions);
    renderer_code.function_names = win32_render_functions;
    renderer_code.functions      = (void**)&renderer;

    win32_load_code(&renderer_code);

    ASSERT(game_code.is_valid && renderer_code.is_valid);

    RenderAPI* render_api = renderer.win32_load_renderer(GetDC(handle));

    // NOTE(selina): Load audio engine
    Win32SoundInfo sound_info = {};
    sound_info.samples_per_second    = 44100;
    sound_info.bytes_per_sample      = 2 * sizeof(int16_t);
    sound_info.buffer_size           = sound_info.samples_per_second * sound_info.bytes_per_sample;

    auto dsound_lib = LoadLibraryA("dsound.dll");
    if (dsound_lib)
    {
        LPDIRECTSOUND dsound;
        auto DirectSoundCreate = (DirectSoundCreate_t*)GetProcAddress(dsound_lib, "DirectSoundCreate");

        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &dsound, 0)))
        {
            WAVEFORMATEX wave_format = {};
            wave_format.wFormatTag      = WAVE_FORMAT_PCM;
            wave_format.nChannels       = 2;
            wave_format.nSamplesPerSec  = sound_info.samples_per_second;
            wave_format.wBitsPerSample  = 16;
            wave_format.nBlockAlign     = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
            wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;

            if (SUCCEEDED(dsound->SetCooperativeLevel(handle, DSSCL_PRIORITY)))
            {
                LPDIRECTSOUNDBUFFER buffer;
                DSBUFFERDESC buffer_desc = {};
                buffer_desc.dwSize  = sizeof(DSBUFFERDESC);
                buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

                if (!SUCCEEDED(dsound->CreateSoundBuffer(&buffer_desc, &buffer, 0)))
                {
                    printf("[win32] Failed to initialise primary sound buffer\n");
                }
                
                buffer_desc = {};
                buffer_desc.dwSize        = sizeof(buffer_desc);
                buffer_desc.dwFlags       = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
                buffer_desc.dwBufferBytes = sound_info.buffer_size;
                buffer_desc.lpwfxFormat   = &wave_format;

                if (!SUCCEEDED(dsound->CreateSoundBuffer(&buffer_desc, &sound_info.buffer, 0)))
                {
                    printf("[win32] Failed to initialise secondary sound buffer\n");
                }
            }
        }
    }

    // NOTE(selina): Clear the buffer before looping
    win32_clear_sound_buffer(&sound_info);
    ASSERT(SUCCEEDED(sound_info.buffer->Play(0, 0, DSBPLAY_LOOPING)));
    int16_t* samples = (int16_t*)VirtualAlloc(0, sound_info.buffer_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    SoundBuffer sound_buffer = {};
    sound_buffer.samples_per_second = sound_info.samples_per_second;
    sound_buffer.samples = samples;

    LARGE_INTEGER frequency_counter_large;
    QueryPerformanceFrequency(&frequency_counter_large);
    float frequency_counter = (float)frequency_counter_large.QuadPart;

    LARGE_INTEGER last_counter;
    QueryPerformanceCounter(&last_counter);

    float last_dt = 0.01666f;

    GameMemory game_memory = {};

    game_memory.platform_api.add_job = win32_add_job;
    game_memory.platform_api.load_file = win32_read_file;
    game_memory.platform_api.allocate_memory = win32_allocate_memory;

    while (running)
    {
        // NOTE(selina): Process system messages
        MSG msg = {};
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // NOTE(selina): Run game for frame
        if (game.update_and_render)
        {
            game.update_and_render(&game_memory);
        }
        
        f32 dt = 0.0166f;
        
        DWORD play_cursor, write_cursor;
        if (SUCCEEDED(sound_info.buffer->GetCurrentPosition(&play_cursor, &write_cursor)))
        {
            // TODO(selina): Documentation pass - comment all of the stuff in this scope block
            
            DWORD safety_bytes = (i32)((f32)(sound_info.bytes_per_sample * sound_info.samples_per_second) * dt * 1);
            safety_bytes -= safety_bytes % sound_info.bytes_per_sample;

            DWORD to_lock = (sound_info.sample_index * sound_info.bytes_per_sample) % sound_info.buffer_size;

            DWORD expected_bytes = (DWORD)((f32)(sound_info.bytes_per_sample * sound_info.samples_per_second) * dt);
            expected_bytes -= expected_bytes % sound_info.bytes_per_sample;

            DWORD expected_boundary = play_cursor + expected_bytes;

            DWORD safe_to_write = write_cursor;
            if (safe_to_write < play_cursor)
            {
                safe_to_write += sound_info.buffer_size;
            }
            else
            {
                safe_to_write += safety_bytes;
            }

            DWORD target_cursor;
            if (safe_to_write < expected_boundary)
            {
                target_cursor = expected_boundary + expected_bytes;
            }
            else
            {
                target_cursor = write_cursor + expected_bytes + safety_bytes;
            }

            target_cursor %= sound_info.buffer_size;

            DWORD to_write;
            if (to_lock > target_cursor)
            {
                to_write = sound_info.buffer_size - to_lock + target_cursor;
            }
            else
            {
                to_write = target_cursor - to_lock;
            }

            if (to_write)
            {
                sound_buffer.sample_count = to_write / sound_info.bytes_per_sample;
                ASSERT(to_write % 4 == 0);

                game.get_sound_samples(&game_memory, &sound_buffer);
                win32_fill_sound_buffer(&sound_info, &sound_buffer, to_lock, to_write);
            }
        }
        
        // NOTE(selina): Display frame
        renderer.win32_end_frame(render_api);

        if (win32_try_update_code(&game_code))
        {
            printf("[win32] Reloaded game code...\n");
            // TODO(selina): Let game know this happened
        }

        if (win32_try_update_code(&renderer_code))
        {
            printf("[win32] Reloaded renderer code...\n");
            // NOTE(selina): Should we inform the renderer about this change?
        }


        // NOTE(selina): Handle timing
        LARGE_INTEGER current_counter;
        QueryPerformanceCounter(&current_counter);

        last_dt = min(.1f, (f32)(current_counter.QuadPart - last_counter.QuadPart) / frequency_counter);
        last_counter = current_counter;
    }

    return 0;
}

#endif