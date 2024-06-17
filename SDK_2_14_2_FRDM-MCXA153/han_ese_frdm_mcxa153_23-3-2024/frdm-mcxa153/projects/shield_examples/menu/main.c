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

#include "encoder.h"
#include "fsm.h"
#include "lpuart0_interrupt.h"
#include "neopixel_flexpwm.h"
#include "rgb_led.h"
#include "ssd1306.h"
#include "shiftregister.h"
#include "switches.h"

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

//void S_START_onExit(void);
void S_INIT_onEntry(void);
void S_INIT_onExit(void);

void S_MAIN_1_onEntry(void);
void S_MAIN_1_onExit(void);
void S_MAIN_2_onEntry(void);
void S_MAIN_2_onExit(void);
void S_MAIN_3_onEntry(void);
void S_MAIN_3_onExit(void);

void S_RGBLED_1_onEntry(void);
void S_RGBLED_1_onExit(void);
void S_RGBLED_2_onEntry(void);
void S_RGBLED_2_onExit(void);
void S_RGBLED_3_onEntry(void);
void S_RGBLED_3_onExit(void);

void S_NEO_R1_onEntry(void);
void S_NEO_R2_onEntry(void);
void S_NEO_R3_onEntry(void);
void S_NEO_G1_onEntry(void);
void S_NEO_G2_onEntry(void);
void S_NEO_G3_onEntry(void);
void S_NEO_B1_onEntry(void);
void S_NEO_B2_onEntry(void);
void S_NEO_B3_onEntry(void);
void S_NEO_onExit(void);

void S_SR_1_onEntry(void);
void S_SR_2_onEntry(void);
void S_SR_3_onEntry(void);
void S_SR_4_onEntry(void);
void S_SR_5_onEntry(void);
void S_SR_6_onEntry(void);
void S_SR_7_onEntry(void);
void S_SR_8_onEntry(void);
void S_SR_onExit(void);

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static volatile uint32_t ms = 0;
static volatile uint32_t previous_ms = 0;
static const uint32_t interval_ms = 500;

static const char * eventEnumToText[] =
{
    "E_NO",
    "E_ANY",
    "E_CONTINUE",
    "E_UP",
    "E_DOWN",
    "E_SELECT",
    "E_BACK",
    "E_TIMER",
};

static const char * stateEnumToText[] =
{
	"S_START",
    "S_INIT",

	"S_MAIN_1",
    "S_MAIN_2",
    "S_MAIN_3",

	"S_RGBLED_1",
    "S_RGBLED_2",
    "S_RGBLED_3",

    "S_NEO_R1",
    "S_NEO_R2",
    "S_NEO_R3",
    "S_NEO_G1",
    "S_NEO_G2",
    "S_NEO_G3",
    "S_NEO_B1",
    "S_NEO_B2",
    "S_NEO_B3",

	"S_SR_1",
	"S_SR_2",
	"S_SR_3",
	"S_SR_4",
	"S_SR_5",
	"S_SR_6",
	"S_SR_7",
	"S_SR_8",
};

static state_t state;
static event_t event;

// -----------------------------------------------------------------------------
// Main application
// -----------------------------------------------------------------------------

