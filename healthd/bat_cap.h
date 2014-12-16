#ifndef _BAT_CAP_H
#define _BAT_CAP_H

//#define CHARGER_SAVE_WHEN_RECOVERY 1
#ifdef CHARGER_SAVE_WHEN_RECOVERY
#define CAPACITY_PATH "/metadata/battery.bat"
#else
#define CAPACITY_PATH "/data/battery.bat"
#endif

extern int put_old_cap(int bat_cap);
extern int load_old_cap(void);

#endif // BAT_CAP_H
