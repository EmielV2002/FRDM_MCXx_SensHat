/*! ***************************************************************************
 *
 * \brief     RGB led
 * \file      rgb_led.c
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
#include "rgb_led.h"
#include "helper_functions/SysTick.h"
#include "switches/switches.h"
// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
//static volatile uint32_t ms = 0;
volatile uint32_t ms = 0;
static volatile uint32_t previous_ms = 0;
static const uint32_t interval_ms = 500;

// Lookup table for the RGB LEDs pins. All pins are on P2.
static const uint8_t pin_lut[3] = 
{
    1, // P2_0 - LED_R
    2, // P2_2 - LED_G
    0, // P2_1 - LED_B
};

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
// static volatile uint32_t systick_flag = 0;
volatile uint32_t systick_flag = 0;

void rgb_led_init(void)
{
    // Initialize pins
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO2(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_CC1_GPIO2(1);

    PORT2->PCR[pin_lut[0]] = PORT_PCR_LK(1);
    PORT2->PCR[pin_lut[1]] = PORT_PCR_LK(1);
    PORT2->PCR[pin_lut[2]] = PORT_PCR_LK(1);

    GPIO2->PDOR |= (1<<pin_lut[0]) | (1<<pin_lut[1]) | (1<<pin_lut[2]);
    GPIO2->PDDR |= (1<<pin_lut[0]) | (1<<pin_lut[1]) | (1<<pin_lut[2]);
}

void rgb_led_on(const rgb_led_t led)
{
    GPIO2->PCOR = (1<<pin_lut[led]);
}

void rgb_led_off(const rgb_led_t led)
{
    GPIO2->PSOR = (1<<pin_lut[led]);
}

void rgb_led_toggle(const rgb_led_t led)
{
    GPIO2->PTOR = (1<<pin_lut[led]);
}

int rgb_led_blink_fixed(const rgb_led_t led)
{
    
    // Initialize SysTick to generate an interrupt every 100ms
    SysTick_Config(4800000);
    
    // Enable interrupts
    __enable_irq();

    while(1)
    {
        // Wait for interrupt
        __WFI();
        
        // Check the SysTick flag
        if(systick_flag == 1)
        {
            // Reset the flag
            systick_flag = 0;

            // Toggle the pin
            rgb_led_toggle(pin_lut[led]);
        }
    }
  }


int rgb_led_blink_all(void)
{
      // Initialize SysTick to generate an interrupt every 1ms
      SysTick_Config(48000);
      
      // Enable interrupts
      __enable_irq();

      rgb_led_t rgb_led = RGB_LED_RED;

      while(1)
      {
          // Wait for interrupt
          __WFI();
          
          uint32_t current_ms = ms;
          
          if((current_ms - previous_ms) >= interval_ms)
          {
              previous_ms = current_ms;

              // RGB led off
              rgb_led_off(rgb_led);
              
              // Next RGB led
              rgb_led = (rgb_led + 1) % 3;
              
              // RGB led on
              rgb_led_on(rgb_led);            
          }
      }    
 }

int rgb_led_switch(const rgb_led_t led1, const rgb_led_t led2)
{
   while(1)
   {
      // Wait for interrupt
      __WFI();
    
      if(sw_clicked(SW2))
      {
         rgb_led_toggle(led1);
      }

      if(sw_clicked(SW3))
      {
         rgb_led_toggle(led2);
      }
   }    
}
 
// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
//void SysTick_Handler(void)
//{
//    systick_flag = 1;
//}