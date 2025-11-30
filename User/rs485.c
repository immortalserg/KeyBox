#include "rs485.h"

void rs_send(uint8_t *buf, uint32_t size) {

    int i = 0;

    // Enable transceiver output
    GPIO_SetBits(GPIOA, GPIO_Pin_8);

    for(i = 0; i < size; i++)
    {

        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *buf++);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

    }
    
    // Switch transceiver to input
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}

QueueHandle_t rs_recv = NULL;
QueueHandle_t rs_recv_byte = NULL;
uint8_t rs_recv_buf[RS_RX_SZ];

void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART1_IRQHandler(void) {
    // Store new char
    uint8_t dat = USART_ReceiveData(USART1);
    xQueueSendFromISR(rs_recv_byte, &dat, 0);
}

void rs_recv_task(void *unused) {
    uint32_t i = 0;

    while (1) {
        if (xQueueReceive(rs_recv_byte, rs_recv_buf + i + 1, 50) != pdPASS) {
            if (i > 0) {
                rs_recv_buf[0] = i;
                xQueueSend(rs_recv, rs_recv_buf, 0);
                i = 0;
            }
        } else {
            i++;

            if ((i + 1) == RS_RX_SZ) {
                rs_recv_buf[0] = i;
                xQueueSend(rs_recv, rs_recv_buf, 0);
                i = 0;
            }
        }
    }
}

uint32_t rs_init() {
    rs_recv_byte = xQueueCreate(RS_RX_SZ, sizeof(uint8_t));
    rs_recv = xQueueCreate(2, RS_RX_SZ);

    // Enable clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    // TX pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RX pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // DIR pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Enable clock
    RCC_APB1PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    USART_InitTypeDef USART_InitStructure = {0};

    // UART
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
    NVIC_EnableIRQ(USART1_IRQn);

    int32_t err = xTaskCreate((TaskFunction_t )rs_recv_task,
        (const char*    )"rs_rx",
        (uint16_t       )RS_RECV_TASK_STK,
        (void*          )NULL,
        (UBaseType_t    )RS_RECV_TASK_PRIO,
        (TaskHandle_t*  )NULL);

    return err != pdPASS;
}