#pragma once

struct WorkQueue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(WorkQueue* queue, void* data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(work_queue_callback_t);

#define PLATFORM_ADD_JOB(name) void name(WorkQueue* queue, work_queue_callback_t* callback, void* data)
typedef PLATFORM_ADD_JOB(platform_add_job_t);

#define PLATFORM_LOAD_FILE(name) void* name(const char* filename)
typedef PLATFORM_LOAD_FILE(platform_load_file_t);

#define PLATFORM_ALLOCATE_MEMORY(name) void* name(int size)
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory_t);

struct PlatformAPI
{
    platform_add_job_t*         add_job;
    platform_load_file_t*       load_file;
    platform_allocate_memory_t* allocate_memory;
};

struct GameMemory
{
    struct Game* game_state;

    PlatformAPI platform_api;

    WorkQueue* work_queue;
};