int main(void)
{
    // 96 MHz FIRC clock selected
    SCG0->FIRCCFG = SCG_FIRCCFG_FREQ_SEL(0b101);

    // Initialize SysTick to generate an interrupt every 1ms
    lpuart0_init(9600);
    SysTick_Config(96000);
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    // VT100 code to clear entire screen
	char cmd1[] = {0x1B, '[', '2', 'J', '\0'}; // Clear screen
	char cmd2[] = {0x1B, '[', 'f', '\0'}; // Cursor home
    printf("%s%s", cmd1, cmd2);

    printf("Menu example implemented with a finite state machine");
    printf(" - %s\r\n", TARGETSTR);
    printf("Build %s %s\r\n", __DATE__, __TIME__);

    // Build state machine

    //           State      |                   onEntry()           | onExit()
    FSM_AddState(S_START    , &(state_funcs_t){ NULL                , NULL });
    FSM_AddState(S_INIT     , &(state_funcs_t){ S_INIT_onEntry      , NULL });
    //                      |                                       |
    FSM_AddState(S_MAIN_1   , &(state_funcs_t){ S_MAIN_1_onEntry    , S_MAIN_1_onExit });
    FSM_AddState(S_MAIN_2   , &(state_funcs_t){ S_MAIN_2_onEntry    , S_MAIN_2_onExit });
    FSM_AddState(S_MAIN_3   , &(state_funcs_t){ S_MAIN_3_onEntry    , S_MAIN_3_onExit });
    //                      |                                       |
    FSM_AddState(S_RGBLED_1 , &(state_funcs_t){ S_RGBLED_1_onEntry  , S_RGBLED_1_onExit });
    FSM_AddState(S_RGBLED_2 , &(state_funcs_t){ S_RGBLED_2_onEntry  , S_RGBLED_2_onExit });
    FSM_AddState(S_RGBLED_3 , &(state_funcs_t){ S_RGBLED_3_onEntry  , S_RGBLED_3_onExit });
    //                      |                                       |
    FSM_AddState(S_NEO_R1   , &(state_funcs_t){ S_NEO_R1_onEntry    , S_NEO_onExit });
    FSM_AddState(S_NEO_R2   , &(state_funcs_t){ S_NEO_R2_onEntry    , S_NEO_onExit });
    FSM_AddState(S_NEO_R3   , &(state_funcs_t){ S_NEO_R3_onEntry    , S_NEO_onExit });
    FSM_AddState(S_NEO_G1   , &(state_funcs_t){ S_NEO_G1_onEntry    , S_NEO_onExit });
    FSM_AddState(S_NEO_G2   , &(state_funcs_t){ S_NEO_G2_onEntry    , S_NEO_onExit });
    FSM_AddState(S_NEO_G3   , &(state_funcs_t){ S_NEO_G3_onEntry    , S_NEO_onExit });
    FSM_AddState(S_NEO_B1   , &(state_funcs_t){ S_NEO_B1_onEntry    , S_NEO_onExit });
    FSM_AddState(S_NEO_B2   , &(state_funcs_t){ S_NEO_B2_onEntry    , S_NEO_onExit });
    FSM_AddState(S_NEO_B3   , &(state_funcs_t){ S_NEO_B3_onEntry    , S_NEO_onExit });
    //                      |                                       |
    FSM_AddState(S_SR_1     , &(state_funcs_t){ S_SR_1_onEntry      , S_SR_onExit });
    FSM_AddState(S_SR_2     , &(state_funcs_t){ S_SR_2_onEntry      , S_SR_onExit });
    FSM_AddState(S_SR_3     , &(state_funcs_t){ S_SR_3_onEntry      , S_SR_onExit });
    FSM_AddState(S_SR_4     , &(state_funcs_t){ S_SR_4_onEntry      , S_SR_onExit });
    FSM_AddState(S_SR_5     , &(state_funcs_t){ S_SR_5_onEntry      , S_SR_onExit });
    FSM_AddState(S_SR_6     , &(state_funcs_t){ S_SR_6_onEntry      , S_SR_onExit });
    FSM_AddState(S_SR_7     , &(state_funcs_t){ S_SR_7_onEntry      , S_SR_onExit });
    FSM_AddState(S_SR_8     , &(state_funcs_t){ S_SR_8_onEntry      , S_SR_onExit });


    //                                 From           Event       To
    FSM_AddTransition(&(transition_t){ S_START      , E_CONTINUE, S_INIT });
    FSM_AddTransition(&(transition_t){ S_INIT       , E_CONTINUE, S_MAIN_1 });
    FSM_AddTransition(&(transition_t){ S_MAIN_1     , E_UP      , S_MAIN_2 });
    FSM_AddTransition(&(transition_t){ S_MAIN_2     , E_UP      , S_MAIN_3 });
    FSM_AddTransition(&(transition_t){ S_MAIN_3     , E_UP      , S_MAIN_1 });
    FSM_AddTransition(&(transition_t){ S_MAIN_1     , E_DOWN    , S_MAIN_3 });
    FSM_AddTransition(&(transition_t){ S_MAIN_2     , E_DOWN    , S_MAIN_1 });
    FSM_AddTransition(&(transition_t){ S_MAIN_3     , E_DOWN    , S_MAIN_2 });

    FSM_AddTransition(&(transition_t){ S_MAIN_1     , E_SELECT  , S_RGBLED_1 });

    FSM_AddTransition(&(transition_t){ S_RGBLED_1   , E_UP      , S_RGBLED_2 });
    FSM_AddTransition(&(transition_t){ S_RGBLED_2   , E_UP      , S_RGBLED_3 });
    FSM_AddTransition(&(transition_t){ S_RGBLED_3   , E_UP      , S_RGBLED_1 });
    FSM_AddTransition(&(transition_t){ S_RGBLED_1   , E_DOWN    , S_RGBLED_3 });
    FSM_AddTransition(&(transition_t){ S_RGBLED_2   , E_DOWN    , S_RGBLED_1 });
    FSM_AddTransition(&(transition_t){ S_RGBLED_3   , E_DOWN    , S_RGBLED_2 });
    FSM_AddTransition(&(transition_t){ S_RGBLED_1   , E_BACK    , S_MAIN_1 });
    FSM_AddTransition(&(transition_t){ S_RGBLED_2   , E_BACK    , S_MAIN_1 });
    FSM_AddTransition(&(transition_t){ S_RGBLED_3   , E_BACK    , S_MAIN_1 });

    FSM_AddTransition(&(transition_t){ S_MAIN_3     , E_SELECT  , S_SR_1 });

    FSM_AddTransition(&(transition_t){ S_SR_1       , E_UP      , S_SR_2 });
    FSM_AddTransition(&(transition_t){ S_SR_2       , E_UP      , S_SR_3 });
    FSM_AddTransition(&(transition_t){ S_SR_3       , E_UP      , S_SR_4 });
    FSM_AddTransition(&(transition_t){ S_SR_4       , E_UP      , S_SR_5 });
    FSM_AddTransition(&(transition_t){ S_SR_5       , E_UP      , S_SR_6 });
    FSM_AddTransition(&(transition_t){ S_SR_6       , E_UP      , S_SR_7 });
    FSM_AddTransition(&(transition_t){ S_SR_7       , E_UP      , S_SR_8 });
    FSM_AddTransition(&(transition_t){ S_SR_8       , E_UP      , S_SR_1 });
    FSM_AddTransition(&(transition_t){ S_SR_1       , E_DOWN    , S_SR_8 });
    FSM_AddTransition(&(transition_t){ S_SR_2       , E_DOWN    , S_SR_1 });
    FSM_AddTransition(&(transition_t){ S_SR_3       , E_DOWN    , S_SR_2 });
    FSM_AddTransition(&(transition_t){ S_SR_4       , E_DOWN    , S_SR_3 });
    FSM_AddTransition(&(transition_t){ S_SR_5       , E_DOWN    , S_SR_4 });
    FSM_AddTransition(&(transition_t){ S_SR_6       , E_DOWN    , S_SR_5 });
    FSM_AddTransition(&(transition_t){ S_SR_7       , E_DOWN    , S_SR_6 });
    FSM_AddTransition(&(transition_t){ S_SR_8       , E_DOWN    , S_SR_7 });
    FSM_AddTransition(&(transition_t){ S_SR_1       , E_BACK    , S_MAIN_3 });
    FSM_AddTransition(&(transition_t){ S_SR_2       , E_BACK    , S_MAIN_3 });
    FSM_AddTransition(&(transition_t){ S_SR_3       , E_BACK    , S_MAIN_3 });
    FSM_AddTransition(&(transition_t){ S_SR_4       , E_BACK    , S_MAIN_3 });
    FSM_AddTransition(&(transition_t){ S_SR_5       , E_BACK    , S_MAIN_3 });
    FSM_AddTransition(&(transition_t){ S_SR_6       , E_BACK    , S_MAIN_3 });
    FSM_AddTransition(&(transition_t){ S_SR_7       , E_BACK    , S_MAIN_3 });
    FSM_AddTransition(&(transition_t){ S_SR_8       , E_BACK    , S_MAIN_3 });

    FSM_AddTransition(&(transition_t){ S_MAIN_2     , E_SELECT  , S_NEO_R1 });

    FSM_AddTransition(&(transition_t){ S_NEO_R1     , E_TIMER   , S_NEO_R2 });
    FSM_AddTransition(&(transition_t){ S_NEO_R2     , E_TIMER   , S_NEO_R3 });
    FSM_AddTransition(&(transition_t){ S_NEO_R3     , E_TIMER   , S_NEO_R1 });
    FSM_AddTransition(&(transition_t){ S_NEO_G1     , E_TIMER   , S_NEO_G2 });
    FSM_AddTransition(&(transition_t){ S_NEO_G2     , E_TIMER   , S_NEO_G3 });
    FSM_AddTransition(&(transition_t){ S_NEO_G3     , E_TIMER   , S_NEO_G1 });
    FSM_AddTransition(&(transition_t){ S_NEO_B1     , E_TIMER   , S_NEO_B2 });
    FSM_AddTransition(&(transition_t){ S_NEO_B2     , E_TIMER   , S_NEO_B3 });
    FSM_AddTransition(&(transition_t){ S_NEO_B3     , E_TIMER   , S_NEO_B1 });
    FSM_AddTransition(&(transition_t){ S_NEO_R1     , E_UP      , S_NEO_G1 });
    FSM_AddTransition(&(transition_t){ S_NEO_R2     , E_UP      , S_NEO_G2 });
    FSM_AddTransition(&(transition_t){ S_NEO_R3     , E_UP      , S_NEO_G3 });
    FSM_AddTransition(&(transition_t){ S_NEO_G1     , E_UP      , S_NEO_B1 });
    FSM_AddTransition(&(transition_t){ S_NEO_G2     , E_UP      , S_NEO_B2 });
    FSM_AddTransition(&(transition_t){ S_NEO_G3     , E_UP      , S_NEO_B3 });
    FSM_AddTransition(&(transition_t){ S_NEO_B1     , E_UP      , S_NEO_R1 });
    FSM_AddTransition(&(transition_t){ S_NEO_B2     , E_UP      , S_NEO_R2 });
    FSM_AddTransition(&(transition_t){ S_NEO_B3     , E_UP      , S_NEO_R3 });
    FSM_AddTransition(&(transition_t){ S_NEO_R1     , E_BACK    , S_MAIN_2 });
    FSM_AddTransition(&(transition_t){ S_NEO_R2     , E_BACK    , S_MAIN_2 });
    FSM_AddTransition(&(transition_t){ S_NEO_R3     , E_BACK    , S_MAIN_2 });
    FSM_AddTransition(&(transition_t){ S_NEO_G1     , E_BACK    , S_MAIN_2 });
    FSM_AddTransition(&(transition_t){ S_NEO_G2     , E_BACK    , S_MAIN_2 });
    FSM_AddTransition(&(transition_t){ S_NEO_G3     , E_BACK    , S_MAIN_2 });
    FSM_AddTransition(&(transition_t){ S_NEO_B1     , E_BACK    , S_MAIN_2 });
    FSM_AddTransition(&(transition_t){ S_NEO_B2     , E_BACK    , S_MAIN_2 });
    FSM_AddTransition(&(transition_t){ S_NEO_B3     , E_BACK    , S_MAIN_2 });


    // Should unexpected events in a state be flushed or not?
    FSM_FlushEnexpectedEvents(true);

    // Set the finite state machine in the S_START state
    // and add a E_CONTINUE event into the event buffer
    state = S_START;
    FSM_AddEvent(E_CONTINUE);

    while(1)
    {
        if(!FSM_NoEvents())
        {
            // Get the event and handle it
            event = FSM_GetEvent();

            // For debugging
            printf("%s -> ", eventEnumToText[event]);

            state = FSM_EventHandler(state, event);

            // For debugging
            printf("%s\r\n", stateEnumToText[state]);
        }

        // ---------------------------------------------------------------------
        if(encoder_cw())
        {
            FSM_AddEvent(E_UP);
        }

        // ---------------------------------------------------------------------
        if(encoder_ccw())
        {
            FSM_AddEvent(E_DOWN);
        }

        // ---------------------------------------------------------------------
        if(sw_clicked(SW2))
        {
            FSM_AddEvent(E_SELECT);
        }

        // ---------------------------------------------------------------------
        if(sw_clicked(SW3))
        {
            FSM_AddEvent(E_BACK);
        }

        // ---------------------------------------------------------------------
        uint32_t current_ms = ms;

        if((current_ms - previous_ms) >= interval_ms)
        {
            previous_ms = current_ms;

            FSM_AddEvent(E_TIMER);
        }
    }
}

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------
void SysTick_Handler(void)
{
    ms++;
}

