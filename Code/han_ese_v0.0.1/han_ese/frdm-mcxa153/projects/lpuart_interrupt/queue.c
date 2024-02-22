/*! ***************************************************************************
 *
 * \brief     Circular queue
 * \file      queue.c
 * \author    Hugo Arends
 * \date      February 2024
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
#include "queue.h"

/*!
 * \brief Initialize a queue
 *
 * Resets all the local variables of the queue. All data in the queue is set to
 * 0, which is convenient for debugging purpose.
 *
 * \param[in]  q  Pointer to the queue
 */
void q_init(queue_t *q)
{
    // Clear the data for debugging purpose
    for(uint32_t i=0; i<Q_SIZE; i++) 
    {    
        q->data[i] = 0;
    }
    
    q->head = 0;
    q->tail = 0;
    q->size = 0;
}

/*!
 * \brief Check if a queue is empty
 *
 * The function can be used to check if a queue is empty.
 * - true : queue is empty
 * - false: queue is not empty
 *
 * \param[in]  q  Pointer to the queue
 *
 * \return Wether the queue is empty or not
 */
bool q_empty(const queue_t *q)
{
    return q->size == 0;
}

/*!
 * \brief Check if a queue is full
 *
 * The function can be used to check if a queue is full.
 * - true : queue is full
 * - false: queue is not full
 *
 * \param[in]  q  Pointer to the queue
 *
 * \return Wether the queue is empty or not
 */
bool q_full(const queue_t *q)
{
    return q->size == Q_SIZE;
}

/*!
 * \brief Check the number of elements in the queue
 *
 * The function can be used to check the number of elements that are currently
 * in the queue.
 *
 * \param[in]  q  Pointer to the queue
 *
 * \return Number of elements in the queue
 */
uint32_t q_size(const queue_t *q)
{
	return q->size;
}

/*!
 * \brief Add an element to the queue
 *
 * The function can be used to add an element to the queue. If the queue is
 * full before the element could be added, the function aborts and returns 
 * false.
 *
 * \param[in]  q  Pointer to the queue
 * \param[in]  d  Data element to add
 *
 * \return Wether the data element was successfully added to the queue
 */
bool q_enqueue(queue_t *q, const uint8_t d) 
{
    // Is the queue not full?
    if(!q_full(q)) 
    {
        // Critical section
        uint32_t m = __get_PRIMASK();
        __disable_irq();

        // Add element to the queue
        q->data[q->tail++] = d;
        q->tail %= Q_SIZE;
        q->size++;
        
        __set_PRIMASK(m);
        
        return true;
    } 
    else 
    {
        // Queue is full
        return false;
    }
}

/*!
 * \brief Get an element from the queue
 *
 * The function can be used to get an element from the queue. If the queue is
 * empty, the function aborts and returns false.
 *
 * \param[in]  q  Pointer to the queue
 * \param[in]  d  Pointer to where the data element can be stored
 *
 * \return Wether a data element was successfully read from the queue
 */
bool q_dequeue(queue_t *q, uint8_t *d)
{
    // Is the queue not empty?
    if(!q_empty(q)) 
    {
        // Critical section
        uint32_t m = __get_PRIMASK();
        __disable_irq();

        // Get and store the data element
        *d = q->data[q->head];        
        q->data[q->head++] = 0;
        q->head %= Q_SIZE;
        q->size--;
        
        __set_PRIMASK(m);
        
        return true;
    }
    else
    {
        // Queue is empty
        return false;
    }
}
