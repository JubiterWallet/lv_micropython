#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"



#define PIN_NUM_POWER_CONTROL        	42
#define PIN_NUM_POWER_OFF_CHECK     	37
#define ESP_INTR_FLAG_DEFAULT 	0


static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR power_gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void power_gpio_task(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            //printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            vTaskDelay(30 / portTICK_PERIOD_MS);
            if(gpio_get_level(io_num))
            	gpio_set_level(PIN_NUM_POWER_CONTROL, 0);
        }
    }
}


void power_gpio_init(void)
{

	gpio_config_t control_gpio_config = {
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = 1,
		.pin_bit_mask = 1ULL << PIN_NUM_POWER_CONTROL
	};

	gpio_config(&control_gpio_config);
	
	gpio_set_level(PIN_NUM_POWER_CONTROL, 1);		//power on
	
	gpio_config_t check_gpio_config = {
		.mode = GPIO_MODE_DEF_INPUT,
		.intr_type = GPIO_INTR_POSEDGE,
		.pin_bit_mask = 1ULL << PIN_NUM_POWER_OFF_CHECK
	};

	gpio_config(&check_gpio_config);
		
	//create a queue to handle gpio event from isr
	gpio_evt_queue = xQueueCreate(1, sizeof(uint32_t));
	//start gpio task
	xTaskCreate(power_gpio_task, "power gpio task", 2048, NULL, 10, NULL);

	//install gpio isr service
	//gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT); 		//main.c ---> machine_pins_init()
	//hook isr handler for specific gpio pin
	gpio_isr_handler_add(PIN_NUM_POWER_OFF_CHECK, power_gpio_isr_handler, (void*) PIN_NUM_POWER_OFF_CHECK);		
}






