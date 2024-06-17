/*! ***************************************************************************
 *
 * \brief     Finate statemachine
 * \file      fsm.c
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
#include "fsm.h"

#include <string.h>

#define MAX_EVENTS_IN_BUFFER_MASK (MAX_EVENTS_IN_BUFFER - 1)
#if (MAX_EVENTS_IN_BUFFER & MAX_EVENTS_IN_BUFFER_MASK)
#error events size is not a power of two
#endif

// Global variables
static state_funcs_t state_funcs[MAX_STATES] = {0};

static transition_t transitions[MAX_TRANSITIONS];
static volatile uint8_t transition_cnt = 0;

static event_t events[MAX_EVENTS_IN_BUFFER];
static volatile uint8_t head = 0;
static volatile uint8_t tail = 0;

static volatile bool flush_event = 0;

state_t FSM_EventHandler(const state_t state, const event_t event)
{
	state_t nextState = state;
	
	// Check all transitions in the transition matrix
	for(uint8_t i=0; i <= transition_cnt; ++i)
	{
		// Is the state equal to the from state?
		if(transitions[i].from == state)
		{
			// And is the event equal to the event?
			if(transitions[i].event == event)
			{
				// Execute the from state onExit() function
				if(state_funcs[transitions[i].from].onExit != NULL)
				{
					state_funcs[transitions[i].from].onExit();
				}

				// Set the next state
				nextState = transitions[i].to;
				
				// Execute the to state onEntry() function
				if(state_funcs[transitions[i].to].onEntry != NULL)
				{
					state_funcs[transitions[i].to].onEntry();
				}
				
				return nextState;
			}
		}
	}
	
	// Still here, so the event is unexpected in the current state. Remain in
    // current state. Optionally, return the event back in the event buffer.
    if(!flush_event)
	{
        FSM_AddEvent(event);
    }
	
	return nextState;
}

void FSM_FlushEnexpectedEvents(const bool flush)
{
    flush_event = flush;
}

void FSM_AddState(const state_t state, const state_funcs_t *funcs)
{
	if(state >= MAX_STATES)
	{
		// Error, state is out of bounds
		return;
	}	
	
	// Copy the state and save locally
	memcpy(&state_funcs[state], funcs, sizeof(state_funcs_t));
}

void FSM_AddTransition(const transition_t *transition)
{	
	if(transition_cnt == MAX_TRANSITIONS)
	{
		// Error, too many transitions
		return;
	}
	
	// Copy the transition and save locally
	memcpy(&transitions[transition_cnt], transition, sizeof(transition_t));
	
	++transition_cnt;
}

event_t FSM_PeekForEvent(void)
{
	return events[head];
}

bool FSM_NoEvents(void)
{
	return (head == tail);	
}

event_t FSM_WaitForEvent(void)
{
	while(FSM_NoEvents())
	{}
		
	return FSM_GetEvent();
}

uint8_t FSM_NofEvents(void)
{
	if(head == tail)
		return 0;
	else if(head > tail)
		return head - tail;
	else
		return MAX_EVENTS_IN_BUFFER - tail + head;
}

void FSM_AddEvent(const event_t event)
{
	uint8_t tmpHead;
	
	// Calculate index
	tmpHead = (head + 1) & MAX_EVENTS_IN_BUFFER_MASK;
	
	// Check if queue is full
	if(tmpHead == tail)
	{
		// Queue is full, flush the event
		return;
	}
	
	// Store the event in the queue
	events[tmpHead] = event;
	
	// Save the new index
	head = tmpHead;
}

event_t FSM_GetEvent(void)
{
	event_t event = E_NO;
	uint8_t tmpTail;
	
	if(!FSM_NoEvents())
	{
		// Calculate index
		tmpTail = (tail + 1) & MAX_EVENTS_IN_BUFFER_MASK;
		
		// Get the event from the queue
		event = events[tmpTail];
		
		// Store the new index
		tail = tmpTail;
	}

	return event;
}
