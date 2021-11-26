
/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "uart/uart.h"
#include "uart/plic.h"

extern uart_instance_t * const gp_my_uart;
extern void uart_isr_rx_polling_handler();

void main_baremetal( void )
{
	printf( "\n==== Calling %s ====\n", __func__);
	uint32_t plic_en  = PLIC->HART0_MMODE_ENA[0];
	uint32_t plic_en1  = PLIC->HART0_MMODE_ENA[1];
	
	uint32_t int_pending = PLIC->PENDING_ARRAY[0];
	uint8_t rx_buff[32];
    uint32_t rx_size = 0;
	
	
	printf( "\n plic_en = %d plic_en1 = %d\n", plic_en, plic_en1);
	while(1){
		plic_en  = PLIC->HART0_MMODE_ENA[0];
		plic_en1  = PLIC->HART0_MMODE_ENA[1];
		int_pending = PLIC->PENDING_ARRAY[0];

		printf( "plic_en = %d plic_en1 = %d\n", plic_en,plic_en1);
		printf( "plic interrupt pending = %d\n", int_pending);
 		
		uint64_t mie  = read_csr(mie);
		printf( "mie = 0x%x\n ", mie);

 		
		uint32_t mstatus  = read_csr(mstatus);
		printf( "mstatus = 0x%x mpp = 0x%x mie = 0x%x mpie = 0x%x\n", mstatus,  (mstatus >> 11) & 0x3, (mstatus >> 3) & 0x1, (mstatus >> 7) & 0x1  );
 		
		uint32_t mip  = read_csr(mip);
		printf( "mip = 0x%x\n", mip);
 		
		uint32_t mtvec  = read_csr(mtvec);
		printf( "mtvec = 0x%x\n", mtvec);
 		
		uint32_t mcause  = read_csr(mcause);
		printf( "mcause = 0x%x\n", mcause);
 		
		uint32_t mepc  = read_csr(mepc);
		printf( "mepc = 0x%x\n", mepc);
 		
		uint32_t mideleg  = read_csr(mideleg);
		printf( "mideleg = 0x%x\n", mideleg);
 		

		// jump to mtvec goes to TRAP
		//void *address = mtvec;
		//goto *address;

		//printf( "\n UART_get_rx_status = %d\n", UART_get_rx_status());
		//printf( "\n UART_get_modem_status = %d\n", UART_get_modem_status());

		//uart_isr_rx_polling_handler();


		for(int i = 0; i < 1000000; i++){
		

		}
		//PLIC_ClaimIRQ();
		//PLIC_ClearPendingIRQ();
	}	
	
	/*
	printf( "\n plic_en = %d\n", plic_en);
	while(1){
		
		rx_size = UART_get_rx(gp_my_uart, rx_buff, sizeof(rx_buff));
    	if(rx_size > 0)
    	{
      	  printf("\nInt received : size = %d Key = %s\n",rx_size, rx_buff);
    	}
		
		for(int i = 0; i < 10000000; i++){
		

		}
		
	}
	*/
	
	for(;;); 	
	printf( "\n==== Exiting %s ====\n", __func__);	
}


