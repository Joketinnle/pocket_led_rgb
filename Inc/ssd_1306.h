#ifndef __ssd_1306_h
#define __ssd_1306_h

/* oled screen driver */

typedef struct {
	uint16_t current_x;
	uint16_t current_y;
	uint8_t inverted;
	uint8_t initialized;
} ssd1306_t;

typedef enum {
    black,
    white
} ssd1306_color;

void ssd1306_write_cmd(uint8_t cmd);
void ssd1306_write_byte(uint8_t data);
void ssd1306_write_data(uint8_t *data, uint16_t size);

void ssd1306_fill(ssd1306_color color);
void ssd1306_update(void);

void ssd1306_reset(void);
void ssd1306_init(void);
void ssd1306_display(uint8_t *pic);

#endif
