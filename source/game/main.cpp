#include "engine/platform/os.h"

#include <stdio.h>

int main()
{
    engine::Config cfg;
    cfg.title = "Test";
    cfg.width = 720;
    cfg.height = 480;

    auto context = engine::os_create_context(cfg);
    if (!context.has_value()) 
    {
        printf("monkaW\n");
        return -1;
    }

    for (int i = 0; i < 100000000; i++) engine::os_update_context(context.value());
    engine::os_delete_context(context.value());
    
    return 0;
}