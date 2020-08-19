#ifndef __key_h
#define __key_h

#define KEY_DELAY_MS                            20



#define KEY_PLUS_LONG_PRESS                     0x01
#define KEY_PLUS_LONG_RELEASE                   0x00
#define KEY_MINUS_LONG_PRESS                    0x02
#define KEY_MINUS_LONG_RELEASE                  0x00


enum KEY_ACTION {
    KEY_NONE_ACTION = 0,
    KEY_SHORT_PRESS = 1,
    KEY_SHORT_RELEASE, 
    KEY_LONG_PRESS,
    KEY_LONG_RELEASE
};




enum KEY_TYPE {
    KEY_POWER,
    KEY_MENU,
    KEY_PLUS,
    KEY_MINUS
};


void key_poll(void);
uint8_t key_process(struct page_info *page, uint32_t key_sta);
#endif
