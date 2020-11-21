#include "stm32f4xx.h"

void RCC_Configuration(void)
{
    // System Clocks Configuration

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);      // USART2 clock enable
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);       // GPIOA clock enable
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC, ENABLE );     // GPIOC clock enable - user button


}

void GPIO_Configuration(void)   // I/O pins configuration
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;  // PA.2 USART2_T X, PA.3 USART2_RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Alternative Functions: Connect USART pins to AF
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    //  PA5 = LED
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 ;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init( GPIOA, &GPIO_InitStructure );

    // PC13 = USER Button
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init( GPIOC, &GPIO_InitStructure );
}

void USART2_Configuration(void)
{
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = (9600);
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;     // 8 bit word length
    USART_InitStructure.USART_StopBits = USART_StopBits_1;          // one stop bit
    USART_InitStructure.USART_Parity = USART_Parity_No;             // no parity
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // hardware flow control disabled

    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // recieve and transmit enabled
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
}

void OutString(char *s)     // function sends characters until \0
{
    while (*s)
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);   // Wait for Tx Empty
        USART_SendData(USART2, *s++);   // Send Char
    }
}

int main(void) {
    // call all init functions
    RCC_Configuration();
    GPIO_Configuration();
    USART2_Configuration();

    // Process
    OutString("Welcome to Nucleo F446RE\r\n");

    int btData;
    int btSend, btSendReset, iItemsInBuffer = 0;
    char btInputBuffer[255];
    uint16_t o = 111;   // 111 is ascii o, assuming gtkterm sends ascii chars
    uint16_t c = 99;
    uint16_t r = 114;

    while (1)
    {
        // code to change LED status from user button
        btData = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);
        if (!btData)
        {
            btSendReset = 0;
            if (btSend == 0)
            {
                // OutString("Button pressed LED on\r\n");
                // GPIO_ToggleBits(GPIOA, GPIO_Pin_5);
                GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
                btSend = 1;
            }
        } else
        {
            btSend = 0;
            if (btSendReset == 0)
            {
                GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
                // OutString("Button pressed LED off\r\n");
                btSendReset = 1;
            }
        }

        uint16_t Data;
        if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET)// Wait for Char
        {}
        else
        {
            Data = USART_ReceiveData(USART2);// Collect Char

            USART_SendData(USART2, Data); // Echo Char
            OutString("\n");

            if (Data == 'o')
            {
                OutString("You've selected o. LED is now ON\r\n");
                GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
            }
            else if (Data == 'c')
            {
                OutString("You've selected c. LED is now OFF\r\n");
                GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
            }
            else if (Data == 'r')
            {
                if (btSend == 1)
                {    OutString("Button is pressed.\r\n");     }
                else
                {     OutString("Button is released.\r\n");   }
            }
            else
            {
                OutString("Invalid option selected.\r\n");
                break;
            }
        }
    }
}



#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
 
  while (1)
  {}
}
#endif