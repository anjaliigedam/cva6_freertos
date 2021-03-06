/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
 * NOTE 1:  This project provides two demo applications.  A simple blinky
 * style project, and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting in main.c is used to select
 * between the two.  See the notes on using mainCREATE_SIMPLE_BLINKY_DEMO_ONLY
 * in main.c.  This file implements the simply blinky style version.
 *
 * NOTE 2:  This file only contains the source code that is specific to the
 * basic demo.  Generic functions, such FreeRTOS hook functions, and functions
 * required to configure the hardware are defined in main.c.
 ******************************************************************************
 *
 * main_blinky() creates one queue, and two tasks.  It then starts the
 * scheduler.
 *
 * The Queue Send Task:
 * The queue send task is implemented by the taskSend1() function in
 * this file.  taskSend1() sits in a loop that causes it to repeatedly
 * block for 1000 milliseconds, before sending the value 100 to the queue that
 * was created within main_blinky().  Once the value is sent, the task loops
 * back around to block for another 1000 milliseconds...and so on.
 *
 * The Queue Receive Task:
 * The queue receive task is implemented by the taskRec1() function
 * in this file.  taskRec1() sits in a loop where it repeatedly
 * blocks on attempts to read data from the queue that was created within
 * main_blinky().  When data is received, the task checks the value of the
 * data, and if the value equals the expected 100, writes 'Blink' to the UART
 * (the UART is used in place of the LED to allow easy execution in QEMU).  The
 * 'block time' parameter passed to the queue receive function specifies that
 * the task should be held in the Blocked state indefinitely to wait for data to
 * be available on the queue.  The queue receive task will only leave the
 * Blocked state when the queue send task writes to the queue.  As the queue
 * send task writes to the queue every 1000 milliseconds, the queue receive
 * task leaves the Blocked state every 1000 milliseconds, and therefore toggles
 * the LED every 200 milliseconds.
 */

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uart/uart.h"

/* Priorities used by the tasks. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the pdMS_TO_TICKS() macro. */
//#define mainQUEUE_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 10 )

//?? time delay ?? 
#define mainQUEUE_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 200 )

/* The maximum number items the queue can hold.  The priority of the receiving
task is above the priority of the sending task, so the receiving task will
preempt the sending task and remove the queue items each time the sending task
writes to the queue.  Therefore the queue will never have more than one item in
it at any time, and even with a queue length of 1, the sending task will never
find the queue full. */
#define mainQUEUE_LENGTH					( 1 )

/*-----------------------------------------------------------*/

/*
 * Called by main when mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 1 in
 * main.c.
 */
extern uart_instance_t * const gp_my_uart;
extern dummyLoop();
/*
 * The tasks as described in the comments at the top of this file.
 */
static void taskRec1( void *pvParameters );
static void taskSend1( void *pvParameters );

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t xQueue1 = NULL;
static QueueHandle_t xQueue2 = NULL;

/*-----------------------------------------------------------*/


void uart_isr_rx_polling_handler(){
	uint8_t rx_buff[32];
    uint32_t rx_size = 0;

	rx_size = UART_get_rx(gp_my_uart, rx_buff, sizeof(rx_buff));
    if(rx_size > 0)
    {
        printf("\nInt received : Key = %c (%d)\n", rx_buff[0],  rx_buff[0]);
    }
}

unsigned int ulValueToSend8 = 0;

static void taskSend1( void *pvParameters )
{
	TickType_t xNextWakeTime;
	BaseType_t xReturned;
	int i = 0;
	printf( "Calling %s\n", __func__ );
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* Place this task in the blocked state until it is time to run again. */
		vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

		/* Send to the queue - causing the queue receive task to unblock and
		toggle the LED.  0 is used as the block time so the sending operation
		will not block - it shouldn't need to block as the queue should always
		be empty at this point in the code. */
		ulValueToSend8++;
		printf("taskSend1()\n");
		xReturned = xQueueSend( xQueue1, &ulValueToSend8, 0U );
		uart_isr_rx_polling_handler();
		
		configASSERT( xReturned == pdPASS );
	}
}
/*-----------------------------------------------------------*/


