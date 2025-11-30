
#include "button.h"

QueueHandle_t button_cmd = NULL;

uint8_t button_get() {
    return GPIO_ReadInputDataBit(GPIOA, (1 << 15));
}

void button_task(void *unused) {
    uint32_t prev = 1;

    while (1) {

        uint32_t cur = button_get();

        if (prev == 1 && cur == 0) {
#if 0
            // Testing only. To be removed.
            uint8_t led = LED_STATE_PRESS;
            xQueueSend(led_state, &led, 0);

            uint8_t cmd[2] = {0, 0};
            xQueueSend(keyfob_cmd, &cmd, 0);
#endif
            // Current system time
            uint32_t hold_time = xTaskGetTickCount();

            // Deglitch
            vTaskDelay(100);

            // Wait for button to be released
            while (button_get() == 0);
            
            // Calculate hold time
            // TODO: Time roll over?
            hold_time = xTaskGetTickCount() - hold_time;

            uint8_t hold = hold_time > 2500;
            xQueueSend(button_cmd, &hold, 0);

#if 0
            // Testing only. To be removed.
            uint8_t led = hold ? LED_STATE_HOLD : LED_STATE_PRESS;
            xQueueSend(led_state, &led, 0);

            uint8_t cmd[2] = {0, 0};
            cmd[1] = hold;
            xQueueSend(keyfob_cmd, &cmd, 0);
#endif
        }

        vTaskDelay(10);

        prev = cur;

        
    }
}

uint32_t button_init() {
    // Enable clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // Init outputs
    GPIO_InitTypeDef init;
    init.GPIO_Mode = GPIO_Mode_IPU;
    init.GPIO_Speed = GPIO_Speed_2MHz;
    init.GPIO_Pin = (1 << 15);

    GPIO_Init(GPIOA, &init);

    // Create queue for button states
    button_cmd = xQueueCreate(32, sizeof(uint8_t));

    if (!button_cmd)
        return 2;

    // Create task for cmd execution
    int32_t err = xTaskCreate((TaskFunction_t )button_task,
        (const char*    )"button",
        (uint16_t       )BUTTON_TASK_STK,
        (void*          )NULL,
        (UBaseType_t    )BUTTON_TASK_PRIO,
        (TaskHandle_t*  )NULL);
    
    return err != pdPASS; // returns 1 in case of error
}
