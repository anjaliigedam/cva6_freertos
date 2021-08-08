
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

//#define mainDELAY_LOOP_COUNT                (1000UL) //test_2
#define mainDELAY_LOOP_COUNT                (1000000UL) //test_2

//extern static uart_instance_t * const gp_my_uart = &g_uart_0;

/*-----------------------------------------------------------*/

void test_SingleTask_LEDBlink_LoopDelay( void );
void test_2Task_LED_blink_vdelay( void );


void test_2( void );
void test_3( void );
void test_4( void );


/*-----------------------------------------------------------*/

void vTaskDelay( TickType_t xTicksToDelay ); // test_3

/*-----------------------------------------------------------*/

static void vTask_SingleTask_LEDBlink_LoopDelay( void *pvParameters );
static void vTask_SingleTask_LED_ON( void *pvParameters );
static void vTask_SingleTask_LED_OFF( void *pvParameters );

static void vTask1_LoopDelay( void *pvParameters ); //test_2
static void vTask2_LoopDelay( void *pvParameters ); //test_2

static void vTask1_xTaskDelay( void *pvParameters ); //test_3
static void vTask2_xTaskDelay( void *pvParameters ); //test_3

static void vTask1_xTaskWait( void *pvParameters ); //test_4
static void vTask2_xTaskWait( void *pvParameters ); //test_4

/*-----------------------------------------------------------*/



/*-----------------------------------------------------------*/
void test_SingleTask_LEDBlink_LoopDelay( void )
{
    xTaskCreate(vTask_SingleTask_LEDBlink_LoopDelay, "Task1", 1000, NULL, 1, NULL );
    
    /* Start the kernel.  From here on, only tasks and interrupts will run. */
    vTaskStartScheduler();
}
/*-----------------------------------------------------------*/

void test_2Task_LED_blink_vdelay( void )
{
    xTaskCreate(vTask_SingleTask_LED_ON, "Task1", 1000, NULL, 1, NULL );
    xTaskCreate(vTask_SingleTask_LED_OFF, "Task2", 1000, NULL, 1, NULL );
    
    /* Start the kernel.  From here on, only tasks and interrupts will run. */
    vTaskStartScheduler();
}
/*-----------------------------------------------------------*/


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

void test_4( void )
{
    xTaskCreate( vTask1_xTaskWait, "Task1", 1000, NULL, 1, NULL );
    xTaskCreate( vTask2_xTaskWait, "Task2", 1000, NULL, 2, NULL );

    /* Start the kernel.  From here on, only tasks and interrupts will run. */
    vTaskStartScheduler();
}
/*-----------------------------------------------------------*/


/*-------------------------SingleTask_LEDBlink_LoopDelay-------------------------------*/

static void vTask_SingleTask_LEDBlink_LoopDelay( void *pvParameters )
{

    ( void ) pvParameters;
    extern void vToggleLED( void );

    for( ;; )
    {
        vToggleLED();
        //UART_polled_tx(gp_my_uart, "Task_LED_ON\r\n", 14);
        printf("Task_LED_ON\r\n");
        for(int i = 0; i < mainDELAY_LOOP_COUNT; i++)
        {

        }
        
        vToggleLED();
        //UART_polled_tx(gp_my_uart, "Task_LED_OFF\r\n", 15);
        printf("Task_LED_OFF\r\n");
        for(int i = 0; i < mainDELAY_LOOP_COUNT; i++)
        {

        }
        
    }
}

/*-------------------------SingleTask_LED_ON-------------------------------*/

static void vTask_SingleTask_LED_ON( void *pvParameters )
{

    const TickType_t xDelay1s = pdMS_TO_TICKS( 1000 );
    ( void ) pvParameters;
    extern void led_on( void );
    
    for( ;; )
    {
        led_on();
        //UART_polled_tx(gp_my_uart, "Task_LED_ON\r\n", 14);
        printf("Task_LED_ON\r\n");
        vTaskDelay( xDelay1s ); //Delay for a period    '
        
    }
}

/*-------------------------SingleTask_LED_OFF-------------------------------*/

static void vTask_SingleTask_LED_OFF( void *pvParameters )
{
    const TickType_t xDelay1s = pdMS_TO_TICKS( 1000 );
    ( void ) pvParameters;
    extern void led_off( void );

    for( ;; )
    {
        led_off();       
        //UART_polled_tx(gp_my_uart, "Task_LED_OFF\r\n", 14);
        printf("Task_LED_OFF\r\n");
        vTaskDelay( xDelay1s ); //Delay for a period
    }
}


/*-------------------------test_1-------------------------------*/

static void vTestTask1( void *pvParameters )
{
    ( void ) pvParameters;
    
    extern void led_on( void );
    
    for( ;; )
    {
        led_on();
        printf("Task - 1\r\n" );
        //UART_polled_tx(gp_my_uart, "Task1", 5);
        vTaskDelay(10);
    }
}

static void vTestTask2( void *pvParameters )
{
    ( void ) pvParameters;
    
    extern void led_off( void );
    
    for( ;; )
    {
        led_off();
        printf("Task - 2\r\n" );
        //UART_polled_tx(gp_my_uart, "Task2", 5);
        vTaskDelay(5);
    }
}
/*-----------------------------test_2------------------------------*/
static void vTask1_LoopDelay( void *pvParameters )
{
    ( void ) pvParameters;
    volatile uint32_t ul;
    
    extern void led_on( void );
    
    for( ;; )
    {
        led_on();
        printf("Task 1 is running\r\n" );
        //UART_polled_tx(gp_my_uart, "Task1", 5);

        for(ul = 0; ul < mainDELAY_LOOP_COUNT; ul++)
        {

        }
    }
}


static void vTask2_LoopDelay( void *pvParameters )
{    
    ( void ) pvParameters;
    volatile uint32_t ul;

    extern void led_off( void );

    for( ;; )
    {
        led_off();
        printf("Task 2 is running\r\n" );
        //UART_polled_tx(gp_my_uart, "Task2", 5);

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
    
    extern void led_on( void );
    
    for( ;; )
    {
        led_on();
        printf("Task 1 is running\r\n" );
        //UART_polled_tx(gp_my_uart, "Task1", 5);

        vTaskDelay( xDelay250ms ); //Delay for a period    
    }
}
static void vTask2_xTaskDelay( void *pvParameters )
{
    ( void ) pvParameters;
    const TickType_t xDelay250ms = pdMS_TO_TICKS( 250 );

    extern void led_off( void );

    for( ;; )
    {
        led_off();
        printf("Task 2 is running\r\n" );
        //UART_polled_tx(gp_my_uart, "Task2", 5);

        vTaskDelay( xDelay250ms ); //Delay for a period    
    }
}

/*-----------------------------test_4------------------------------*/
static void vTask1_xTaskWait( void *pvParameters )
{
    ( void ) pvParameters;
    volatile uint32_t ul;

    extern void led_on( void );

    for( ;; )
    {
        led_on();
        printf("Task 1 is running\r\n" );
        //UART_polled_tx(gp_my_uart, "Task1", 5);

        for(ul = 0; ul < mainDELAY_LOOP_COUNT; ul++)
        {

        }
    }
}
static void vTask2_xTaskWait( void *pvParameters )
{
    ( void ) pvParameters;
    volatile uint32_t ul;
    
    extern void led_off( void );
    
    for( ;; )
    {
        led_off();
        printf("Task 2 is running\r\n" );
        //UART_polled_tx(gp_my_uart, "Task2", 5);

        for(ul = 0; ul < mainDELAY_LOOP_COUNT; ul++)
        {

        }
    }
}
/*-----------------------------------------------------------*/
