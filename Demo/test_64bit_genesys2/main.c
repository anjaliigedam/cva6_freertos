/*
    FreeRTOS V8.2.3 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution and was contributed
    to the project by Technolution B.V. (www.technolution.nl,
    freertos-riscv@technolution.eu) under the terms of the FreeRTOS
    contributors license.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*
 * main() creates a set of standard demo task and a timer.
 * It then starts the scheduler.  The web documentation provides
 * more details of the standard demo application tasks, which provide no
 * particular functionality, but do provide a good example of how to use the
 * FreeRTOS API.
 *
 *
 * In addition to the standard demo tasks, the following tasks and timer are
 * defined and/or created within this file:
 *
 * "Check" software timer - The check timer period is initially set to three
 * seconds.  Its callback function checks that all the standard demo tasks, and
 * the register check tasks, are not only still executing, but are executing
 * without reporting any errors.
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Common demo includes. */
#include "blocktim.h"
#include "countsem.h"
#include "recmutex.h"

/* RISCV includes */
#include "arch/syscalls.h"
#include "arch/clib.h"
#include "uart/uart.h"
#include "encoding.h"
#include "plic.h"

// clib.c - putchar is updated to route printf to uart
static uart_instance_t * const gp_my_uart = &g_uart_0;
static int g_stdio_uart_init_done = 0;
int64_t tick_count = 0;

#ifndef STDIO_BAUD_RATE
#define STDIO_BAUD_RATE  UART_115200_BAUD
#endif

// For Verilator = 1, for FPGA = 0
int use_verilator = 0;


/* The period after which the check timer will expire provided no errors have
been reported by any of the standard demo tasks.  ms are converted to the
equivalent in ticks using the portTICK_PERIOD_MS constant. */
//#define mainCHECK_TIMER_PERIOD_MS            ( 3000UL / portTICK_PERIOD_MS )
#define mainCHECK_TIMER_PERIOD_MS            ( 300UL / portTICK_PERIOD_MS )

/* A block time of zero simply means "don't block". */
#define mainDONT_BLOCK                        ( 0UL )

//#define mainDELAY_LOOP_COUNT                (1000UL) //test_2
/* test_1 */
/*
 * The check timer callback function, as described at the top of this file.
 */
static void prvCheckTimerCallback( TimerHandle_t xTimer );

/*
 * FreeRTOS hook for when malloc fails, enable in FreeRTOSConfig.
 */
void vApplicationMallocFailedHook( void );

/*
 * FreeRTOS hook for when freertos is idling, enable in FreeRTOSConfig.
 */
void vApplicationIdleHook( void );
void vApplicationTickHook(void);
/*
 * FreeRTOS hook for when a stackoverflow occurs, enable in FreeRTOSConfig.
 */
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );



/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/
#define LEDS_ADDR     0x40000000
#define LEDS (*((volatile unsigned int *) (LEDS_ADDR + 0x0)))
static int LED_STATE = 0x0f;

void led_on(){
  //__asm volatile("sw 0x40000000, 0xffff");
  //=====
  //__asm volatile("li x5, 0x40000000");
  //__asm volatile("li x6, 0xff");
  //__asm volatile("sd x6, 0(x5)");
  LEDS = 0xff;

}

void led_off(){
  //__asm volatile("sw 0x40000000, 0xffff");
  //=====
  //__asm volatile("li x5, 0x40000000");
  //__asm volatile("li x6, 0x00");
  //__asm volatile("sd x6, 0(x5)");
  LEDS = 0x00;

}

//----------------------------------------
// main_baremetal.c
void main_baremetal( void );

//----------------------------------------
// main_math_demo.c
void main_floating_mult_demo( void );
void main_floating_div_demo( void );

// main_waveform_demo.c
void main_LED_demo( void );
void main_waveform_demo( void);
//----------------------------------------

void main_blinky( void );
void main_full( void );
void main_full2( void );
void main_interrupt_demo( void );
void main_polling_demo( void );
void semaphore_demo( void );
void attack_demo( void );
void attack_without_disruptor( void );

void test_SingleTask_LEDBlink_LoopDelay( void );
void test_2Task_LED_blink_vdelay( void );

void test_2( void );
void test_3( void );
void test_4( void );

int dummyLoop2(int i){
    
    int loop_count = 100000*i;
    
    if(use_verilator == 1)
    {
        loop_count = 10;
    }

    //for
    for(i = 0; i < loop_count; i++)
    {
        i++;
    }

    return i;
}


