#include "core/time.h"

#include "platform.hpp"
#include <ctime>

static time_millis current_time;

time_millis time_get_millis()
{
    return current_time;
}

void time_set_millis(time_millis millis)
{
    current_time = millis;
}


Time Time::current()
{
    tm d;

#if defined(JULIUS_PLATFORM_WIN)
    _getsystime( &d );
#elif defined(JULIUS_PLATFORM_UNIX) || defined(JULIUS_PLATFORM_HAIKU)
    time_t rawtime;
    ::time( &rawtime );

    d = *localtime( &rawtime );
#endif //JULIUS_PLATFORM_UNIX

    return Time{ d.tm_year+1900, d.tm_mon, d.tm_mday, d.tm_hour, d.tm_min, d.tm_sec };
}
