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


void  Print(void *pvParamters){
    while(1){

     if(scale_is_ready()){
       // printf("Decimal: %lu, Hex: %lx\n", byte, byte);
       int32_t a = (uint32_t)scale_get_byte()  - ZERO_LOAD;
       double b = a*(1/(Kilo-ZERO_LOAD));
       printf("%f\n" ,b);

       scale_reset();
    }

    vTaskDelay(10);
    }
}


void app_main(void)
{

    scale_init(HX711_ON_PIN);

    // Create task to print value (you can do this in the main task but whatever)
    xTaskCreate(&Print, "Print", 2048, NULL, 1, NULL);

    while (1)
    {
        vTaskDelay(1000);
    }
    
}