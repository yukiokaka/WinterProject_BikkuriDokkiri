#include <string.h>
#include "LPC1100.h"
#include "xprintf.h"
#include "uart.h"
#include "diskio.h"
#include "ff.h"
#include "rtc.h"
#include "terminal.h"
#define F_CPU	36000000

FATFS Fatfs;

char Line[64];	/* Console input buffer */
BYTE Buff[4096] __attribute__ ((aligned(4)));

volatile UINT Timer;	/* 1kHz increment timer */

DWORD get_fattime (void)
{
	RTC rtc;


	/* Get local time */
	rtc_gettime(&rtc);

	/* Pack date and time into a DWORD variable */
	return	  ((DWORD)(rtc.year - 1980) << 25)
			| ((DWORD)rtc.month << 21)
			| ((DWORD)rtc.mday << 16)
			| ((DWORD)rtc.hour << 11)
			| ((DWORD)rtc.min << 5)
			| ((DWORD)rtc.sec >> 1);
}



/*--------------------------------------------------------------*/
/* 1000Hz interval timer                                         */
/*--------------------------------------------------------------*/

void SysTick_Handler (void)
{
	Timer++;		/* Performance counter */
#if DISP_USE_FILE_LOADER
	TmrFrm += 1000;	/* Increment frame time (disp.c) */
#endif
}


/*--------------------------------------------------------------*/
/* Put FatFs result code                                        */
/*--------------------------------------------------------------*/

static
void put_rc (FRESULT rc)
{
	const char *str =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
		"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
	FRESULT i;

	for (i = 0; i != rc && *str; i++) {
		while (*str++) ;
	}
	xprintf("rc=%u FR_%s\n", (UINT)rc, str);
}



void terminal_init(void)
{
#if _MODE_STANDALONE
    f_mount(0, &Fatfs);
#endif
    
}

