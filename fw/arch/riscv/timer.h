#ifndef __TIMER_H__
#define __TIMER_H__

//-----------------------------------------------------------------
// Defines:
//-----------------------------------------------------------------
#ifndef CPU_KHZ
#define CPU_KHZ         50000
#endif

//-----------------------------------------------------------------
// Types
//-----------------------------------------------------------------
typedef unsigned long   t_time;

//-----------------------------------------------------------------
// Prototypes:
//-----------------------------------------------------------------

// General timer
void            timer_init(void);
t_time          timer_now(void); // in milliseconds since boot
static inline long timer_diff(t_time a, t_time b) { return (long)(a - b); } 
void            timer_sleep(int timeMs);
void            timer_sleep_us(int timeUs);

#endif
