#include "Scale.h"


 static const char *TAG = "example";
gpio_config_t SCK;
gpio_config_t DOUT;

int i = 0;
bool bit_read;
uint8_t bit_postion;

void setup_scale(gptimer_handle_t *timer, scale *scale){

    SCK.mode = GPIO_MODE_DEF_OUTPUT;
    SCK.pin_bit_mask =(1ULL << SCK_PIN);

    DOUT.mode = GPIO_MODE_DEF_INPUT;
    DOUT.pin_bit_mask =(1ULL << DOUT_PIN);
        //interrupt of rising edge
    DOUT.intr_type = GPIO_INTR_NEGEDGE;
    //enable pull-up mode
    DOUT.pull_up_en = 1;
    gpio_config(&DOUT);

    

    gpio_config(&SCK);
    gpio_set_level(SCK_PIN,0);

    example_queue_element_t ele;

    QueueHandle_t queue = xQueueCreate(10, sizeof(example_queue_element_t));
    if (!queue) {
        ESP_LOGE(TAG, "Creating queue failed");
        return;
    }

    
    //ESP_LOGI(TAG, "Create timer handle");
    //gptimer_handle_t gptimer = NULL;
    
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 10000000, // 10MHz, 1 tick=0.1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer));
     
    gptimer_event_callbacks_t cbs = {
        .on_alarm = example_timer_on_alarm_cb_v2,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer, &cbs, scale));
    
    ESP_LOGI(TAG, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(timer));

   ESP_LOGI(TAG, "Start timer, auto-reload at alarm event");
    gptimer_alarm_config_t alarm_config2 = {
        .reload_count = 0,
        .alarm_count = 50, // period = 5us
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarm_config2));
    //ESP_ERROR_CHECK(gptimer_start(gptimer));*/
}


void read_scale(gptimer_handle_t timer , scale *scale){
    if(i < (NUMBER_OF_PULSES*2)){
            if(i % 2 != 0){
                gpio_set_level(SCK_PIN,1);
                }
                else{
                    gpio_set_level(SCK_PIN,0);
                    bit_read = gpio_get_level(DOUT_PIN);
                    if(bit_postion <= 24){
                        if(bit_read){
                            scale->byte |= (0x800000 >> bit_postion);
                        }
                        bit_postion++;
                    }
                }
         //   }
         i++;

        }
        else{
            scale->flag = 1;
            gpio_set_level(SCK_PIN,0);
            gptimer_stop(timer);
            i = 0;
            bit_postion = 0;
            //byte =0;
            gpio_intr_enable(DOUT_PIN);

        }

}

static bool IRAM_ATTR example_timer_on_alarm_cb_v2(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data )
{
    BaseType_t high_task_awoken = pdFALSE;
   // QueueHandle_t queue = (QueueHandle_t)user_data;
    // Retrieve count value and send to queue
    //example_queue_element_t ele = {
   //    .event_count = edata->count_value
  //  };
    scale *myObj = (scale*)user_data;
    
    read_scale(timer, myObj);

    //xQueueSendFromISR(queue, &ele, &high_task_awoken);
    // return whether we need to yield at the end of ISR
   return (high_task_awoken == pdTRUE);

}
