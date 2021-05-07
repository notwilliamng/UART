#include "msp.h"


/**
 * main.c
 */
void UART_init(void);
void UART_write_string(const char* string);
void UART_esc_code(const char* esc_code);

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    UART_init();
    UART_esc_code("[2J");//clear display
    UART_esc_code("[H");//cursor to top left
    UART_write_string("Hello");
    UART_esc_code("[5B");//move down 5 lines
    UART_esc_code("[5D");//move left 5 lines
    UART_write_string("World!");
    UART_esc_code("[5B");//move down 5 lines
    UART_esc_code("[6D");//move left 6 lines
    UART_write_string("Input:");
    UART_esc_code("[1C");//move right 1 lines

    //
    while(1);
}

void UART_init(void){
    EUSCI_A0 -> CTLW0 |=(EUSCI_A_CTLW0_SWRST);//software reset
    EUSCI_A0 -> CTLW0 |=(EUSCI_A_CTLW0_SWRST|//software reset
                            EUSCI_A_CTLW0_SSEL__SMCLK);//select SMCLK

    EUSCI_A0 -> BRW = 1;//n>16
    EUSCI_A0 -> MCTLW |= (10<< EUSCI_A_MCTLW_BRF_OFS)|1; //oversampling 16 times

    P1 ->SEL1 &= ~(BIT2|BIT3);//P1.2 Rx, P1.3Tx
    P1 ->SEL0 |=(BIT2|BIT3);//config GPIO

    EUSCI_A0 -> CTLW0 &= ~(EUSCI_A_CTLW0_SWRST);//take out of sw reset

    EUSCI_A0 -> IFG &= ~EUSCI_A_IFG_RXIFG;//clear interupt flag
    EUSCI_A0 ->IE |= EUSCI_A_IE_RXIE;//enable interrupt on the receive
    NVIC-> ISER[0] = 1<<((EUSCIA0_IRQn) & 31);//enable interrupt module
    __enable_irq();//enable interrupts global

}

void UART_write_string(const char* string){
    uint8_t letter =0;//index of string
    while(string[letter]!=0){//if there's an input
        while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));//wait for txbuf to be empty
        EUSCI_A0 -> TXBUF = string[letter];//send
        letter++;//next
    }
}

void UART_esc_code(const char* esc_code){//same as write string
    uint8_t letter =0;//
    while(!(EUSCI_A0->IFG &EUSCI_A_IFG_TXIFG));
    EUSCI_A0->TXBUF = 0x1B;//escape
    while(esc_code[letter]!=0){
        while(!(EUSCI_A0-> IFG &EUSCI_A_IFG_TXIFG));
        EUSCI_A0 -> TXBUF = esc_code[letter];
        letter++;
    }
}


void EUSCIA0_IRQHandler(void){
    if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG){//check flag
        while (!(EUSCI_A0->IFG &EUSCI_A_IFG_TXIFG));//wait for TXBUF to be empty
        EUSCI_A0->TXBUF = EUSCI_A0-> RXBUF;//echo receive character
    }
}
