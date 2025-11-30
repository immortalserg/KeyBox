#include "led.h"

QueueHandle_t led_state = NULL;

void led_set(uint32_t col) {
    GPIO_ResetBits(GPIOC, LED_COL_WHITE);
    GPIO_SetBits(GPIOC, col & LED_COL_WHITE);
}

void led_set_state(uint8_t state) {
    xQueueSend(led_state, &state, 0);
}

void led_time_set(uint32_t col, uint32_t time_on, uint32_t time_off) {
    led_set(col);
    vTaskDelay(time_on);
    led_set(LED_COL_OFF);
    vTaskDelay(time_off);
}

void led_task(void *unused)
{
    // White start
    led_time_set(LED_COL_WHITE, 1000, 100);

    uint8_t state = LED_STATE_IDLE;
    uint8_t back = LED_STATE_IDLE;

    while(1)
    {
        // Feed the dog
        IWDG_ReloadCounter();

        // Check if led state updated
        xQueueReceive(led_state, &state, 5);

        switch(state)
        {
            case LED_STATE_IDLE:
                led_time_set(LED_COL_MAGENTA, 50, 500);
                // Permanent
                back = state;
                break;
        
            case LED_STATE_CONNECTED:
                led_time_set(LED_COL_MAGENTA, 50, 50);
                led_time_set(LED_COL_MAGENTA, 50, 500);
                 // Permanent state
                back = state;
                break;

            case LED_STATE_PRESS:
                led_time_set(LED_COL_GREEN, 500, 500);
                // Single time state
                state = back;
                break;

            case LED_STATE_ERASE:
                led_time_set(LED_COL_RED, 100, 100);
                // Permanent state
                back = state;
                break;
        }
    }
}

uint32_t led_init() {
    // Enable clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    // Init outputs
    GPIO_InitTypeDef init;
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    init.GPIO_Speed = GPIO_Speed_2MHz;
    init.GPIO_Pin = LED_COL_WHITE;
    
    GPIO_Init(GPIOC, &init);
    GPIO_ResetBits(GPIOC, init.GPIO_Pin);

    // Create queue
    led_state = xQueueCreate(64, sizeof(uint8_t));

    if (!led_state)
        return 2;

    // Create task
    int32_t err = xTaskCreate((TaskFunction_t )led_task,
        (const char*    )"led",
        (uint16_t       )LED_TASK_STK,
        (void*          )NULL,
        (UBaseType_t    )LED_TASK_PRIO,
        (TaskHandle_t*  )NULL);
    
    return err != pdPASS; // returns 1 in case of error
}