void S_INIT_onEntry(void)
{
    ssd1306_init();
    ssd1306_clearscreen();
    ssd1306_update();
    ssd1306_setorientation(1);

    encoder_init();
    sw_init();
    rgb_led_init();
    shiftregister_init();
    neopixel_init();

    rgb_t data[N_NEOPIXELS] = {0};
    neopixel_update(data);

    FSM_AddEvent(E_CONTINUE);
}

void S_INIT_onExit(void)
{

}

void S_MAIN_1_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Main menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "* RGB LED");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "  Neopixel");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "  Shift register");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "                SW2 >");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);
    ssd1306_update();
}

void S_MAIN_1_onExit(void)
{

}

void S_MAIN_2_onEntry(void)
{
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);
    
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Main menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "  RGB LED");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "* Neopixel");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "  Shift register");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "                SW2 >");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);

    ssd1306_update();
}

void S_MAIN_2_onExit(void)
{

}

void S_MAIN_3_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Main menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "  RGB LED");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "  Neopixel");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "* Shift register");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "                SW2 >");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);

    ssd1306_update();
}

void S_MAIN_3_onExit(void)
{

}

void S_RGBLED_1_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "RGB LED menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "* Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "  Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "  Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);

    ssd1306_update();

    rgb_led_on(RGB_LED_RED);
}

