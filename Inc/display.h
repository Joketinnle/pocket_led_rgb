#ifndef __dispaly_h
#define __dispaly_h

enum DISP_CODE {
    POSITIVE,
    NAGATIVE
};

enum MODE {
    RGB_MODE = 0,
    CW_MODE,
    SCENES_MODE
};

enum RGB_SELECT {
    SELECT_HUE = 0,
    SELECT_SAT,
    SELECT_VAL
};

enum VALUE_SELECT {
    RGB_HUE = 0,
    RGB_SAT = 1,
    RGB_VAL = 2,
    CW_TEMP = 3,
    CW_BRIGHT = 4,
    SCENES_PG = 5 /* SECNES PAGE */
};

enum PAGE_SELECT {
    PAGE_RGB,
    PAGE_CW,
    PAGE_SCENES,
    PAGE_OFF
};

enum CW_SELECT {
    SELECT_TEMP,
    SELECT_BRIGHT
};

enum SCENES_SELECT {
    SCN_FIRST = 0,
    NONE ,
    POLICE,
    AMBULIENCE,
    LIGHTNING,
    FIRE,
    COLORCHASE,
    SCN_LAST
};

enum BATTERY_STATUS {
    REMI_0 = 0,
    REMI_20 ,
    REMI_40 ,
    REMI_60 ,
    REMI_80 ,
    REMI_100 ,
    CHRG_0 ,
    CHRG_20 ,
    CHRG_40 ,
    CHRG_60 ,
    CHRG_80 ,
    CHRG_100
};

enum LED_STATUS {
    OFF = 0,
    ON
};

struct page_info {
    enum PAGE_SELECT PAGE;
    enum VALUE_SELECT select_num;
    uint16_t hue;
    uint8_t sat;
    uint8_t val;
    uint16_t color_temp;
    uint8_t brightness;
    enum SCENES_SELECT SECN;
    bool charging;
    enum BATTERY_STATUS BAT;
    enum LED_STATUS LED_SWITCH;
};

void disp_led_value_init(struct page_info *page_init);

void disp_update(struct page_info *page);


void disp_test(void);

#endif

