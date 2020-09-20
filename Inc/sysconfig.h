#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H

#define VERSION_MAJOR                       0
#define VERSION_MINOR                       2
#define VERSION_NUM                         7

void sys_show_info(void);
void sys_stop_mode_enable(void);
void sys_stop_mode_disable(void);
void sys_reset(void);
bool is_sys_in_stop_mode(void);

#endif
