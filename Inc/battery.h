#ifndef __battery_h
#define __battery_h

struct voltage{
    float battery;
    float temp;
};

struct batter_status{
    bool chrg;
    bool chrg_cmplt;
    enum BATTERY_STATUS bat_pct;
};

void bettery_status_check(void);
void battery_process(struct page_info *page, struct batter_status *bat_stat);


#endif
