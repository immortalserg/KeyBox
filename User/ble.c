#include "ble.h"

//////////////
// BLE CTRL //
//////////////

void ble_ctrl_init() {
    // Enable clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    // TX pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // RX pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Enable clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);
    USART_InitTypeDef USART_InitStructure = {0};

    // UART
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(UART8, &USART_InitStructure);
    USART_Cmd(UART8, ENABLE);
}

#define BLE_CTRL_CHECK

#ifdef BLE_CTRL_CHECK
uint8_t ble_ctrl_recv_byte() {
    while (USART_GetFlagStatus(UART8, USART_FLAG_RXNE) == RESET);
    return USART_ReceiveData(UART8);
}
#endif

uint32_t ble_ctrl_check() {
#ifdef BLE_CTRL_CHECK
    return ble_ctrl_recv_byte() != '*';
#else
    return 0;
#endif
}

void ble_ctrl_send_byte(uint8_t dat) {
    while(USART_GetFlagStatus(UART8, USART_FLAG_TC) == RESET);
    USART_SendData(UART8, dat);
}

void ble_ctrl_send_str(uint8_t *cmd) {
    while (*cmd) {
        ble_ctrl_send_byte(*cmd);
        cmd++;
    }
}

void ble_ctrl_send_dat(uint8_t *dat, uint32_t sz) {
    uint32_t i;

    for (i = 0; i < sz; i++) {
        ble_ctrl_send_byte(dat[i]);
    }
}

uint32_t ble_set_name(uint8_t *name) {
    ble_ctrl_send_byte('*');
    ble_ctrl_send_byte('N');
    ble_ctrl_send_str(name);
    ble_ctrl_send_byte(0);

    dbg_printf("ble ctrl set name: '%s'\n", name);

    return ble_ctrl_check();
}

uint32_t ble_set_passkey(uint32_t passkey) {
    ble_ctrl_send_byte('*');
    ble_ctrl_send_byte('P');
    ble_ctrl_send_dat((uint8_t *)&passkey, 4);

    dbg_printf("ble ctrl set passkey: %u\n", passkey);

    return ble_ctrl_check();
}

uint32_t ble_ctrl_simple(uint8_t cmd) {
    ble_ctrl_send_byte('*');
    ble_ctrl_send_byte(cmd);

    dbg_printf("ble ctrl simple '%c'\n", cmd);

    return ble_ctrl_check();
}

uint32_t ble_set_advert(uint32_t is_b) {
    return ble_ctrl_simple(is_b ? 'B' : 'A');
}

uint32_t ble_factory_reset() {
    return ble_ctrl_simple('R');
}

// Start ble module. Works only single time after power-up
// Must be issued last
uint32_t ble_continue() {
    return ble_ctrl_simple('C');
}

uint32_t ble_overview() {
    return ble_ctrl_simple('O');
}

uint32_t ble_set_serial(uint16_t serial) {
    ble_ctrl_send_byte('*');
    ble_ctrl_send_byte('S');
    ble_ctrl_send_dat((uint8_t *)&serial, 2);

    dbg_printf("ble ctrl set serial: %04X\n", serial);

    return ble_ctrl_check();
}

//////////////
// BLE DATA //
//////////////

void ble_data_init() {
    // Enable clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    // TX pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RX pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Enable clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    USART_InitTypeDef USART_InitStructure = {0};

    // UART
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
    NVIC_EnableIRQ(USART2_IRQn);
    
}

void ble_data_byte(uint8_t dat) {
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {
        taskYIELD();
    }
    USART_SendData(USART2, dat);
    //while (!USART_GetFlagStatus(USART2, USART_FLAG_TC));
}

xQueueHandle ble_data_recv_byte = NULL;
xQueueHandle ble_data_recv = NULL;

void ble_data_send(uint8_t *dat, uint32_t sz) {
    uint32_t i;

    for (i = 0; i < sz; i++) {
        ble_data_byte(dat[i]);
    }
}

void ble_data_send_str(char *str) {
    ble_data_send((uint8_t *)str, strlen(str));
}

void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void) {
    // Store new char
    uint8_t dat = USART_ReceiveData(USART2);
    xQueueSendFromISR(ble_data_recv_byte, &dat, 0);
}

uint8_t ble_data_rx_buf[BLE_DATA_RX_SZ];

void ble_data_recv_task(void *unused) {
    uint32_t i = 0;

    while (1) {
        if (xQueueReceive(ble_data_recv_byte, ble_data_rx_buf + i, 50) != pdPASS) {
            if (i > 0) {
                xQueueSend(ble_data_recv, ble_data_rx_buf, 0);
                i = 0;
            }
        } else {
            i++;

            if (i == BLE_DATA_RX_SZ) {
                xQueueSend(ble_data_recv, ble_data_rx_buf, 0);
                i = 0;
            }
        }
    }
}

////////////////
// BLE STATUS //
////////////////

uint32_t ble_status_get() {
    static uint8_t prev = 0;
    uint32_t result = 0;
    uint8_t cur = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3);

    if (cur != prev)
        result |= BLE_STATUS_UPDATED;

    result |= cur;

    prev = cur;

    return result;
}

void ble_status_init() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

uint32_t ble_init() {
    ble_data_recv_byte = xQueueCreate(BLE_DATA_RX_SZ, sizeof(uint8_t));
    ble_data_recv = xQueueCreate(2, BLE_DATA_RX_SZ);

    ble_status_init();
    ble_data_init();
    ble_ctrl_init();

    // Create task for cmd execution
    int32_t err = xTaskCreate((TaskFunction_t )ble_data_recv_task,
        (const char*    )"ble_rx",
        (uint16_t       )BLE_DATA_RECV_TASK_STK,
        (void*          )NULL,
        (UBaseType_t    )BLE_DATA_RECV_TASK_PRIO,
        (TaskHandle_t*  )NULL);

    return err != pdPASS;
}
