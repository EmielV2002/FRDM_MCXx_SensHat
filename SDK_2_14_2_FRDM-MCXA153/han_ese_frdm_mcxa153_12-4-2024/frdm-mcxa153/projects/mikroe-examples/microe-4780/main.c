/*! ***************************************************************************
 *
 * \brief     Main application
 * \file      main.c
 * \author    Hugo Arends
 * \date      February 2024
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
#include <stdio.h>
#include "fxls8974cf.h"
#include "lpuart0_interrupt.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
#ifdef DEBUG
#define TARGETSTR "Debug"
#else
#define TARGETSTR "Release"
#endif

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile uint32_t ms = 0;

static uint8_t reg;

static volatile float x_out_mg;
static volatile float y_out_mg;
static volatile float z_out_mg;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------
int main(void)
{
    lpuart0_init(115200);
    fxls8974cf_init();
    
    // Generate an interrupt every 1 ms
    SysTick_Config(48000);   
    
    printf("Microe 4780: Accel 4 click - example project");
    printf(" - %s\r\n", TARGETSTR);
    printf("Build %s %s\r\n", __DATE__, __TIME__);

    // Check if FXLS8974CF connected
    fxls8974cf_read(FXLS8974CF_WHO_AM_I, &reg, 1);
    if(reg != 0x86)
    {
        // Error expected 86h for FXLS8974CF
        
        printf("ERROR: WHO_AM_I expected 0x86\r\n");
        while(1)
        {}
    }
    
    printf("FXLS8974CF connected with I2C\r\n");
    
    // Wait for system boot complete event flag
    do
    {
        fxls8974cf_read(FXLS8974CF_INT_STATUS, &reg, 1);
    }
    while((reg & 0x01) == 0);

    // ACTIVE disable
    fxls8974cf_read(FXLS8974CF_SENS_CONFIG1, &reg, 1);
    reg &= ~0x01;
    fxls8974cf_write(FXLS8974CF_SENS_CONFIG1, &reg, 1);
    
    // The ODR may only be changed in Standby mode, with
    // SENS_CONFIG1[ACTIVE] = 0 and SENS_CONFIG4[INT2_FUNC] = 0.
    // ODR: 100 Hz
    reg = 0x55;
    fxls8974cf_write(FXLS8974CF_SENS_CONFIG3, &reg, 1);
    
    // All interrutps routed to INT0
    reg = 0x00;
    fxls8974cf_write(FXLS8974CF_SENS_PIN_SEL, &reg, 1);    

    // Data-ready interrupt output enable, rest disable
    reg = 0x80;
    fxls8974cf_write(FXLS8974CF_SENS_INT_EN, &reg, 1);    
    
    // ACTIVE enable
    fxls8974cf_read(FXLS8974CF_SENS_CONFIG1, &reg, 1);
    reg |= 0x01;
    fxls8974cf_write(FXLS8974CF_SENS_CONFIG1, &reg, 1);
    
    while(1)
    {
        // INT0 interrupt example
        
        // A new set of XYZ acceleration and temperature data available?
        if(fxls8974cf_int0 == true)
        {
            uint32_t ms1 = ms;
            
            // reset the flag
            fxls8974cf_int0 = false;
            
            struct
            {
                int16_t x;
                int16_t y;
                int16_t z;
            }out;
            
            fxls8974cf_read(0x04, (uint8_t *)&out, 6);
            
            // Full-scale measurement range (FSR) selection
            // 00b: ±2 g; 0.98 mg/LSB (1024 LSB/g) nominal sensitivity
            
            x_out_mg = (float)out.x * 0.98f;
            y_out_mg = (float)out.y * 0.98f;
            z_out_mg = (float)out.z * 0.98f;            
                       
            uint32_t ms2 = ms;

            // Printing three double precision floating point numbers takes
            // approximately 25ms (in both debug and release target)
            printf("%d,%d,%.3f,%.3f,%.3f\r\n", 
                ms1,
                ms2,
                (double)x_out_mg, 
                (double)y_out_mg, 
                (double)z_out_mg);   
        }
    
//        // Polling the INT_STATUS register
//    
//        fxls8974cf_read(FXLS8974CF_INT_STATUS, &reg, 1);
//        
//        // A new set of XYZ acceleration and temperature data available?
//        if((reg & 0x80) != 0)
//        {
//            uint32_t ms1 = ms;
//            
//            struct
//            {
//                int16_t x;
//                int16_t y;
//                int16_t z;
//            }out;
//            
//            fxls8974cf_read(0x04, (uint8_t *)&out, 6);
//            
//            // Full-scale measurement range (FSR) selection
//            // 00b: ±2 g; 0.98 mg/LSB (1024 LSB/g) nominal sensitivity

//            x_out_mg = (float)out.x * 0.98f;
//            y_out_mg = (float)out.y * 0.98f;
//            z_out_mg = (float)out.z * 0.98f;  
//            
//            uint32_t ms2 = ms;

//            // Printing three double precision floating point numbers takes
//            // approximately 25ms (in both debug and release target)
//            printf("%d,%d,%.3f,%.3f,%.3f\r\n", 
//                ms1,
//                ms2,
//                (double)x_out_mg, 
//                (double)y_out_mg, 
//                (double)z_out_mg);                        
//        }
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void SysTick_Handler(void)
{
    ms++;    
}
