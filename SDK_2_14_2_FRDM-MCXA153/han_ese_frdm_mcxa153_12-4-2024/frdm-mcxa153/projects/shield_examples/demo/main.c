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
  #include "encoder/encoder.h"
  #include "helper_functions/lpi2c0_controller_dma.h"
  #include "oled/ssd1306.h"
  #include "rgb_led/rgb_led.h"
  #include "shiftregister/shiftregister.h"
  #include "neopixel/neopixel_polling.h"
  #include "switches/switches.h"
  #include "eeprom/eeprom.h"
  #include "accelerometer/accelerometer.h"
  #include <stdbool.h>
  #include "helper_functions/adc0_interrupt.h"
  #include "potentiometer/potentiometer.h"

  int next = 1;
    
  int main(void)
  {
    ssd1306_init();
    sw_init();
    // Set initial message
    ssd1306_setorientation(1);
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_clearscreen();
    ssd1306_update();                
            
    // Delay
    for(volatile int i=0; i<1000000; i++)
    {}
      
    ssd1306_drawbitmap(nxp_logo);
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(53, 42, "MCX");
    ssd1306_putstring(38, 53, "A Series");
    ssd1306_update();       

    // Delay
    for(volatile int i=0; i<10000000; i++)
    {}

    ssd1306_drawbitmap(bitmap1);
    ssd1306_setfont(Monospaced_bold_24);
    ssd1306_putstring(54, 16, "HAN_");
    ssd1306_update();       

    // Delay
    for(volatile int i=0; i<10000000; i++)
    {}
    ssd1306_clearscreen();
    ssd1306_update(); 
    
    while(1)
    {
      if(sw_clicked(SW2))
    {
       next++;  
    }

    if(sw_clicked(SW3))
    {
       next--;
    }
    
      //int select = 3;
      switch (next)
      {
        case 1:
          encoder_init();
          encoder_main();
        break;
        case 2:
          ssd1306_init();
          ssd1306_main();
        break;
        case 3:
          rgb_led_init();
          rgb_led_blink_fixed(RGB_LED_BLUE);
          //rgb_led_blink_all();
          //rgb_led_switch(RGB_LED_GREEN, RGB_LED_RED);
        break;
        case 4:
          shiftregister_init();
          shiftregister_main();
        break;
        case 5:
          neopixel_init();
          neopixel_main();
        break;
        case 6:
          eeprom_init();
          eeprom_main();
        break;
        case 7:
          accelerometer_init();
          accelerometer_main();
        break;
        case 8:
          adc0_init();
          uint16_t adc_val = map(potentiometer_main(), 0 , 4096, 0, 100);
        default:
          //select = 1;
          next = 1;
          //do stuff
      }
    }
  }
