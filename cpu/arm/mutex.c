/*
 *  ____________________________________________________________________
 * 
 *  Copyright (c) 2002, Andrew N. Sloss, Chris Wright and Dominic Symes
 *  All rights reserved.
 *  ____________________________________________________________________
 * 
 *  NON-COMMERCIAL USE License
 *  
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met: 
 *  
 *  1. For NON-COMMERCIAL USE only.
 * 
 *  2. Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer. 
 * 
 *  3. Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials provided 
 *     with the distribution. 
 * 
 *  4. All advertising materials mentioning features or use of this 
 *     software must display the following acknowledgement:
 * 
 *     This product includes software developed by Andrew N. Sloss,
 *     Chris Wright and Dominic Symes. 
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY 
 *   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *   PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE 
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
 *   OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 *   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 *   OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
 *   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
 *   OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 *   OF SUCH DAMAGE. 
 * 
 *  If you have questions about this license or would like a different
 *  license please email :
 * 
 * 	andrew@sloss.net
 * 
 * 
 */
 
/*****************************************************************************
 * Simple Little Operating System - SLOS
 *****************************************************************************/

/*****************************************************************************
 *
 * Module      : mutex
 * Description : Simple Mutual Exclusion module that allows 
 *               critical sections of code to have access to 
 *               shared memory.
 * OS          : SLOS
 * Platform    : generic
 * History     :
 *		
 * 10th November 2001 Andrew N. Sloss
 * - add mutex to SLOS
 *
 *****************************************************************************/

/*****************************************************************************
 * IMPORT
 *****************************************************************************/

/* none... */

/*****************************************************************************
 * MACROS
 *****************************************************************************/

/* none... */

/*****************************************************************************
 * STATICS
 *****************************************************************************/

unsigned volatile int semaphore	= 2; /* this is a start value */

/*****************************************************************************
 * ROUTINES
 *****************************************************************************/

/* -- mutex_gatelock ----------------------------------------------------------
 *
 * Description : Locks the semaphore... 
 * 
 * Parameters  : none...
 * Return      : none...
 * Notes       :
 *
 */
	
void mutex_gatelock (void) 
{
  register unsigned int semaAddr = (unsigned int) &semaphore;

  asm volatile
  (
    "spin:\n"
    "  MOV   r1, %0\n"
    "  MOV   r2, #0x1\n"
    "  SWP   r3, r2, [r1]\n"
    "  CMP   r3, #0x1\n"
    "  BEQ   spin\n"
    : /* no output */
    : "r" (semaAddr)
   );
}

/* -- mutex_gateunlock --------------------------------------------------------
 *
 * Description : Unlocks the semaphore ...
 * 
 * Parameters  : none...
 * Return      : none...
 * Notes       :
 *
 */

void mutex_gateunlock (void) 
{
  register unsigned int semaAddr = (unsigned int) &semaphore;

  asm volatile 
  (
    "  mov  r1, %0\n"
    "  mov  r2, #0\n"
    "  swp  r0,r2,[r1]"
    : /* no output */
    : "r" (semaAddr)
  );
}

