#include "keyfob.h"

#define KEYFOB_RELAY0 (1 << 6)
#define KEYFOB_RELAY1 (1 << 5)
#define KEYFOB_RELAY2 (1 << 4)
#define KEYFOB_RELAY3 (1 << 3)
#define KEYFOB_RELAYS (KEYFOB_RELAY0 | KEYFOB_RELAY1 | KEYFOB_RELAY2 | KEYFOB_RELAY3)
#define KEYFOB_PWR_EN (1 << 7)

void keyfob(uint32_t relay, uint32_t hold) {
    // Enable power
    GPIO_SetBits(GPIOB, KEYFOB_PWR_EN);
    vTaskDelay(500);

    // Press or hold the button
#if 1
    uint16_t bits = 0;
    uint32_t i;
    for (i = 0; i < 4; i++) {
        if (relay & (1 << i)) {
            bits |= KEYFOB_RELAY0 >> i;
        }
    }
    GPIO_SetBits(GPIOB, bits);
#else
    GPIO_SetBits(GPIOB, KEYFOB_RELAY0 >> relay);
#endif
    vTaskDelay((hold + 1) * 250);
    GPIO_ResetBits(GPIOB, KEYFOB_RELAYS);

    // Disable power
    vTaskDelay(500);
    GPIO_ResetBits(GPIOB, KEYFOB_PWR_EN);
}

QueueHandle_t keyfob_cmd = NULL;

void keyfob_set(uint8_t relay, uint8_t hold) {
    uint8_t cmd[2];
    
    cmd[0] = relay;
    cmd[1] = hold;

    xQueueSend(keyfob_cmd, cmd, 0);
}

void keyfob_task(void *unused)
{
    while (1) {
        // Get new kefob cmd
        uint8_t cmd[2];
        if (xQueueReceive(keyfob_cmd, &cmd, portMAX_DELAY) == pdPASS) {
            keyfob(cmd[0], cmd[1]);
        }
    }
}

uint32_t keyfob_init() {
    // Enable clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // Init outputs
    GPIO_InitTypeDef init;
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    init.GPIO_Speed = GPIO_Speed_2MHz;
    init.GPIO_Pin = KEYFOB_RELAYS | KEYFOB_PWR_EN;
    
    GPIO_Init(GPIOB, &init);
    GPIO_ResetBits(GPIOB, init.GPIO_Pin);

    // Create queue for cmd
    keyfob_cmd = xQueueCreate(64, sizeof(uint8_t) * 2);

    if (!keyfob_cmd)
        return 2;

    // Create task for cmd execution
    int32_t err = xTaskCreate((TaskFunction_t )keyfob_task,
        (const char*    )"keyfob",
        (uint16_t       )KEYFOB_TASK_STK,
        (void*          )NULL,
        (UBaseType_t    )KEYFOB_TASK_PRIO,
        (TaskHandle_t*  )NULL);

    return err != pdPASS; // returns 1 in case of error
}