void uart1_rx_handler(uart_instance_t * this_uart)
{
    uint32_t rx_size; 
    uint8_t rx_buff[32];
    uint32_t rx_idx  = 0;
    rx_size = UART_get_rx(this_uart, rx_buff, sizeof(rx_buff));  
    printf("uart1_rx_handler interrupt called\n");
    dummyLoop();
    dummyLoop();
    dummyLoop();
    dummyLoop();
    
}

void user_irq_handler()
{
    printf("\n==== user_irq_handler called ====\r\n");
 	int interrupt_id = 0;
	uint32_t *interrupt_claim_address = NULL;
    int int_pending = PLIC->PENDING_ARRAY[0];
	printf( "plic interrupt pending = %d\n", int_pending);
 		
   	uint32_t mcause  = read_csr(mcause);
	printf( "mcause = 0x%x\n", mcause);
 	
    interrupt_id = PLIC_ClaimIRQ();
	printf("interrupt_id from PLIC = %d\r\n", interrupt_id);
    
    int_pending = PLIC->PENDING_ARRAY[0];
	printf( "interrupt pending post claim= %d\n", int_pending);
 	
    uint8_t rx_buff[32];
    uint32_t rx_size = 0;
	rx_size = UART_get_rx(gp_my_uart, rx_buff, sizeof(rx_buff));
    printf("typed = %c\r\n", rx_buff[0]);
	
    PLIC_CompleteIRQ(interrupt_id);
	
    		
	dummyLoop2(500);
    
    printf("==== user_irq_handler interrupt EXIT ====\n");
    
}



int dummyLoop(){
    int i;
    int loop_count = 100000;
    
    if(use_verilator == 1)
    {
        loop_count = 10;
    }

    //for
    for(i = 0; i < loop_count; i++)
    {
        i++;
    }

    return i;
}

void LED_INIT_TEST(){
    printf("LED_INIT_TESTING START\n");
    for(int i = 0; i< 3; i++){
        LEDS = 0x00;
        dummyLoop();
        LEDS = 0x01;
        dummyLoop();
        LEDS = 0x02;
        dummyLoop();
        LEDS = 0x04;
        dummyLoop();
        LEDS = 0x08;
        dummyLoop();
        LEDS = 0x10;
        dummyLoop();
        LEDS = 0x20;
        dummyLoop();
        LEDS = 0x40;
        dummyLoop();
        LEDS = 0x80;
        dummyLoop();
        
        
    }
    
    printf("LED_INIT_TESTING DONE\n");
    dummyLoop();
}


void enable_interrupt_main()
{

    UART_set_rx_handler(&g_uart_0,
                            uart1_rx_handler,
                            UART_FIFO_FOUR_BYTES);
}


