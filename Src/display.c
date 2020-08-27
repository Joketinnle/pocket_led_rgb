#include "includes.h"
#include "font.h"

static uint8_t s_column = 0;
static uint8_t s_row = 0;

void disp_clear(void)
{
    ssd1306_fill(black);
    ssd1306_update();
}

static void disp_set_pos(uint8_t column, uint8_t row)
{
    ssd1306_write_cmd(0xb0+column);
    ssd1306_write_cmd(((row&0xf0)>>4)|0x10);
    ssd1306_write_cmd((row&0x0f)|0x01);
    s_column = column;
    s_row = row;
}

static void disp_show_font_6x8_char(char data)
{
    uint8_t line = 0;
    /* ascii code trans to font array */
    data -= 32; 
    for (; line<6; line++)
        ssd1306_write_byte(font6x8[data][line]);
}

static void disp_show_font_6x8_str(uint8_t *str)
{
    while (*str)
        disp_show_font_6x8_char(*str++);
}

void disp_6x8_printf(const char *ch, ...)
{
    uint8_t dis_buf[24];
    va_list ap;
    va_start(ap,ch);
    vsprintf((char*)dis_buf,ch,ap);
    va_end(ap);
    disp_show_font_6x8_str(dis_buf);
}


static void disp_show_font_8x16_char(char data)
{
    uint8_t line = 0;

    data -= 32;
    for (; line<8; line++)
        ssd1306_write_byte(font8x16[data*16+line]);

    s_column += 1; /* \n */
    disp_set_pos(s_column, s_row);

    for (line=0; line<8; line++)
        ssd1306_write_byte(font8x16[data*16+line+8]);

    s_row += 8; /* next */
    s_column -= 1;
    disp_set_pos(s_column, s_row);
}

static void disp_show_font_8x16_nagative_char(char data)
{
    uint8_t line = 0;

    data -= 32;
    for (; line<8; line++)
        ssd1306_write_byte(~font8x16[data*16+line]);

    s_column += 1; /* \n */
    disp_set_pos(s_column, s_row);

    for (line=0; line<8; line++)
        ssd1306_write_byte(~font8x16[data*16+line+8]);

    s_row += 8; /* next */
    s_column -= 1;
    disp_set_pos(s_column, s_row);
}


static void disp_show_font_8x16_str(enum DISP_CODE CODE, uint8_t *str)
{
    if (CODE == POSITIVE) {
        while (*str)
            disp_show_font_8x16_char(*str++);
    } else {
        while (*str)
            disp_show_font_8x16_nagative_char(*str++);
    }
}

void disp_8x16_printf(const char *ch, ...)
{
    uint8_t dis_buf[24];
    va_list ap;
    va_start(ap,ch);
    vsprintf((char*)dis_buf,ch,ap);
    va_end(ap);
    disp_show_font_8x16_str(POSITIVE, dis_buf);
}

void disp_8x16_nagative_printf(const char *ch, ...)
{
    uint8_t dis_buf[24];
    va_list ap;
    va_start(ap,ch);
    vsprintf((char*)dis_buf,ch,ap);
    va_end(ap);
    disp_show_font_8x16_str(NAGATIVE, dis_buf);
}

static void disp_bettery(enum BATTERY_STATUS pct)
{
    uint8_t tmp_row = 0;
    uint8_t tmp_col = 0;
    
    disp_set_pos(0, 96);
    for (; tmp_col<2; tmp_col++) {
        for (tmp_row=0 ;tmp_row<32; tmp_row++)
            ssd1306_write_byte(battery_remain[pct][tmp_col*32+tmp_row]);
        disp_set_pos(s_column+1, s_row);
    }
}

static void disp_bettery_big(enum BATTERY_STATUS pct)
{
    uint8_t tmp_row = 0;
    uint8_t tmp_col = 0;
    
    disp_set_pos(2, 0);
    for (; tmp_col<4; tmp_col++) {
        for (tmp_row=0 ;tmp_row<64; tmp_row++)
            ssd1306_write_byte(battery_chrg_big[pct][tmp_col*64+tmp_row]);
        disp_set_pos(s_column+1, s_row);
    }
}