void terminal(void)
{
	long p1, p2;
	char *ptr, *ptr2;
	FRESULT res;
	FIL fil;
	DIR dir;
	FILINFO fno;
	UINT ofs, s1, s2, cnt;
	RTC rtc;
	FATFS *fs;


    xputc('>');
    xgets(Line, sizeof Line);
    ptr = Line;
    switch (*ptr++) {

    case 'd' :
        switch (*ptr++) {

        case 'i' :	/* di - Initialize disk */
            xprintf("rc=%d\n", disk_initialize(0));
            break;

        case 'd' :	/* dd <sector> - Dump secrtor */
            if (!xatoi(&ptr, &p2)) break;
            res = disk_read(0, Buff, p2, 1);
            if (res) { xprintf("rc=%d\n", res); break; }
            xprintf("Sector:%lu\n", p2);
            for (ptr2 = (char*)Buff, ofs = 0; ofs < 0x200; ptr2 += 16, ofs += 16)
                put_dump(ptr2, ofs, 16, DW_CHAR);
            break;
        }
        break;

    case 'f' :
        switch (*ptr++) {
        case 'i' :	/* fi - Initialize logical drive */
            put_rc(f_mount(0, &Fatfs));
            break;

        case 'l' :	/* fl [<path>] - Directory listing */
            while (*ptr == ' ') ptr++;
            res = f_opendir(&dir, ptr);
            if (res) { put_rc(res); break; }
            p1 = s1 = s2 = 0;
            for(;;) {
                res = f_readdir(&dir, &fno);
                if ((res != FR_OK) || !fno.fname[0]) break;
                if (fno.fattrib & AM_DIR) {
                    s2++;
                } else {
                    s1++; p1 += fno.fsize;
                }
                xprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s\n", 
                        (fno.fattrib & AM_DIR) ? 'D' : '-',
                        (fno.fattrib & AM_RDO) ? 'R' : '-',
                        (fno.fattrib & AM_HID) ? 'H' : '-',
                        (fno.fattrib & AM_SYS) ? 'S' : '-',
                        (fno.fattrib & AM_ARC) ? 'A' : '-',
                        (fno.fdate >> 9) + 1980, (fno.fdate >> 5) & 15, fno.fdate & 31,
                        (fno.ftime >> 11), (fno.ftime >> 5) & 63,
                        fno.fsize, &(fno.fname[0]));
            }
            xprintf("%4u File(s),%10lu bytes\n%4u Dir(s)", s1, p1, s2);
            if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
                xprintf(", %10luK bytes free\n", p1 * fs->csize / 2);
            break;

        case 'o' :	/* fo <mode> <file> - Open a file */
            if (!xatoi(&ptr, &p1)) break;
            while (*ptr == ' ') ptr++;
            res = f_open(&fil, ptr, (BYTE)p1);
            put_rc(res);
            break;

        case 'c' :	/* fc - Close a file */
            res = f_close(&fil);
            put_rc(res);
            break;

        case 'e' :	/* fe - Seek file pointer */
            if (!xatoi(&ptr, &p1)) break;
            res = f_lseek(&fil, p1);
            put_rc(res);
            if (res == FR_OK)
                xprintf("fptr = %lu(0x%lX)\n", f_tell(&fil), f_tell(&fil));
            break;

        case 'r' :	/* fr <len> - read file */
            if (!xatoi(&ptr, &p1)) break;
            p2 = 0;
            Timer = 0;
            while (p1) {
                if ((UINT)p1 >= sizeof Buff)	{ cnt = sizeof Buff; p1 -= sizeof Buff; }
                else 			{ cnt = (WORD)p1; p1 = 0; }
                res = f_read(&fil, Buff, cnt, &s2);
                if (res != FR_OK) { put_rc(res); break; }
                p2 += s2;
                if (cnt != s2) break;
            }
            s2 = Timer;
            xprintf("%lu bytes read with %lu kB/sec.\n", p2, p2 / s2);
            break;

        case 'd' :	/* fd <len> - read and dump file from current fp */
            if (!xatoi(&ptr, &p1)) break;
            ofs = f_tell(&fil);
            while (p1) {
                if (p1 >= 16)	{ cnt = 16; p1 -= 16; }
                else 			{ cnt = (WORD)p1; p1 = 0; }
                res = f_read(&fil, Buff, cnt, &cnt);
                if (res != FR_OK) { put_rc(res); break; }
                if (!cnt) break;
                put_dump(Buff, ofs, cnt, DW_CHAR);
                ofs += 16;
            }
            break;

        case 'w' :	/* fw <len> <val> - write file */
            if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
            for (cnt = 0; cnt < sizeof Buff; Buff[cnt++] = 0) ;
            p2 = 0;
            Timer = 0;
            while (p1) {
                if ((UINT)p1 >= sizeof Buff) { cnt = sizeof Buff; p1 -= sizeof Buff; }
                else { cnt = (WORD)p1; p1 = 0; }
                res = f_write(&fil, Buff, cnt, &s2);
                if (res != FR_OK) { put_rc(res); break; }
                p2 += s2;
                if (cnt != s2) break;
            }
            s2 = Timer;
            xprintf("%lu bytes written with %lu kB/sec.\n", p2, p2 / s2);
            break;

        case 'v' :	/* fv - Truncate file */
            put_rc(f_truncate(&fil));
            break;

        case 'n' :	/* fn <old_name> <new_name> - Change file/dir name */
            while (*ptr == ' ') ptr++;
            ptr2 = strchr(ptr, ' ');
            if (!ptr2) break;
            *ptr2++ = 0;
            while (*ptr2 == ' ') ptr2++;
            put_rc(f_rename(ptr, ptr2));
            break;

        case 'u' :	/* fu <name> - Unlink a file or dir */
            while (*ptr == ' ') ptr++;
            put_rc(f_unlink(ptr));
            break;

        case 'k' :	/* fk <name> - Create a directory */
            while (*ptr == ' ') ptr++;
            put_rc(f_mkdir(ptr));
            break;

        case 'g' :	/* fg <path> - Change current directory */
            while (*ptr == ' ') ptr++;
            put_rc(f_chdir(ptr));
            break;

        case 'q' :	/* fq - Show current dir path */
            res = f_getcwd(Line, sizeof Line);
            if (res)
                put_rc(res);
            else
                xprintf("%s\n", Line);
            break;
        }
        break;

    case 't' :	/* t [<year> <mon> <mday> <hour> <min> <sec>] */
        if (xatoi(&ptr, &p1)) {
            rtc.year = (WORD)p1;
            xatoi(&ptr, &p1); rtc.month = (BYTE)p1;
            xatoi(&ptr, &p1); rtc.mday = (BYTE)p1;
            xatoi(&ptr, &p1); rtc.hour = (BYTE)p1;
            xatoi(&ptr, &p1); rtc.min = (BYTE)p1;
            if (!xatoi(&ptr, &p1)) break;
            rtc.sec = (BYTE)p1;
            rtc_settime(&rtc);
        }
        rtc_gettime(&rtc);
        xprintf("%u/%u/%u %02u:%02u:%02u\n", rtc.year, rtc.month, rtc.mday, rtc.hour, rtc.min, rtc.sec);
        break;
        
    }
    
    
}
