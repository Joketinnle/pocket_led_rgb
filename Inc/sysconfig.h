#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H

#define VERSION_MAJOR                       0
#define VERSION_MINOR                       2
#define VERSION_NUM                         0

void sys_show_info(void);
void sys_stop_mode_enable(void);
void sys_stop_mode_disable(void);
void sys_suspend_all_task(void);
void sys_reset(void);

#endif
