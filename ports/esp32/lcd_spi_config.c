#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"



#define LCD_COMMAND		(0 << 8)
#define LCD_DATA		(1 << 8)

#define LCD_HOST    SPI2_HOST

#define PIN_NUM_MISO -1
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK  12
#define PIN_NUM_CS   10

#define SPI_CLK_HIGH()		gpio_set_level(PIN_NUM_CLK, 1)
#define SPI_CLK_LOW()		gpio_set_level(PIN_NUM_CLK, 0)
#define SPI_MOSI_HIGH()		gpio_set_level(PIN_NUM_MOSI, 1)
#define SPI_MOSI_LOW()		gpio_set_level(PIN_NUM_MOSI, 0)
#define SPI_MOSI_SET_LEVEL(x)	gpio_set_level(PIN_NUM_MOSI, x)
#define SPI_CS_HIGH()		gpio_set_level(PIN_NUM_CS, 1)
#define SPI_CS_LOW()		gpio_set_level(PIN_NUM_CS, 0)

/*
 The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct.
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[32];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[]={
	{0x3A, {0x50}, 1},
    {0xFF, {0x77, 0x01, 0x00, 0x00, 0x13}, 5},
    {0xE8, {0x00, 0x0E}, 2},
    {0xFF, {0x77, 0x01, 0x00, 0x00, 0x00}, 5},
	{0xFF, {0x77, 0x01, 0x00, 0x00, 0x10}, 5},
	{0xC6, {0x07}, 1},
	{0x11, {0}, 0xC0},
	{0xFF, {0x77, 0x01, 0x00, 0x00, 0x13}, 5},
	{0xE8, {0x00, 0x0C}, 0x22},
	{0xE8, {0x00, 0x00}, 2},
	{0xFF, {0x77, 0x01, 0x00, 0x00, 0x00}, 5},
	{0xFF, {0x77, 0x01, 0x00, 0x00, 0x10}, 5},
	{0xC0, {0x4F, 0x00}, 2},
	{0xC1, {0x07, 0x02}, 2},
	{0xC2, {0x20, 0x05}, 2},
	{0xC6, {0x01}, 1},
	{0xCC, {0x18}, 1},
	{0xB0, {0x00, 0x0A, 0x11, 0x0C, 0x10, 0x05, 0x00, 0x08, 0x08, 0x1F, 0x07, 0x13, 0x10, 0xA9, 0x30, 0X18}, 16},
	{0xB1, {0x00, 0x0B, 0x11, 0x0D, 0x0F, 0x05, 0x02, 0x07, 0x06, 0x20, 0x05, 0x15, 0x13, 0xA9, 0x30, 0X18}, 16},
	{0xFF, {0x77, 0x01, 0x00, 0x00, 0x11}, 5},
	{0xB0, {0x65}, 1},
	{0xB1, {0x60}, 1},
	{0xB2, {0x80}, 1},
	{0xB3, {0x80}, 1},
	{0xB5, {0x4D}, 1},
	{0xB7, {0x85}, 1},
	{0xB8, {0x21}, 1},
	{0xBB, {0x33}, 1},
	{0xBC, {0x33}, 1},
	{0xC0, {0x09}, 1},
	{0xC1, {0x78}, 1},
	{0xC2, {0x78}, 1},
	{0xEE, {0x42}, 0x81},
	{0xE0, {0x00, 0x00, 0x02}, 3},
	{0xE1, {0x03, 0xA0, 0x00, 0x00, 0x02, 0xA0, 0x00, 0x00, 0x00, 0x33, 0x33}, 11},
	{0xE2, {0x22, 0x22, 0x33, 0x33, 0x88, 0xA0, 0x00, 0x00, 0x87, 0xA0, 0x00, 0x00}, 12},
	{0xE3, {0x00, 0x00, 0x22, 0x22}, 4},
	{0xE4, {0x44, 0x44}, 2},
	{0xE5, {0x04, 0x84, 0xA0, 0xA0, 0x06, 0x86, 0xA0, 0xA0, 0x08, 0x88, 0xA0, 0xA0, 0x0A, 0x8A, 0xA0, 0xA0}, 16},
	{0xE6, {0x00, 0x00, 0x22, 0x22}, 4},
	{0xE7, {0x44, 0x44}, 2},
	{0xE8, {0x03, 0x83, 0xA0, 0xA0, 0x05, 0x85, 0xA0, 0xA0, 0x07, 0x87, 0xA0, 0xA0, 0x09, 0x89, 0xA0, 0xA0}, 16},
	{0xEB, {0x00, 0x01, 0xE4, 0xE4, 0x88, 0x00, 0x40}, 7},
	{0xEC, {0x3C, 0x01}, 2},
	{0xED, {0xAB, 0x89, 0x76, 0x54, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x45, 0x67, 0x98, 0xBA}, 16},
	{0xEF, {0x08, 0x08, 0x08, 0x45, 0x3F, 0x54}, 6},
	{0xFF, {0x77, 0x01, 0x00, 0x00, 0x00}, 0x45},
	{0x29, {0}, 0x00},	

    {0, {0}, 0xff}
	
};

void spi_delay(void)
{
	int i;
	for(i=0; i<16;i++);
}

void spi_send_bit(uint8_t value)
{
	uint8_t level = value ? 1: 0;
	
	SPI_CLK_LOW();

	SPI_MOSI_SET_LEVEL(level);

	spi_delay();

	SPI_CLK_HIGH();

	spi_delay();

}
/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void spi_lcd_cmd(const uint8_t cmd, bool keep_cs_active)
{
	int i;
	uint8_t data = cmd;
	SPI_CS_LOW();
	
	spi_send_bit(0);	//D/CX
	
	for(i=0; i < 8; i++)
	{
		spi_send_bit(data & 0x80);
		
		data <<= 1;
	}

	SPI_CLK_LOW();

	SPI_CS_HIGH();	
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void spi_lcd_data(const uint8_t *data, int len)
{
	int i, j;
	uint8_t send;

	if(!len) return;
	
	SPI_CS_LOW();

	for(i=0; i < len; i++)
	{		
		spi_send_bit(1);	//D/CX
		send = data[i];
		for(j=0; j < 8; j++)
		{
			spi_send_bit(send & 0x80);
			
			send <<= 1;
		}
	}
	
	SPI_CLK_LOW();

	SPI_CS_HIGH();	

}

void lcd_config_init()
{
    int cmd=0;
    const lcd_init_cmd_t* lcd_init_cmds;
	int delay = 0;

    //printf("LCD ST7701S initialization.\n");
    lcd_init_cmds = st_init_cmds;

    //Send all the commands
    while (lcd_init_cmds[cmd].databytes!=0xff) {
        spi_lcd_cmd(lcd_init_cmds[cmd].cmd, false);
        spi_lcd_data(lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes&0x1F);

		delay = 0;
        if (lcd_init_cmds[cmd].databytes&0x80) {
            delay += 100;
        }
        else if (lcd_init_cmds[cmd].databytes&0x40) {
            delay += 20;
        }
        else if (lcd_init_cmds[cmd].databytes&0x20) {
            delay += 10;
        }

		if(delay)
			vTaskDelay(delay / portTICK_PERIOD_MS);
		
        cmd++;
    }

    ///Enable backlight
    //gpio_set_level(PIN_NUM_BCKL, 0);
}

void spi_gpio_init(void)
{	
	gpio_set_level(PIN_NUM_CS, 1);
	gpio_set_level(PIN_NUM_CLK, 0);
	gpio_set_level(PIN_NUM_MOSI, 0);
	
    gpio_config_t spi_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_NUM_MOSI
    };
    gpio_config(&spi_gpio_config);
	spi_gpio_config.pin_bit_mask = 1ULL << PIN_NUM_CLK;
    gpio_config(&spi_gpio_config);
	spi_gpio_config.pin_bit_mask = 1ULL << PIN_NUM_CS;
    gpio_config(&spi_gpio_config);
}


void spi_lcd_control_init(void)
{
	spi_gpio_init();
	
	//Initialize the LCD
	lcd_config_init();

}
