#pragma once

struct WorkQueue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(WorkQueue* queue, void* data);
typedef PLATFORM_WORK_QUEUE_CALLBACK(WorkQueueCallback);

typedef void platform_add_job(WorkQueue* queue, WorkQueueCallback* callback, void* data);

typedef void* platform_load_file(const char* filename);

struct Platform
{
    platform_add_job*   add_job;
    platform_load_file* load_file;
};