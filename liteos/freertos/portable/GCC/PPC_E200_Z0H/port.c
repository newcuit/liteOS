/*
    FreeRTOS V7.3.0 - Copyright (C) 2012 Real Time Engineers Ltd.

    FEATURES AND PORTS ARE ADDED TO FREERTOS ALL THE TIME.  PLEASE VISIT
    http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, training, latest versions, license
    and contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool.

    Real Time Engineers ltd license FreeRTOS to High Integrity Systems, who sell
    the code with commercial support, indemnification, and middleware, under
    the OpenRTOS brand: http://www.OpenRTOS.com.  High Integrity Systems also
    provide a safety engineered and independently SIL3 certified version under
    the SafeRTOS brand: http://www.SafeRTOS.com.
*/

/*
 * (C) Copyright 2013
 * Promwad Innovation Company <www.promwad.com>
 */

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "osal.h"

/*-----------------------------------------------------------*/

/*
 * Defined by the linker EABI
 */
#if __ghs__
extern unsigned long _SDA2_BASE_[];
#else
extern unsigned long __sdata_start__[];
extern unsigned long __sdata2_start__[];
#endif

/*
 * Function to start the scheduler running by starting the highest
 * priority task that has thus far been created.
 */
extern void vPortStartFirstTask( void );

/*
 * Setup the system timer to generate the tick interrupt.
 */
void prvPortTimerSetup( void );
/*-----------------------------------------------------------*/

/* 
 * Initialise the stack of a task to look exactly as if the task had been
 * interrupted.
 * 
 * See the header file portable.h.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{

#if __ghs__
	register portSTACK_TYPE msr,srr1;

	msr = __GETSR();
	/* External interrupt enable */
	srr1 = msr | (1 << 15);
#else
    /* r3, r4 and r5 are the parameters, first GPRS are r6 and r7 */
	register portSTACK_TYPE msr  asm ("%r6");
	register portSTACK_TYPE srr1 asm ("%r7");

	asm  volatile ("mfmsr	%r6\t\n");
	asm  volatile ("e_ori	%r7, %r6, 32768\t\n");
#endif

	/* leave space for compiler */
    pxTopOfStack -= 8;
    *--pxTopOfStack = 0;                                                        /* XER */
    *--pxTopOfStack = 0;                                                        /* CTR */
    *--pxTopOfStack = ( portSTACK_TYPE ) pxCode;                                /* LR  */
    *--pxTopOfStack = 0;                                                        /* CR  */
    *--pxTopOfStack = 0x31L;                                                    /* r31 */
    *--pxTopOfStack = 0x30L;                                                    /* r30 */
    *--pxTopOfStack = 0x29L;                                                    /* r29 */
    *--pxTopOfStack = 0x28L;                                                    /* r28 */
    *--pxTopOfStack = 0x27L;                                                    /* r27 */
    *--pxTopOfStack = 0x26L;                                                    /* r26 */
    *--pxTopOfStack = 0x25L;                                                    /* r25 */
    *--pxTopOfStack = 0x24L;                                                    /* r24 */
    *--pxTopOfStack = 0x23L;                                                    /* r23 */
    *--pxTopOfStack = 0x22L;                                                    /* r22 */
    *--pxTopOfStack = 0x21L;                                                    /* r21 */
    *--pxTopOfStack = 0x20L;                                                    /* r20 */
    *--pxTopOfStack = 0x19L;                                                    /* r19 */
    *--pxTopOfStack = 0x18L;                                                    /* r18 */
    *--pxTopOfStack = 0x17L;                                                    /* r17 */
    *--pxTopOfStack = 0x16L;                                                    /* r16 */
    *--pxTopOfStack = 0x15L;                                                    /* r15 */
    *--pxTopOfStack = 0x14L;                                                    /* r14 */
#if __ghs__
    *--pxTopOfStack = ( portSTACK_TYPE )_SDA2_BASE_;                            /* r13 */
#else
    *--pxTopOfStack = ( portSTACK_TYPE )__sdata2_start__; /*it should be sdata*//* r13 */
#endif
    *--pxTopOfStack = 0x12L;                                                    /* r12 */
    *--pxTopOfStack = 0x11L;                                                    /* r11 */
    *--pxTopOfStack = 0x10L;                                                    /* r10 */
    *--pxTopOfStack = 0x9L;                                                     /* r09 */
    *--pxTopOfStack = 0x8L;                                                     /* r08 */
    *--pxTopOfStack = 0x7L;                                                     /* r07 */
    *--pxTopOfStack = 0x6L;                                                     /* r06 */
    *--pxTopOfStack = 0x5L;                                                     /* r05 */
    *--pxTopOfStack = 0x4L;                                                     /* r04 */
    *--pxTopOfStack = ( portSTACK_TYPE ) pvParameters;                          /* r03 */
#if __ghs__
    *--pxTopOfStack = ( portSTACK_TYPE )_SDA2_BASE_;                            /* r02 */
#else
    *--pxTopOfStack = ( portSTACK_TYPE )__sdata2_start__;                       /* r02 */
#endif
    *--pxTopOfStack = 0x1L;                                                     /* r01 */
    *--pxTopOfStack = 0x0L;                                                     /* r00 */
    *--pxTopOfStack = srr1;                                                     /* SRR1 */
    *--pxTopOfStack = ( portSTACK_TYPE ) pxCode;                                /* SRR0 */
    *--pxTopOfStack = msr;                                                      /* MSR */
	/* reserved area */
    pxTopOfStack -= 8;
    *pxTopOfStack = 0xBADABADA;

    return pxTopOfStack;
}
/*-----------------------------------------------------------*/

portBASE_TYPE xPortStartScheduler( void )
{
    prvPortTimerSetup();

    vPortStartFirstTask();

    /* Should not get here as the tasks are now running! */
    return pdFALSE;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* Not implemented. */
    for( ;; );
}
/*-----------------------------------------------------------*/

void prvPortTimerSetup(void)
{
#if (OSAL_SYSTIMER_ENABLE == TRUE)
    osalSysTimeSetup(configTICK_RATE_HZ);
#endif /* (OSAL_SYSTIMER_ENABLE == TRUE) */
}
/*-----------------------------------------------------------*/

void vPortTimerReset(void)
{
#if (OSAL_SYSTIMER_ENABLE == TRUE)
    osalSysTimeReset();
#endif /* (OSAL_SYSTIMER_ENABLE == TRUE) */
}
/*-----------------------------------------------------------*/

void OSAL_SYSTIMER_IRQ_HANDLER(void)
{
    xTaskIncrementTick();
#if (configUSE_PREEMPTION == 1)
    vTaskSwitchContext();
#endif
    vPortTimerReset();
}

void vPortEnterCritical( void )
{
    vTaskEnterCritical();
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
    vTaskExitCritical();
}
