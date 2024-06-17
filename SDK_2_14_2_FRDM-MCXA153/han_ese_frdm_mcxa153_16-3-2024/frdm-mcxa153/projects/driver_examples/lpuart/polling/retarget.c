/*! ***************************************************************************
 *
 * \brief     Functions for retargeting stdio
 * \file      retarget.c
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
#include <stdio.h>
#include "lpuart0_polling.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

#ifdef MDKARM

// Functions for redirecting standard output to LPUART0 for MDK-ARM.

int stdout_putchar(int ch)
{
    lpuart0_putchar(ch);
    return ch;
}

int stdin_getchar(void)
{
    return lpuart0_getchar();
}

#endif

#ifdef __NEWLIB__

// Functions for redirecting standard output to LPUART0 for GCC ARM

int _write(int fd, const void *buf, size_t count)
{
	(void)fd;

	for(size_t i=0; i<count; i++)
	{
		lpuart0_putchar(((char *)buf)[i]);
	}

	return count;
}

int _read(int fd, const void *buf, size_t count)
{
	(void)fd;

	for(size_t i=0; i<count; i++)
	{
		((char *)buf)[i] = lpuart0_getchar();
	}

	return count;
}

#endif