void S_RGBLED_1_onExit(void)
{
    rgb_led_off(RGB_LED_RED);
}

void S_RGBLED_2_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "RGB LED menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "  Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "* Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "  Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    rgb_led_on(RGB_LED_GREEN);
}

void S_RGBLED_2_onExit(void)
{
    rgb_led_off(RGB_LED_GREEN);
}

void S_RGBLED_3_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "RGB LED menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "  Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "  Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "* Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    rgb_led_on(RGB_LED_BLUE);
}

void S_RGBLED_3_onExit(void)
{
    rgb_led_off(RGB_LED_BLUE);
}

void S_SR_1_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Shift register menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "1");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    uint8_t data = (uint8_t)(1<<0);
    shiftregister_write(&data, 1);
}

void S_SR_2_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Shift register menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "2");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);
    ssd1306_update();

    uint8_t data = (uint8_t)(1<<1);
    shiftregister_write(&data, 1);
}

void S_SR_3_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Shift register menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "3");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    uint8_t data = (uint8_t)(1<<2);
    shiftregister_write(&data, 1);
}

void S_SR_4_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Shift register menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "4");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    uint8_t data = (uint8_t)(1<<3);
    shiftregister_write(&data, 1);
}

void S_SR_5_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Shift register menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "5");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    uint8_t data = (uint8_t)(1<<4);
    shiftregister_write(&data, 1);
}