static void disp_mode_left_more(bool onoff)
{
    uint8_t row_tmp = 0;
    uint8_t col_tmp = 0;

    disp_set_pos(0, 0);
    if (true == onoff) {
        for (row_tmp=0; row_tmp<2; row_tmp++) {
            for (col_tmp=0; col_tmp<8; col_tmp++)
                ssd1306_write_byte(mode_left_more[row_tmp*8+col_tmp]);
            disp_set_pos(1,0);
        }
    } else {
        for (row_tmp=0; row_tmp<2; row_tmp++) {
            for (col_tmp=0; col_tmp<8; col_tmp++)
                ssd1306_write_byte(mode_more_none[row_tmp*8+col_tmp]);
            disp_set_pos(1,0);
        }
    }
}

static void disp_mode_right_more(bool onoff)
{
    uint8_t row_tmp = 0;
    uint8_t col_tmp = 0;

    disp_set_pos(0, 72);
    if (true == onoff) {
        for (row_tmp=0; row_tmp<2; row_tmp++) {
            for (col_tmp=0; col_tmp<8; col_tmp++)
                ssd1306_write_byte(mode_right_more[row_tmp*8+col_tmp]);
            disp_set_pos(1, 72);
        }
    } else {
        for (row_tmp=0; row_tmp<2; row_tmp++) {
            for (col_tmp=0; col_tmp<8; col_tmp++)
                ssd1306_write_byte(mode_more_none[row_tmp*8+col_tmp]);
            disp_set_pos(1, 72);
        }
    }
}

static void disp_mode(enum MODE mode_setting)
{
    disp_set_pos(0, 8);

    switch (mode_setting) {
    case RGB_MODE:
        disp_8x16_printf("RGB MODE");
        disp_mode_left_more(false);
        disp_mode_right_more(true);
        break;

    case CW_MODE:
        disp_8x16_printf("C&W MODE");
        disp_mode_right_more(true);
        disp_mode_left_more(true);
        break;

    case SCENES_MODE:
        disp_8x16_printf("SCN MODE");
        disp_mode_right_more(false);
        disp_mode_left_more(true);
        break;

    default:
        break;
    }

}

/**
 * @brief select hue satu and value in RGB mode page
 * 
*/
static void disp_select_rgb_cw(enum VALUE_SELECT select)
{
    uint8_t i, j;
    uint8_t row_tmp_1, row_tmp_2, col_tmp_1 = 2, col_tmp_2 = 5;
    uint8_t non_select[2] = {0,0};
    
    if (select < CW_TEMP) {
        row_tmp_1 = select*40;
        row_tmp_2 = select*40+32;
    } else {
        row_tmp_1 = (select-3)*64;
        row_tmp_2 = (select-3)*64+48;
    }
    
    disp_set_pos(col_tmp_1, row_tmp_1);
    for (i=0; i<8; i++)
        ssd1306_write_byte(left_up[i]);
    
    disp_set_pos(col_tmp_2, row_tmp_1);
    for (i=0; i<8; i++)
        ssd1306_write_byte(left_down[i]);
    
    
    disp_set_pos(col_tmp_1, row_tmp_2);
    for(i=0; i<8; i++)
        ssd1306_write_byte(right_up[i]);
    
    disp_set_pos(col_tmp_2, row_tmp_2);
    for(i=0; i<8; i++)
        ssd1306_write_byte(right_down[i]);
    
    if (select == RGB_HUE) {
        non_select[0] = 1;
        non_select[1] = 2;
    } else if (select == RGB_SAT) {
        non_select[0] = 0;
        non_select[1] = 2;   
    } else if (select == RGB_VAL) {
        non_select[0] = 0;
        non_select[1] = 1;
    } else if (select == CW_TEMP) {
        non_select[0] = 1;
    } else if (select == CW_BRIGHT) {
        non_select[0] = 0;
    }

    if (select < CW_TEMP) {
        for (j=0; j<2; j++) {
            row_tmp_1 = non_select[j]*40;
            row_tmp_2 = non_select[j]*40+32;
            
            disp_set_pos(col_tmp_1, row_tmp_1);
            for (i=0; i<8; i++)
                ssd1306_write_byte(clear[i]);
            
            disp_set_pos(col_tmp_2, row_tmp_1);
            for (i=0; i<8; i++)
                ssd1306_write_byte(clear[i]);
            
            disp_set_pos(col_tmp_1,row_tmp_2);
            for (i=0; i<8; i++)
                ssd1306_write_byte(clear[i]);
            
            disp_set_pos(col_tmp_2, row_tmp_2);
            for (i=0; i<8; i++)
                ssd1306_write_byte(clear[i]);
        }
    } else {
        if (non_select[0] == 1) {
            row_tmp_1 = 64;
            row_tmp_2 = 64+48;
        } else {
            row_tmp_1 = 0;
            row_tmp_2 = 48;
        }

        disp_set_pos(col_tmp_1, row_tmp_1);
        for (i=0; i<8; i++)
            ssd1306_write_byte(clear[i]);
        
        disp_set_pos(col_tmp_2, row_tmp_1);
        for (i=0; i<8; i++)
            ssd1306_write_byte(clear[i]);
        
        disp_set_pos(col_tmp_1,row_tmp_2);
        for (i=0; i<8; i++)
            ssd1306_write_byte(clear[i]);
        
        disp_set_pos(col_tmp_2, row_tmp_2);
        for (i=0; i<8; i++)
            ssd1306_write_byte(clear[i]);
    }
}

