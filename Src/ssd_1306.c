#include "includes.h"

#define SSD1306_I2C_ADDR            0x78

/* screen width & height in pixels */
#define SSD1306_WIDTH               128
#define SSD1306_HEIGHT              64

extern I2C_HandleTypeDef hi2c2;

ssd1306_t ssd1306;

uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];


void ssd1306_write_cmd(uint8_t cmd)
{
    HAL_I2C_Mem_Write(&hi2c2, SSD1306_I2C_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 10);
}

void ssd1306_write_data(uint8_t *data, uint16_t size)
{
    HAL_I2C_Mem_Write(&hi2c2, SSD1306_I2C_ADDR, 0x40, 1, data, size, 100);
}

void ssd1306_write_byte(uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c2, SSD1306_I2C_ADDR, 0x40, 1, &data, 1, 100);
}

void ssd1306_fill(ssd1306_color color) 
{
	for(uint32_t i = 0; i < sizeof(ssd1306_buffer); i++)
		ssd1306_buffer[i] = (color == black) ? 0x00 : 0xFF;
}

void ssd1306_update(void)
{	
	for (uint8_t i = 0; i < 8; i++) {
		ssd1306_write_cmd(0xB0 + i);
		ssd1306_write_cmd(0x00);
		ssd1306_write_cmd(0x10);
        // HAL_I2C_Mem_Write(&hi2c2, SSD1306_I2C_ADDR, 0x40,1 ,&ssd1306_buffer[SSD1306_WIDTH * i], SSD1306_WIDTH, 100);
		ssd1306_write_data(&ssd1306_buffer[SSD1306_WIDTH * i],SSD1306_WIDTH);
	}
}

void ssd1306_display(uint8_t *pic)
{
	for (uint8_t i = 0; i < 8; i++) {
		ssd1306_write_cmd(0xB0 + i);
		ssd1306_write_cmd(0x00);
		ssd1306_write_cmd(0x10);
        HAL_I2C_Mem_Write(&hi2c2, SSD1306_I2C_ADDR, 0x40,1 ,&pic[SSD1306_WIDTH * i], SSD1306_WIDTH, 100);
		// ssd1306_write_data(&ssd1306_buffer[SSD1306_WIDTH * i],SSD1306_WIDTH);
	}
}

void ssd1306_reset(void)
{
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET);
	osDelay(800);
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);
}



void ssd1306_init(void)
{
	ssd1306_reset();
	osDelay(100);
	ssd1306_write_cmd(0xAE); //display off
	ssd1306_write_cmd(0x20); //Set Memory Addressing Mode   
	ssd1306_write_cmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	ssd1306_write_cmd(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	ssd1306_write_cmd(0xC8); //Set COM Output Scan Direction
	ssd1306_write_cmd(0x00); //---set low column address
	ssd1306_write_cmd(0x10); //---set high column address
	ssd1306_write_cmd(0x40); //--set start line address
	ssd1306_write_cmd(0x81); //--set contrast control register
	ssd1306_write_cmd(0xFF);
	ssd1306_write_cmd(0xA1); //--set segment re-map 0 to 127
	ssd1306_write_cmd(0xA6); //--set normal display
	ssd1306_write_cmd(0xA8); //--set multiplex ratio(1 to 64)
	ssd1306_write_cmd(0x3F); //
	ssd1306_write_cmd(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	ssd1306_write_cmd(0xD3); //-set display offset
	ssd1306_write_cmd(0x00); //-not offset
	ssd1306_write_cmd(0xD5); //--set display clock divide ratio/oscillator frequency
	ssd1306_write_cmd(0xF0); //--set divide ratio
	ssd1306_write_cmd(0xD9); //--set pre-charge period
	ssd1306_write_cmd(0x22); //
	ssd1306_write_cmd(0xDA); //--set com pins hardware configuration
	ssd1306_write_cmd(0x12);
	ssd1306_write_cmd(0xDB); //--set vcomh
	ssd1306_write_cmd(0x20); //0x20,0.77xVcc
	ssd1306_write_cmd(0x8D); //--set DC-DC enable
	ssd1306_write_cmd(0x14); //
	ssd1306_write_cmd(0xAF); //--turn on SSD1306 panel

    ssd1306_fill(black);
	ssd1306_update();
}