void S_SR_6_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Shift register menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "6");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    uint8_t data = (uint8_t)(1<<5);
    shiftregister_write(&data, 1);
}

void S_SR_7_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Shift register menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "7");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    uint8_t data = (uint8_t)(1<<6);
    shiftregister_write(&data, 1);
}

void S_SR_8_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Shift register menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "8");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    uint8_t data = (uint8_t)(1<<7);
    shiftregister_write(&data, 1);
}

void S_SR_onExit(void)
{
    uint8_t data = 0;
    shiftregister_write(&data, 1);
}

void S_NEO_R1_onEntry(void)
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Neopixel menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "* Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "  Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "  Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);
    ssd1306_update();

    rgb_t data[N_NEOPIXELS] = {{10, 0, 0},{ 0, 0, 0},{ 0, 0, 0}};
    neopixel_update(data);
}

void S_NEO_R2_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Neopixel menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "* Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "  Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "  Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    rgb_t data[N_NEOPIXELS] = {{ 0, 0, 0},{10, 0, 0},{ 0, 0, 0}};
    neopixel_update(data);
}

void S_NEO_R3_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Neopixel menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "* Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "  Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "  Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    rgb_t data[N_NEOPIXELS] = {{ 0, 0, 0},{ 0, 0, 0},{10, 0, 0}};
    neopixel_update(data);
}

