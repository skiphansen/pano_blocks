#include "timer.h"
#include <sys/times.h>

#define WEAK __attribute__((weak))

//-----------------------------------------------------------------
// cpu_timer_get_count:
//-----------------------------------------------------------------
static inline unsigned long cpu_timer_get_count(void)
{    
    unsigned long value;
    asm volatile ("csrr %0, cycle" : "=r" (value) : );
    return value;
}
//--------------------------------------------------------------------------
// timer_init:
//--------------------------------------------------------------------------
void timer_init(void)
{

}
//--------------------------------------------------------------------------
// timer_sleep:
//--------------------------------------------------------------------------
void timer_sleep(int timeMs)
{
    t_time t = timer_now();

    while (timer_now() - t < timeMs);
}

void timer_sleep_us(int timeUs)
{
   unsigned long t = cpu_timer_get_count();
   unsigned long Timeout = timeUs * (CPU_KHZ / 1000);

    while (cpu_timer_get_count() - t < Timeout)
        ;
}

//--------------------------------------------------------------------------
// timer_now: return millisecond timer
//--------------------------------------------------------------------------
t_time timer_now(void)
{
   return cpu_timer_get_count() / CPU_KHZ;
}
//--------------------------------------------------------------------------
// times:
//--------------------------------------------------------------------------
WEAK clock_t times(struct tms *tp)
{
    t_time t = timer_now();
    if (tp)
    {
        tp->tms_utime  = t / 1000;
        tp->tms_stime  = 0;
        tp->tms_cutime = 0;
        tp->tms_cstime = 0;
    }

    return t;
}
