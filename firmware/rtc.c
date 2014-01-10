
#include "rtc.h"

#define RTC_TYPE	1338	/* 8564:RTC8564, 1338:DS1338 */


/*-------------------------------------------------*/
/* RTC functions                                   */


int rtc_gettime (
	RTC *rtc
)
{
	BYTE buf[8];
	/* Return error with default time (Jan 1, 2011) */
	rtc->sec = 0; rtc->min = 0; rtc->hour = 0;
	rtc->wday = 0; rtc->mday = 1; rtc->month = 1; rtc->year = 2011;
	return 0;
}




int rtc_settime (const RTC *rtc)
{
	BYTE buf[10];
    return 1;

}