void S_NEO_G1_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Neopixel menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "  Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "* Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "  Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    rgb_t data[N_NEOPIXELS] = {{ 0,10, 0},{ 0, 0, 0},{ 0, 0, 0}};
    neopixel_update(data);
}

void S_NEO_G2_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Neopixel menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "  Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "* Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "  Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    rgb_t data[N_NEOPIXELS] = {{ 0, 0, 0},{ 0,10, 0},{ 0, 0, 0}};
    neopixel_update(data);

}

void S_NEO_G3_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Neopixel menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "  Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "* Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "  Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    rgb_t data[N_NEOPIXELS] = {{ 0, 0, 0},{ 0, 0, 0},{ 0,10, 0}};
    neopixel_update(data);

}

void S_NEO_B1_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Neopixel menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "  Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "  Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "* Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    rgb_t data[N_NEOPIXELS] = {{ 0, 0,10},{ 0, 0, 0},{ 0, 0, 0}};
    neopixel_update(data);
}

void S_NEO_B2_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Neopixel menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "  Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "  Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "* Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    rgb_t data[N_NEOPIXELS] = {{ 0, 0, 0},{ 0, 0,10},{ 0, 0, 0}};
    neopixel_update(data);
}

void S_NEO_B3_onEntry(void)
{
    ssd1306_clearscreen();
    ssd1306_setfont(Monospaced_plain_10);
    ssd1306_putstring(0, 0 * Monospaced_plain_10[1], "Neopixel menu");
    ssd1306_putstring(0, 1 * Monospaced_plain_10[1], "  Red");
    ssd1306_putstring(0, 2 * Monospaced_plain_10[1], "  Green");
    ssd1306_putstring(0, 3 * Monospaced_plain_10[1], "* Blue");
    ssd1306_putstring(0, 4 * Monospaced_plain_10[1], "< SW3               ");
    ssd1306_drawline(0, 1 * Monospaced_plain_10[1], 128, 1 * Monospaced_plain_10[1]);
    ssd1306_drawline(0, 4 * Monospaced_plain_10[1], 128, 4 * Monospaced_plain_10[1]);    
    ssd1306_update();

    rgb_t data[N_NEOPIXELS] = {{ 0, 0, 0},{ 0, 0, 0},{ 0, 0,10}};
    neopixel_update(data);
}

void S_NEO_onExit(void)
{
    rgb_t data[3] = {{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0}};
    neopixel_update(data);
}
