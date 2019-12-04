#include "serial.h"
#include "clock.h"

void serial_init( UART_BautRate baud ){
    UART_WaitTxFifoEmpty(UART0);
    //UART_WaitTxFifoEmpty(UART1);

    UART_ConfigTypeDef uart_config;
    uart_config.baud_rate    = baud;
    uart_config.data_bits     = UART_WordLength_8b;
    uart_config.parity          = USART_Parity_None;
    uart_config.stop_bits     = USART_StopBits_1;
    uart_config.flow_ctrl      = USART_HardwareFlowControl_None;
    uart_config.UART_RxFlowThresh = 120;
    uart_config.UART_InverseMask = UART_None_Inverse;
    UART_ParamConfig(UART0, &uart_config);

    UART_SetPrintPort(0);
    
    xTaskCreate(&serial_test_task, "Serial test task", serial_test_task_mem, NULL, serial_test_task_prior, NULL);
    
    // Interrupts are not necessary for now
    /*
    UART_IntrConfTypeDef uart_intr;
    uart_intr.UART_IntrEnMask = UART_RXFIFO_TOUT_INT_ENA | UART_FRM_ERR_INT_ENA | UART_RXFIFO_FULL_INT_ENA | UART_TXFIFO_EMPTY_INT_ENA;
    uart_intr.UART_RX_FifoFullIntrThresh = 10;
    uart_intr.UART_RX_TimeOutIntrThresh = 2;
    uart_intr.UART_TX_FifoEmptyIntrThresh = 20;
    UART_IntrConfig(UART0, &uart_intr);

    UART_SetPrintPort(UART0);
    UART_intr_handler_register(uart0_rx_intr_handler, NULL);
    ETS_UART_INTR_ENABLE();
    */
}

void serial_test_task( void *param ){
    while(1){
        printf("Test Printf. Clock: %d.\n", clock_get());
        vTaskDelay(500/portTICK_RATE_MS);
    }
}

void serial_rx_interruptHandler( void )
{

}
