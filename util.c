#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "error.h"

#include "util.h"

uint64_t get_timestamp() {
    struct timespec ts;

/*
 *https://gist.github.com/jbenet/1087739
 */
#ifdef __MACH__  // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t    cclock;
    mach_timespec_t mts;
    host_get_clock_service( mach_host_self(), CALENDAR_CLOCK, &cclock );
    clock_get_time( cclock, &mts );
    mach_port_deallocate( mach_task_self(), cclock );
    ts.tv_sec  = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;
#else
    clock_gettime( CLOCK_REALTIME, &ts );
#endif

    return 1000000000L * ts.tv_sec + ts.tv_nsec;
}

