
/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "uart/uart.h"
#include "uart/plic.h"

extern uart_instance_t * const gp_my_uart;
extern void uart_isr_rx_polling_handler();



void main_floating_mult_demo( void )
{

	int aa, bb, cc;
	//===============================
	aa = 4; bb = 4; cc = aa * bb; ;
	printf("a = %d b = %d result = %d\n",aa, bb, cc); 	
	
	printf( "\n==== Calling %s ====\n", __func__);
	printf("0\n");
	
	float a, b , result;
	// mult-0
	//===============================
	a = 1.234; b = 1; result = (float)(a * b);
	printf("1\n");
	printf("a = %d b = %d result = %d\n",a, b, result); 	
	printf("2\n");
	//===============================
	a = 1; b = 1.234; result = a * b; 
	printf("a = %d b = %d result = %ld\n",a, b, result); 	
	
	// mult-signed value
	//===============================
	a = 1.234; b = 1.1; result = a * b; 
	printf("a = %d b = %d result = %d\n",a, b, result); 	
	//===============================
	a = 1.234; b = -1.1; result = a * b; 
	printf("a = %d b = %d result = %d\n",a, b, result); 	
	//===============================
	a = -1.234; b = 1.1; result = a * b; 
	printf("a = %d b = %d result = %d\n",a, b, result); 	
	//===============================
	a = -1.234; b = -1.1; result = a * b; 
	printf("a = %d b = %d result = %d\n",a, b, result); 	
	//===============================
	
	printf( "\n==== Exiting %s ====\n", __func__);	
	while(1){}
}


void main_floating_div_demo( void )
{

	printf( "\n==== Calling %s ====\n", __func__);
	float a, b , result;
	// mult-0
	//===============================
	a = 1.234; b = 0; result = a / b; 
	printf("a = %d b = %d result = %d\n",a, b, result); 	
	//===============================
	a = 0; b = 1.234; result = a / b; 
	printf("a = %d b = %d result = %d\n",a, b, result); 	
	
	// mult-signed value
	//===============================
	a = 1.234; b = 1.1; result = a / b; 
	printf("a = %d b = %d result = %d\n",a, b, result); 	
	//===============================
	a = 1.234; b = -1.1; result = a / b; 
	printf("a = %d b = %d result = %d\n",a, b, result); 	
	//===============================
	a = -1.234; b = 1.1; result = a / b; 
	printf("a = %d b = %d result = %d\n",a, b, result); 	
	//===============================
	a = -1.234; b = -1.1; result = a / b; 
	printf("a = %d b = %d result = %d\n",a, b, result); 	
	//===============================
	
	printf( "\n==== Exiting %s ====\n", __func__);	
	while(1){}

}
