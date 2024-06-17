/*! ***************************************************************************
 *
 * \brief     Finate statemachine
 * \file      fsm.h
 * \author    Hugo Arends
 * \date      June 2021
 *
 * \copyright 2021 HAN University of Applied Sciences. All Rights Reserved.
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
#ifndef FSM_H_
#define FSM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_STATES           (32)
#define MAX_TRANSITIONS      (128)
#define MAX_EVENTS_IN_BUFFER (32) // 2,4,8,16,32,64,128 or 256

typedef enum
{
    S_START = 0, // S_START must be 0!
    S_INIT,

    S_MAIN_1,
    S_MAIN_2,
    S_MAIN_3,

    S_RGBLED_1,
    S_RGBLED_2,
    S_RGBLED_3,
    
    S_NEO_R1,
    S_NEO_R2,
    S_NEO_R3,
    S_NEO_G1,
    S_NEO_G2,
    S_NEO_G3,
    S_NEO_B1,
    S_NEO_B2,
    S_NEO_B3,
    
    S_SR_1,
    S_SR_2,
    S_SR_3,
    S_SR_4,
    S_SR_5,
    S_SR_6,
    S_SR_7,
    S_SR_8,
        
}state_t;

typedef enum
{
    E_NO = 0,
    E_ANY,
    E_CONTINUE,
    E_UP,
    E_DOWN,    
    E_SELECT,
    E_BACK,
    E_TIMER,
    
}event_t;

typedef struct 
{
    void (*onEntry)(void);
    void (*onExit)(void);
}state_funcs_t;

typedef struct
{
    state_t from;
    event_t event;
    state_t to;
    
}transition_t;

// Function prototypes
state_t FSM_EventHandler(const state_t state, const event_t event);
void    FSM_FlushEnexpectedEvents(const bool flush);
void    FSM_AddState(const state_t state, const state_funcs_t *funcs);
void    FSM_AddTransition(const transition_t *transition);
void    FSM_AddEvent(const event_t event);
event_t FSM_GetEvent(void);
event_t FSM_WaitForEvent(void);
event_t FSM_PeekForEvent(void);
bool    FSM_NoEvents(void);
uint8_t FSM_NofEvents(void);

#endif // FSM_H_
