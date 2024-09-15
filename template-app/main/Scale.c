#include "Scale.h"

#define ESP_INTR_FLAG_DEFAULT 0

gptimer_handle_t gptimer = NULL;

static QueueHandle_t gpio_evt_queue = NULL;
static const char *TAG = "example";

gpio_config_t SCK;
gpio_config_t DOUT;
gpio_config_t HX711_ON;

int i = 0;
bool bit_read;
uint8_t bit_postion;

gpio_num_t scale_pin_on = 0;

scale gas;


void IRAM_ATTR gpio_isr_handler(void* arg)
{
    scale *arg2 = (scale*)arg;
    xQueueSendFromISR(gpio_evt_queue, &arg2, NULL);
}

void gpio_task_example(void* arg)
{
    scale gas;
    gptimer_enable(gptimer);
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &gas, portMAX_DELAY)) {
           // printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
           if(gas.flag == 0){
                gptimer_start(gptimer);
                gpio_intr_disable(DOUT_PIN);
                
            }

            vTaskDelay(10);
        }
    }
}

static void read_scale( scale *scale){
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
            gptimer_stop(gptimer);
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
    scale *myGas = (scale*)user_data;
    
    read_scale(myGas);

    //xQueueSendFromISR(queue, &ele, &high_task_awoken);
    // return whether we need to yield at the end of ISR
   return (high_task_awoken == pdTRUE);

}


static void setup_scale( scale *scale){
    
    SCK.mode = GPIO_MODE_DEF_OUTPUT;
    SCK.pin_bit_mask =(1ULL << SCK_PIN);

    DOUT.mode = GPIO_MODE_DEF_INPUT;
    DOUT.pin_bit_mask =(1ULL << DOUT_PIN);
        //interrupt of rising edge
    DOUT.intr_type = GPIO_INTR_NEGEDGE;
    //enable pull-up mode
    DOUT.pull_up_en = 1;

    // Configure on pin
    HX711_ON.mode = GPIO_MODE_DEF_OUTPUT;
    HX711_ON.pin_bit_mask =(1ULL << scale_pin_on);

    gpio_config(&DOUT);

    

    gpio_config(&SCK);
    gpio_set_level(SCK_PIN,0);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task


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
    
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
     
    gptimer_event_callbacks_t cbs = {
        .on_alarm = example_timer_on_alarm_cb_v2,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, scale));
    
    ESP_LOGI(TAG, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

   ESP_LOGI(TAG, "Start timer, auto-reload at alarm event");
    gptimer_alarm_config_t alarm_config2 = {
        .reload_count = 0,
        .alarm_count = 50, // period = 5us
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config2));
    //ESP_ERROR_CHECK(gptimer_start(gptimer));*/

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
     //hook isr handler for specific gpio pin
    gpio_isr_handler_add(DOUT_PIN, gpio_isr_handler, (void*) &gas);
}


void scale_reset(void){
    gas.byte =0;
    gas.flag = 0;
}


bool scale_is_ready(){
  return (gas.flag == 1);
}

uint8_t scale_get_byte(){
  return gas.byte;
}

void scale_init(gpio_num_t pin_on)
{

  scale_pin_on = pin_on;

  gas.flag = 1;

  // Setup scale
  setup_scale(&gas);

  // Init task to read (and probably convert) scale
  xTaskCreate(&gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);
}
