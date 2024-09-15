#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_mac.h"
#include "Scale.h"

#define HX711_ON_PIN  21

#define ZERO_LOAD 16740795.0
#define Kilo 16679106.0

gpio_config_t HX711_ON;
scale gas;


#define ESP_INTR_FLAG_DEFAULT 0


void  Print(void *pvParamters){
    while(1){
     if(gas.flag == 1){
       // printf("Decimal: %lu, Hex: %lx\n", byte, byte);
       int32_t a = (gas.byte-ZERO_LOAD) ;
       double b = a*(1/(Kilo-ZERO_LOAD));
       printf("%f\n" ,b);

        gas.byte =0;
        gas.flag = 0;
    }

        vTaskDelay(10);
    }
}


void app_main(void)
{
    HX711_ON.mode = GPIO_MODE_DEF_OUTPUT;
    HX711_ON.pin_bit_mask =(1ULL << HX711_ON_PIN);

    gas.flag = 1;

    setup_scale(&gas);

    
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
     //hook isr handler for specific gpio pin
    gpio_isr_handler_add(DOUT_PIN, gpio_isr_handler, (void*) &gas);

    
    xTaskCreate(&gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);
    xTaskCreate(&Print, "Print", 2048, NULL, 1, NULL);

    while (1)
    {
        vTaskDelay(1000);

    }
    
}