static void taskRecSend1( void *pvParameters )
{
	unsigned int ulReceivedValue;
	const unsigned int ulExpectedValue = 100;
	const char * const pcPassMessage = "Blink\r\n";
	const char * const pcFailMessage = "Unexpected value received\r\n";
	extern void vSendString( const char * const pcString );
	extern void vToggleLED( void );

	printf( "Calling %s\n", __func__ );
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;
	
	for( ;; )
	{
		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h. */
		xQueueReceive( xQueue1, &ulReceivedValue, portMAX_DELAY );
		printf("taskRecSend1 : Receive()\n");
		xQueueSend( xQueue2, &ulValueToSend8, 0U );
		printf("taskRecSend1 : Send()\n");
		
		uart_isr_rx_polling_handler();
	  
	}
}


static void taskRec1( void *pvParameters )
{
	unsigned int ulReceivedValue;
	const unsigned int ulExpectedValue = 100;
	const char * const pcPassMessage = "Blink\r\n";
	const char * const pcFailMessage = "Unexpected value received\r\n";
	extern void vSendString( const char * const pcString );
	extern void vToggleLED( void );

	printf( "Calling %s\n", __func__ );
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;
	
	for( ;; )
	{
		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h. */
		xQueueReceive( xQueue2, &ulReceivedValue, portMAX_DELAY );
		printf("taskRec1()\n");

		/*  To get here something must have been received from the queue, but
		is it the expected value?  If it is, toggle the LED. */
		if( ulReceivedValue == ulValueToSend8 )
		{
			//vSendString( pcPassMessage );
			vToggleLED();
			ulReceivedValue = 0U;
		}
		else
		{
			vSendString( pcFailMessage );
		}
		uart_isr_rx_polling_handler();
	  
	}
}

/*-----------------------------------------------------------*/

void main_polling_demo( void )
{

	printf( "\n==== Calling %s ====\n", __func__);
	
	/* Create the queue. */
	
	printf("\n==== Creating Queue ====\n" );
	
	xQueue1 = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );
	xQueue2 = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

	if( xQueue1 != NULL )
	{
	    printf("\n==== Creating two tasks ====\n" );
		/* Start the two tasks as described in the comments at the top of this
		file. */
		xTaskCreate( taskRec1,				/* The function that implements the task. */
					"Rx", 								/* The text name assigned to the task - for debug only as it is not used by the kernel. */
					configMINIMAL_STACK_SIZE * 2U, 		/* The size of the stack to allocate to the task. */
					NULL, 								/* The parameter passed to the task - not used in this case. */
					mainQUEUE_RECEIVE_TASK_PRIORITY, 	/* The priority assigned to the task. */
					NULL );								/* The task handle is not required, so NULL is passed. */
		printf( "\n==== Created taskRec1 ====\n");

		xTaskCreate( taskRecSend1, "Tx", 
						configMINIMAL_STACK_SIZE * 2U, NULL, 
						mainQUEUE_SEND_TASK_PRIORITY, NULL );
		printf( "\n==== Created taskRecSend1 ====\n");


		xTaskCreate( taskSend1, "Tx", 
						configMINIMAL_STACK_SIZE * 2U, NULL, 
						mainQUEUE_SEND_TASK_PRIORITY, NULL );
		printf( "\n==== Created taskSend1 ====\n" );

		// delay - not needed. Added for demo. 
		for(int i = 0; i < 10; i++){
			dummyLoop();
		}
		
	    printf( "Starting scheduler (vTaskStartScheduler)\n" );
		/* Start the tasks and timer running. */
		vTaskStartScheduler();
				
	}

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the Idle and/or
	timer tasks to be created.  See the memory management section on the
	FreeRTOS web site for more details on the FreeRTOS heap
	http://www.freertos.org/a00111.html. */
	for( ;; );
}
/*-----------------------------------------------------------*/