int main( void )
{
    printf("main.c\n");
    //set_csr(mie, 0xffffffff);
    //set_csr(mip, 0xffffffff);
    //set_csr(mideleg, 0xffffffff);
    
    dummyLoop();
    
    uint32_t mstatus  = read_csr(mstatus);
    dummyLoop();

	printf( "mstatus = 0x%x mpp = 0x%x mie = 0x%x mpie = 0x%x\n", 
            mstatus,  (mstatus >> 11) & 0x3, (mstatus >> 3) & 0x1, (mstatus >> 7) & 0x1);
    //set_csr(mstatus, 0x1888);
    mstatus  = read_csr(mstatus);
	printf( "mstatus = 0x%x mpp = 0x%x mie = 0x%x mpie = 0x%x\n", mstatus,  (mstatus >> 11) & 0x3, (mstatus >> 3) & 0x1, (mstatus >> 7) & 0x1  );
    
    dummyLoop();
    //printf("EXIT");
    //return 1;

    UART_init(gp_my_uart,
                  STDIO_BAUD_RATE,
                  UART_DATA_8_BITS | UART_NO_PARITY | UART_ONE_STOP_BIT);
                      
    g_stdio_uart_init_done = 1;

    printf("UART INIT DONE\n");
    printf("HELLO CVA6\n");

    if(use_verilator == 0)
    {
        LED_INIT_TEST();
    }
    //LED_INIT_TEST();
    

    uint8_t rx_buff[32];
    uint32_t rx_idx  = 0;
    uint32_t rx_size  = 0;

    
    printf("==========================\n");
    printf("\nEnter below key for demo:\n");
    printf("a -> LED blinky with 2 task and communication between task usig Queue\n");
    printf("b -> 3 task with interrupt on UART RX from hyper-terminal. HAS ISSUES UNDER DEBUG!. \n");
    printf("c -> Polling on UART RX from hyper-terminal.\n");
    printf("d -> main_full()\n");
    printf("e -> Semaphore testcase\n");
    printf("f -> Attack demo without Disruptor\n");
    printf("g -> Attack demo with Disruptor\n");
    printf("z -> Baremetal test\n");
    printf("==========================\n");

    printf("\n\n==========================\n");
    printf("\nEnter below key for demo:\n");
    printf("1 -> LED Demo \n");
    printf("2 -> Oscilliscope Waveform Demo \n");
    printf("3 -> PLIC Demo - UART interrupt + GPIO Interrupt \n");
    
    printf("==========================\n");



    if(use_verilator == 0)
    {
        while(1)
        { 
            rx_size = UART_get_rx(gp_my_uart, rx_buff, sizeof(rx_buff));
            if(rx_size > 0)
            {
                printf("Received = %c (%d)\n", rx_buff[0],  rx_buff[0]);
                break;
            }
        }
    }
    else
    {
        //rx_buff[0] = 102; //f - attack code without DP
        rx_buff[0] = 122; //z - main_baremetal
    }


    dummyLoop();
    
    printf("switching to selected testcase\n");

    if(rx_buff[0] == 97){   // a
        enable_interrupt_main();
        main_blinky();
    }
    else if(rx_buff[0] == 98){  // b
        enable_interrupt_main();  
        main_interrupt_demo();
    }
    else if(rx_buff[0] == 99){  // c 
        main_polling_demo();
    }
    else if(rx_buff[0] == 100){ // d
        main_full();
    }
    else if(rx_buff[0] == 101){ // e
        semaphore_demo();
    }
    else if(rx_buff[0] == 102){ //f
        attack_demo();
    }
    else if(rx_buff[0] == 103){ //g
        attack_without_disruptor();
    }
    else if(rx_buff[0] == 122){ //z
        enable_interrupt_main();
        main_baremetal();
    }
    //==========================
    //------------- NEW ----------
    //==========================
    

    else if(rx_buff[0] == 49){ //1
        main_LED_demo();
    }
    else if(rx_buff[0] == 50){ //2
        main_waveform_demo();
    }
    else if(rx_buff[0] == 51){ //3
        // PLIC with GPIO and UART interrupt
    }

    printf("EXIT  - FreeRTOS should not reach here");
    /* Exit FreeRTOS */
    return 0;
}
/*-----------------------------------------------------------*/


/* See the description at the top of this file. */
static void prvCheckTimerCallback(__attribute__ ((unused)) TimerHandle_t xTimer )
{
    unsigned long ulErrorFound = pdFALSE;
    printf("prvCheckTimerCallback\n");
        /* Check all the demo and test tasks to ensure that they are all still
    running, and that none have detected an error. */
    //printf("debug1");
    if( xAreBlockTimeTestTasksStillRunning() != pdPASS )
    {
        printf("Error in block time test tasks \n");
        ulErrorFound |= ( 0x01UL << 1UL );
    }
    //printf("debug2");
    if( xAreCountingSemaphoreTasksStillRunning() != pdPASS )
    {
        printf("Error in counting semaphore tasks \n");
        ulErrorFound |= ( 0x01UL << 2UL );
    }
    //printf("debug3");
    if( xAreRecursiveMutexTasksStillRunning() != pdPASS )
    {
        printf("Error in recursive mutex tasks \n");
        ulErrorFound |= ( 0x01UL << 3UL );
    }
    //printf("debug4");
    if( ulErrorFound != pdFALSE )
    {
        __asm volatile("li t6, 0xbeefdead");
        printf("Error found! \n");
    }else{
        __asm volatile("li t6, 0xdeadbeef");
        printf("PASS! \n");
    }

    /* Stop scheduler */
    vTaskEndScheduler();
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    printf("==== vApplicationMallocFailedHook === \n");
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    /* vApplicationTickHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationTickHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
    tick_count++;
    //printf("TICK = %d\n", tick_count);
}

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
    //printf("IDLE TASK\n");
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}



void LEDInit( void )
{
    LEDS = LED_STATE;

}


void vToggleLED( void )
{
	//gpio_pin_toggle( 0x5 );
    
    LEDS = LED_STATE;
    LED_STATE = ~LED_STATE;

}

void vSendString( const char * const pcString )
{
	// TODO: Uart dumping
	printf( "%s", pcString );
}

