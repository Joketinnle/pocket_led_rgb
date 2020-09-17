#include "includes.h"


void sys_show_info(void)
{
    printf("system starting...\r\n");
    printf("FW build %s  %s\r\n", __DATE__, __TIME__);
    printf("version %d.%d.%d\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_NUM);
}
