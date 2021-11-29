
/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "uart/uart.h"
#include "uart/plic.h"

extern uart_instance_t * const gp_my_uart;
extern void uart_isr_rx_polling_handler();

#define LEDS_ADDR     0x40000000
#define LEDS (*((volatile unsigned int *) (LEDS_ADDR + 0x0)))

void delay_call(int delay)
{
	for(int i = 0; i < delay*5000; i++)
	{}
}

void LED_PATTERN_run(int delay){
	LEDS = 0x00;
    delay_call(delay);
	LEDS = 0x01;
    delay_call(delay);
    LEDS = 0x02;
    delay_call(delay);
    LEDS = 0x04;
    delay_call(delay);
    LEDS = 0x08;
    delay_call(delay);
    LEDS = 0x10;
    delay_call(delay);
    LEDS = 0x20;
    delay_call(delay);
    LEDS = 0x40;
    delay_call(delay);
    LEDS = 0x80;
    delay_call(delay);    
}


void LED_PATTERN_merge(int delay){
	LEDS = 0x81; 	// 1000 0001
    delay_call(delay);
	LEDS = 0x42; // 0100 0010
    delay_call(delay);
    LEDS = 0x24; // 020 0010
    delay_call(delay);
    LEDS = 0x18;
    delay_call(delay);
    LEDS = 0x00;
    delay_call(delay);
    
}

void LED_PATTERN_blink(int delay){
    
	for(int i = 0; i< 5; i++){
        LEDS = 0x00;
        delay_call(delay);
        LEDS = 0xff;
		delay_call(delay);        
    }
    
}


void waveform_generation(int delay_on, int delay_off){
    
    LEDS = 0x00; //TO-DO change LED to GPIO
    delay_call(delay_off);
    LEDS = 0xff;
	delay_call(delay_on);        

}


void main_LED_demo( void )
{
	printf("\n\n====main_LED_Demo()==== \n\n");
	printf( "1 -> LED_PATTERN_1\n");
	printf( "2 -> LED_PATTERN_2\n");
	printf( "3 -> LED_PATTERN_3\n");
	printf( "4 -> LED_PATTERN_4\n");
	printf( "5 -> LED_PATTERN_5\n");		

    uint32_t rx_size; 
    uint8_t rx_buff[32];
	uint8_t temp = "1";

	while(1){
		rx_size = UART_get_rx(gp_my_uart, rx_buff, sizeof(rx_buff));
		if(rx_size > 0)
		{
			temp = rx_buff[0];
			printf("\n==== Entered %c ====\r\n", temp);
		} 

		if(temp == 49){ //1
			LED_PATTERN_run(10);
		}
		else if(temp == 50){ //2
			LED_PATTERN_run(100);
		}
		else if(temp == 51){ //3
			LED_PATTERN_blink(10);
		}
		else if(temp == 52){ //4
			LED_PATTERN_blink(100);
		}
		else if(temp == 53){ //5
			LED_PATTERN_merge(10);
		}
		else if(temp == 54){ //6
			LED_PATTERN_merge(100);
		}
	}
	printf( "\n==== Exiting %s ====\n", __func__);	
}