void disp_scenes_select(enum SCENES_SELECT SECN)
{
    if (SECN <= AMBULIENCE) {
        disp_set_pos(2, 0);
        disp_8x16_printf("NONE       ");
        disp_set_pos(4, 0);
        disp_8x16_printf("POLICE     ");
        disp_set_pos(6, 0);
        disp_8x16_printf("AMBULIENCE ");
    } else if (SECN >AMBULIENCE && SECN <= COLORCHASE) {
        disp_set_pos(2, 0);
        disp_8x16_printf("LIGHTNING  ");
        disp_set_pos(4, 0);
        disp_8x16_printf("FIRE       ");
        disp_set_pos(6, 0);
        disp_8x16_printf("COLORCHASE ");
    }

    switch (SECN) {
    case NONE:
        disp_set_pos(2, 0);
        disp_8x16_nagative_printf("NONE       ");
        break;

    case POLICE:
        disp_set_pos(4, 0);
        disp_8x16_nagative_printf("POLICE     ");
        break;

    case AMBULIENCE:
        disp_set_pos(6, 0);
        disp_8x16_nagative_printf("AMBULIENCE ");
        break;
    
    case LIGHTNING:
        disp_set_pos(2, 0);
        disp_8x16_nagative_printf("LIGHTNING  ");
        break;

    case FIRE:
        disp_set_pos(4, 0);
        disp_8x16_nagative_printf("FIRE       ");
        break;

    case COLORCHASE:
        disp_set_pos(6, 0);
        disp_8x16_nagative_printf("COLORCHASE ");
        break;
    }
}

