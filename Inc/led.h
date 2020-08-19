#ifndef __led_h
#define __led_h

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t c;
    uint8_t w;
}led_bright_t;


#define COLOR_TMP_WARM_MAX                      3000
#define COLOR_TMP_COLD_MAX                      7000


void led_init(void);
void led_output_stop(void);
void led_rgb_update(uint16_t hue, uint8_t sat, uint8_t val);
void led_cw_update(uint16_t color_tmp, uint8_t color_bright);

#endif
