/*! ***************************************************************************
 *
 * \brief     Circular queue
 * \file      queue.h
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
#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stdint.h>

#define Q_SIZE (256)

/*!
 * \brief Type definition of a queue
 */
typedef struct
{
    uint8_t data[Q_SIZE];
    uint32_t head; ///< Index of the oldest data element 
    uint32_t tail; ///< Index of the next free space
    uint32_t size; ///< Number of elements in the queue
    
}queue_t;


void q_init(queue_t *q);
uint32_t q_size(const queue_t *q);
bool q_empty(const queue_t *q);
bool q_full(const queue_t *q);
bool q_enqueue(queue_t *q, const uint8_t d);
bool q_dequeue(queue_t *q, uint8_t *d);

#endif // QUEUE_H
