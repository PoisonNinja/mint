#include <kernel/time/time.h>

void tick_handler(void)
{
    jiffy_increase();
    time_update();
}
