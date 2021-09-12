
/* FreeRTOS kernel includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

/* c stdlib */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#define SEM_TASK_PRIORITY	(tskIDLE_PRIORITY + 1)
#define SEM_TASK_PRIORITY	1
#define SEM_STACK_SIZE	     ((unsigned short)configMINIMAL_STACK_SIZE)

void semaphore_demo(void);
void prvSemaphoreTest( void * pvParameters );
void vStartSemaphoreTasks_2();

SemaphoreHandle_t xSemaphore = NULL;

void semaphore_demo(void)
{
	printf("Start semaphore tasks\n");
	vStartSemaphoreTasks_2(SEM_TASK_PRIORITY);

	printf("Start Scheduler\n");
	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the Idle and/or
	timer tasks to be created.  See the memory management section on the
	FreeRTOS web site for more details on the FreeRTOS heap
	http://www.freertos.org/a00111.html. */
	for( ;; );
}


/* A task that creates a semaphore. */
void vStartSemaphoreTasks_2()
{
    /* Create the semaphore to guard a shared resource.  As we are using
    the semaphore for mutual exclusion we create a mutex semaphore
    rather than a binary semaphore. */
    xSemaphore = xSemaphoreCreateBinary();

    xTaskCreate(prvSemaphoreTest, "Sem_Task1", SEM_STACK_SIZE,
			NULL, tskIDLE_PRIORITY + 1, (TaskHandle_t *)NULL);
	//xTaskCreate(prvSemaphoreTest, "Sem_Task2", SEM_STACK_SIZE,
	//		NULL, tskIDLE_PRIORITY + 2, (TaskHandle_t *)NULL);

}


/* A task that uses the semaphore. */
void prvSemaphoreTest( void * pvParameters )
{
    /* ... Do other things. */
    volatile unsigned long ulSharedVariable, ulExpectedValue;

    char *pcTaskName;
    pcTaskName = ( char * ) pvParameters;

    for(;;)
    {
        //printf("\nSem_value=%d\n",xSemaphore);
        if( xSemaphore != NULL )
        {
            /* See if we can obtain the semaphore.  If the semaphore is not
            available wait 10 ticks to see if it becomes free. */

            xSemaphoreGive( xSemaphore ); //works
            printf( pcTaskName );

            //if( xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE ) //don't work with portMAX_DELAY
            if( xSemaphoreTake( xSemaphore, ( TickType_t ) 0 ) == pdTRUE )
            {
                /* We were able to obtain the semaphore and can now access the
                shared resource. */
                //	if (ulSharedVariable != ulExpectedValue) {
                    printf( pcTaskName );
                    printf("\nSemaphore acquired by Sem_Task1\n");
                //}
                /* ... */

                /* We have finished accessing the shared resource.  Release the
                semaphore. */
                //xSemaphoreGive( xSemaphore );
                //printf( pcTaskName );
                printf("\nSemaphore released by Sem_Task1\n");				
            }
            else
            {
                /* We could not obtain the semaphore and can therefore not access
                the shared resource safely. */
                printf("\nCould not obtain the semaphore\n");
            }
        }
    }
    
}
