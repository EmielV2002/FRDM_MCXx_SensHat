/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      March 2024
 *
 * \see       NXP. (2024). MCX A153, A152, A143, A142 Reference Manual. Rev. 4,
 *            01/2024. From:
 *            https://www.nxp.com/docs/en/reference-manual/MCXAP64M96FS3RM.pdf
 *
 * \copyright 2024 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include <MCXA153.h>
#include "gpio_output.h"
#include "serial.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timers.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------
static void vTask1(void *parameters);
static void vTask2(void *parameters);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    // SystemCoreClock is not correctly defined in the startup files. Do it 
    // here, because FreeRTOS is configured to use this variable.
    SystemCoreClock = 48000000;
    
    gpio_output_init();
    xSerialPortInit(9600, 128);
    
    vSerialPutString("\r\nFRDM-MCXA153 FreeRTOS demo\r\n");
    vSerialPutString("By Hugo Arends\r\n\r\n");    
    
    /* Create two tasks. Note that a real application should check the return 
    value of the xTaskCreate() call to ensure the task was created
    successfully. */
    xTaskCreate( vTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
    xTaskCreate( vTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 2, NULL );

    /* Start the scheduler so the tasks start executing. */
    vTaskStartScheduler();

    /* If all is well then main() will never reach here as the scheduler will
    now be running the tasks. If main() does reach here then it is likely that
    there was insufficient heap memory available for the idle task to be created.
    Chapter 2 provides more information on heap memory management. */
    for( ;; );
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

/*----------------------------------------------------------------------------*/

void vTask1( void *pvParameters __attribute__((unused)) )
{
    const char *pcTaskName = "Task 1 is running\r\n";    
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // As per most tasks, this task is implemented in an infinite loop.
    for( ;; )
    {
        // Print the name of this task.
    	vSerialPutString(pcTaskName);
        
        // Toggle green RGB LED
        GPIO3->PTOR = (1<<13);
        
        // Delay for a period of 250 milliseconds.
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 250 ) );
    }    
}

/*----------------------------------------------------------------------------*/

void vTask2( void *pvParameters __attribute__((unused)) )
{
    const char *pcTaskName = "Task 2 is running\r\n";
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // As per most tasks, this task is implemented in an infinite loop.
    for( ;; )
    {
        // Print the name of this task.
    	vSerialPutString(pcTaskName);

        // Toggle red RGB LED
      //GPIO3->PTOR = (1<<12);
        
        // Delay for a period of 1000 milliseconds.
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 1000 ) );
    }
}

/*----------------------------------------------------------------------------*/
