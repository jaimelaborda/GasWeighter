#ifndef  SCALE_H
    #define SCALE_H

    #define NUMBER_OF_PULSES 25
    #define SCK_PIN  11
    #define DOUT_PIN  10

    #include <stdio.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/queue.h"
    #include "driver/gptimer.h"
    #include "esp_log.h"
    #include <string.h>
    #include <stdlib.h>
    #include <inttypes.h>
    #include "driver/gpio.h"

  

    typedef  struct {
        uint32_t byte;
        bool flag;
    }scale;


    typedef struct {
        uint64_t event_count;
    } example_queue_element_t;

    void scale_init(gpio_num_t pin_on);
    void scale_reset(void);
    bool scale_is_ready();
    uint8_t scale_get_byte();


#endif 