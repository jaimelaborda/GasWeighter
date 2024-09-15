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

    void setup_scale(scale *scale);
    void read_scale( scale *scale);
    static bool IRAM_ATTR example_timer_on_alarm_cb_v2(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data);
    void IRAM_ATTR gpio_isr_handler(void* arg);
    void gpio_task_example(void* arg);


#endif 