/* @todo rename this func to disp_update(void) */
void disp_update(struct page_info *page)
{
    static bool init_for_page = true;
    static struct page_info page_prev;
    if (init_for_page == true) {
        init_for_page = false;
        page_prev.PAGE= PAGE_OFF;
    }

    switch (page->PAGE) {
    case PAGE_RGB:
        if (page_prev.PAGE != page->PAGE) {
            disp_clear();
            disp_mode(RGB_MODE);
            disp_set_pos(3, 8);
            disp_8x16_printf("%3d", page->hue);
            disp_set_pos(3, 48);
            disp_8x16_printf("%3d", page->sat);
            disp_set_pos(3, 88);
            disp_8x16_printf("%3d", page->val);
            disp_set_pos(6, 0);
            disp_6x8_printf("  Hue");
            disp_set_pos(6, 40);
            disp_6x8_printf("  Sat");
            disp_set_pos(6, 80);
            disp_6x8_printf("  Val");
            disp_bettery(page->BAT);
            disp_select_rgb_cw(page->select_num);
            led_rgb_update(page->hue, page->sat, page->val);
            printf("PAGE CHANGED? %d\r\n",page_prev.PAGE);
            break;
        }
        if (page_prev.hue != page->hue) {
            disp_set_pos(3, 8);
            disp_8x16_printf("%3d", page->hue);
        } else if (page_prev.sat != page->sat) {
            disp_set_pos(3, 48);
            disp_8x16_printf("%3d", page->sat);
        } else if (page_prev.val != page->val) {
            disp_set_pos(3, 88);
            disp_8x16_printf("%3d", page->val);
        }
        if (page_prev.select_num != page->select_num) {
            disp_select_rgb_cw(page->select_num);
        }
        led_rgb_update(page->hue, page->sat, page->val);
        break;

    case PAGE_CW:
        if (page_prev.PAGE != page->PAGE) {
            disp_clear();
            disp_mode(CW_MODE);
            disp_bettery(page->BAT);
            disp_set_pos(3, 8);
            disp_8x16_printf("%4dK", page->color_temp);
            disp_set_pos(3, 72);
            disp_8x16_printf("%4d%%", page->brightness);
            disp_set_pos(6, 0);
            disp_6x8_printf("  Temp");
            disp_set_pos(6,72);
            disp_6x8_printf("Bright");
            disp_select_rgb_cw(page->select_num);
            led_cw_update(page->color_temp, page->brightness);
            break;
        }
        if (page_prev.color_temp != page->color_temp) {
            disp_set_pos(3, 8);
            disp_8x16_printf("%4dK", page->color_temp);
        } 
        if (page_prev.brightness != page->brightness) {
            disp_set_pos(3, 72);
            disp_8x16_printf("%4d%%", page->brightness);
        }
        if (page_prev.select_num != page->select_num) {
            disp_select_rgb_cw(page->select_num);
        }
        led_cw_update(page->color_temp, page->brightness);
        break;

    case PAGE_SCENES:
        if (page_prev.PAGE != page->PAGE) { 
            disp_clear();
            disp_mode(SCENES_MODE);
            disp_bettery(page->BAT);
            disp_scenes_select(page->SECN);
        }
        if (page_prev.SECN != page->SECN) {
            disp_scenes_select(page->SECN);
        }
        break;

    case PAGE_OFF:
    /* @todo if it is not charging */
        if (page->charging == false) {
            disp_clear();
            led_output_stop();
        } else {
            disp_clear();
            disp_bettery_big(page->BAT);
        }
        break;
    }

    page_prev.PAGE = page->PAGE;
    page_prev.hue = page->hue;
    page_prev.sat = page->sat;
    page_prev.val = page->val;
    page_prev.color_temp = page->color_temp;
    page_prev.brightness = page->brightness;
    page_prev.select_num = page->select_num;
    page_prev.SECN = page->SECN;
    page_prev.BAT = page->BAT;    
}


/**
 *1 RGB MODE  BATTERY
 *2 RGB MODE  BATTERY
 *3 
 *4 NUM1  NUM2  NUM3
 *5 NUM1  NUM2  NUM3
 *6 
 *7 Hue   Sat   Val
 *8 
*/





/**
 * @todo read battery status
 * 
*/
void disp_led_value_init(struct page_info *page_init)
{
    page_init->PAGE = PAGE_OFF;//PAGE_OFF;//PAGE_SCENES;//PAGE_CW;//PAGE_RGB;
    page_init->select_num = RGB_HUE;
    page_init->hue = 0;
    page_init->sat = 0;
    page_init->val = 0;
    page_init->color_temp = 3000;
    page_init->brightness = 0;
    page_init->SECN = NONE;
    /* @debug */
    page_init->charging = true;
    page_init->BAT = REMI_100;
}

void disp_test(void)
{
    disp_clear();

    //disp_page_rgb();


}














void dis_logo(void)
{


}







