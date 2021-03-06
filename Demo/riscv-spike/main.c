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

/* The period after which the check timer will expire provided no errors have
been reported by any of the standard demo tasks.  ms are converted to the
equivalent in ticks using the portTICK_PERIOD_MS constant. */
//#define mainCHECK_TIMER_PERIOD_MS			( 3000UL / portTICK_PERIOD_MS )
#define mainCHECK_TIMER_PERIOD_MS			( 300UL / portTICK_PERIOD_MS )

/* A block time of zero simply means "don't block". */
#define mainDONT_BLOCK						( 0UL )

#define mainDELAY_LOOP_COUNT				(100UL) //test_2

/*-----------------------------------------------------------*/

void test_1( void );
void test_2( void );
void test_3( void );

/*-----------------------------------------------------------*/

void vTaskDelay( TickType_t xTicksToDelay ); // test_3

/*-----------------------------------------------------------*/
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

/*
 * FreeRTOS hook for when a stackoverflow occurs, enable in FreeRTOSConfig.
 */
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );

/*-----------------------------------------------------------*/

static void vTestTask1( void *pvParameters ); //test_1
static void vTestTask2( void *pvParameters ); //test_1

static void vTask1_LoopDelay( void *pvParameters ); //test_2
static void vTask2_LoopDelay( void *pvParameters ); //test_2

static void vTask1_xTaskDelay( void *pvParameters ); //test_3
static void vTask2_xTaskDelay( void *pvParameters ); //test_3
/*-----------------------------------------------------------*/

int main( void )
{
	/* Test_1 */
	//test_1(); //task-priority same

	/* Test_2 */
	//test_2(); //different task-priority 

	/* Test_3 */
	test_3(); //different task-priority and xTaskDelay() API

	/* Test_4 */
	//test_4(); //queues 

	/* Exit FreeRTOS */
	return 0;
}
/*-----------------------------------------------------------*/
void test_1( void )
{
	TimerHandle_t xCheckTimer = NULL;

	/* Create the standard demo tasks, including the interrupt nesting test
	tasks. */

	vCreateBlockTimeTasks();

	vStartCountingSemaphoreTasks();

	vStartRecursiveMutexTasks();

	/* Create the software timer that performs the 'check' functionality,
	as described at the top of this file. */

	xCheckTimer = xTimerCreate( "CheckTimer",					/* A text name, purely to help debugging. */
								( mainCHECK_TIMER_PERIOD_MS ),	/* The timer period, in this case 3000ms (3s). */
								pdTRUE,							/* This is an auto-reload timer, so xAutoReload is set to pdTRUE. */
								( void * ) 0,					/* The ID is not used, so can be set to anything. */
								prvCheckTimerCallback			/* The callback function that inspects the status of all the other tasks. */
							  );

	/* If the software timer was created successfully, start it.  It won't
	actually start running until the scheduler starts.  A block time of
	zero is used in this call, although any value could be used as the block
	time will be ignored because the scheduler has not started yet. */
	if( xCheckTimer != NULL )
	{
		xTimerStart( xCheckTimer, mainDONT_BLOCK );
	}

	xTaskCreate( vTestTask1, "TestTask1", 1000, NULL, uartPRIMARY_PRIORITY, NULL );
	xTaskCreate( vTestTask2, "TestTask2", 1000, NULL, uartPRIMARY_PRIORITY, NULL );

	/* Start the kernel.  From here on, only tasks and interrupts will run. */
	vTaskStartScheduler();

}
/*-----------------------------------------------------------*/
void test_2( void )
{
	xTaskCreate( vTask1_LoopDelay, "Task1", 1000, NULL, 1, NULL );
	xTaskCreate( vTask2_LoopDelay, "Task2", 1000, NULL, 1, NULL );

	/* Start the kernel.  From here on, only tasks and interrupts will run. */
	vTaskStartScheduler();
}
/*-----------------------------------------------------------*/
void test_3( void )
{
	xTaskCreate( vTask1_xTaskDelay, "Task1", 1000, NULL, 1, NULL );
	xTaskCreate( vTask2_xTaskDelay, "Task2", 1000, NULL, 2, NULL );

	/* Start the kernel.  From here on, only tasks and interrupts will run. */
	vTaskStartScheduler();
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
		printf("Error in block time test tasks \r\n");
		ulErrorFound |= ( 0x01UL << 1UL );
	}
	//printf("debug2");
	if( xAreCountingSemaphoreTasksStillRunning() != pdPASS )
	{
		printf("Error in counting semaphore tasks \r\n");
		ulErrorFound |= ( 0x01UL << 2UL );
	}
	//printf("debug3");
	if( xAreRecursiveMutexTasksStillRunning() != pdPASS )
	{
		printf("Error in recursive mutex tasks \r\n");
		ulErrorFound |= ( 0x01UL << 3UL );
	}
	//printf("debug4");
	if( ulErrorFound != pdFALSE )
	{
		__asm volatile("li t6, 0xbeefdead");
		printf("Error found! \r\n");
	}else{
		__asm volatile("li t6, 0xdeadbeef");
		printf("PASS! \r\n");
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
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

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
/*-------------------------test_1-------------------------------*/
static void vTestTask1( void *pvParameters )
{
	( void ) pvParameters;
	for( ;; )
	{
		printf("Task - 1\r\n" );
	    	vTaskDelay(10);
	}
}

static void vTestTask2( void *pvParameters )
{
	( void ) pvParameters;
	for( ;; )
	{
		printf("Task - 2\r\n" );
	    	vTaskDelay(5);
	}
}
/*-----------------------------test_2------------------------------*/
static void vTask1_LoopDelay( void *pvParameters )
{
	( void ) pvParameters;
	volatile uint32_t ul;
	for( ;; )
	{
		printf("Task 1 is running\r\n" );

		for(ul = 0; ul < mainDELAY_LOOP_COUNT; ul++)
		{

		}
	}
}
static void vTask2_LoopDelay( void *pvParameters )
{
	( void ) pvParameters;
	volatile uint32_t ul;
	for( ;; )
	{
		printf("Task 2 is running\r\n" );

		for(ul = 0; ul < mainDELAY_LOOP_COUNT; ul++)
		{

		}
	}
}
/*---------------------------test_3--------------------------------*/
static void vTask1_xTaskDelay( void *pvParameters )
{
	( void ) pvParameters;
	const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );
	for( ;; )
	{
		printf("Task 1 is running\r\n" );

		vTaskDelay( xDelay250ms ); //Delay for a period	
	}
}
static void vTask2_xTaskDelay( void *pvParameters )
{
	( void ) pvParameters;
	const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );
	for( ;; )
	{
		printf("Task 2 is running\r\n" );

		vTaskDelay( xDelay250ms ); //Delay for a period	
	}
}

/*-----------------------------------------------------------*/