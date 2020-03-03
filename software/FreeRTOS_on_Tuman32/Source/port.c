/*
 * FreeRTOS Kernel V10.3.0
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the RISC-V RV32 port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "include/FreeRTOS.h"
#include "include/task.h"
#include "include/portmacro.h"

/* Standard includes. */
#include "include/string.h"

// #ifdef configCLINT_BASE_ADDRESS
// 	#warning The configCLINT_BASE_ADDRESS constant has been deprecated.  configMTIME_BASE_ADDRESS and configMTIMECMP_BASE_ADDRESS are currently being derived from the (possibly 0) configCLINT_BASE_ADDRESS setting.  Please update to define configMTIME_BASE_ADDRESS and configMTIMECMP_BASE_ADDRESS dirctly in place of configCLINT_BASE_ADDRESS.  See https://www.freertos.org/Using-FreeRTOS-on-RISC-V.html
// #endif

// #ifndef configMTIME_BASE_ADDRESS
// 	#warning configMTIME_BASE_ADDRESS must be defined in FreeRTOSConfig.h.  If the target chip includes a memory-mapped mtime register then set configMTIME_BASE_ADDRESS to the mapped address.  Otherwise set configMTIME_BASE_ADDRESS to 0.  See https://www.freertos.org/Using-FreeRTOS-on-RISC-V.html
// #endif

// #ifndef configMTIMECMP_BASE_ADDRESS
// 	#warning configMTIMECMP_BASE_ADDRESS must be defined in FreeRTOSConfig.h.  If the target chip includes a memory-mapped mtimecmp register then set configMTIMECMP_BASE_ADDRESS to the mapped address.  Otherwise set configMTIMECMP_BASE_ADDRESS to 0.  See https://www.freertos.org/Using-FreeRTOS-on-RISC-V.html
// #endif

/* Let the user override the pre-loading of the initial LR with the address of
prvTaskExitError() in case it messes up unwinding of the stack in the
debugger. */
#ifdef configTASK_RETURN_ADDRESS
	#define portTASK_RETURN_ADDRESS	configTASK_RETURN_ADDRESS
#else
	#define portTASK_RETURN_ADDRESS	prvTaskExitError
#endif

/* The stack used by interrupt service routines.  Set configISR_STACK_SIZE_WORDS
to use a statically allocated array as the interrupt stack.  Alternative leave
configISR_STACK_SIZE_WORDS undefined and update the linker script so that a
linker variable names __freertos_irq_stack_top has the same value as the top
of the stack used by main.  Using the linker script method will repurpose the
stack that was used by main before the scheduler was started for use as the
interrupt stack after the scheduler has started. */
// #ifdef configISR_STACK_SIZE_WORDS
// 	static __attribute__ ((aligned(16))) StackType_t xISRStack[ configISR_STACK_SIZE_WORDS ] = { 0 };
// 	const StackType_t xISRStackTop = ( StackType_t ) &( xISRStack[ configISR_STACK_SIZE_WORDS & ~portBYTE_ALIGNMENT_MASK ] );

// 	//*  Don't use 0xa5 as the stack fill bytes as that is used by the kernerl for
// 	//* the task stacks, and so will legitimately appear in many positions within
// 	//* the ISR stack. 
// 	#define portISR_STACK_FILL_BYTE	0xee
// #else
// 	extern const uint32_t __freertos_irq_stack_top[];
// 	const StackType_t xISRStackTop = ( StackType_t ) __freertos_irq_stack_top;
// #endif

/*
 * Setup the timer to generate the tick interrupts.  The implementation in this
 * file is weak to allow application writers to change the timer used to
 * generate the tick interrupt.
 */
void vPortSetupTimerInterrupt( void ) __attribute__(( weak ));

/*-----------------------------------------------------------*/

/* Used to program the machine timer compare register. */
// uint64_t ullNextTime = 0ULL;
// const uint64_t *pullNextTime = &ullNextTime;
// const size_t uxTimerIncrementsForOneTick = ( size_t ) ( ( configCPU_CLOCK_HZ ) / ( configTICK_RATE_HZ ) ); //* Assumes increment won't go over 32-bits. */
// uint32_t const ullMachineTimerCompareRegisterBase = configMTIMECMP_BASE_ADDRESS;
// volatile uint64_t * pullMachineTimerCompareRegister = NULL;

/* Set configCHECK_FOR_STACK_OVERFLOW to 3 to add ISR stack checking to task
stack checking.  A problem in the ISR stack will trigger an assert, not call the
stack overflow hook function (because the stack overflow hook is specific to a
task stack, not the ISR stack). */
#if defined( configISR_STACK_SIZE_WORDS ) && ( configCHECK_FOR_STACK_OVERFLOW > 2 )
	#warning This path not tested, or even compiled yet.

	static const uint8_t ucExpectedStackBytes[] = {
									portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,		\
									portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,		\
									portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,		\
									portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,		\
									portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE };	\

	#define portCHECK_ISR_STACK() configASSERT( ( memcmp( ( void * ) xISRStack, ( void * ) ucExpectedStackBytes, sizeof( ucExpectedStackBytes ) ) == 0 ) )
#else
	/* Define the function away. */
	#define portCHECK_ISR_STACK()
#endif /* configCHECK_FOR_STACK_OVERFLOW > 2 */

/*-----------------------------------------------------------*/

// #if( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIMECMP_BASE_ADDRESS != 0 )

// 	void vPortSetupTimerInterrupt( void )
// 	{
// 	uint32_t ulCurrentTimeHigh, ulCurrentTimeLow;
// 	volatile uint32_t * const pulTimeHigh = ( volatile uint32_t * const ) ( ( configMTIME_BASE_ADDRESS ) + 4UL ); /* 8-byte typer so high 32-bit word is 4 bytes up. */
// 	volatile uint32_t * const pulTimeLow = ( volatile uint32_t * const ) ( configMTIME_BASE_ADDRESS );
// 	volatile uint32_t ulHartId;

// 		__asm volatile( "csrr %0, mhartid" : "=r"( ulHartId ) );
// 		pullMachineTimerCompareRegister  = ( volatile uint64_t * ) ( ullMachineTimerCompareRegisterBase + ( ulHartId * sizeof( uint64_t ) ) );

// 		do
// 		{
// 			ulCurrentTimeHigh = *pulTimeHigh;
// 			ulCurrentTimeLow = *pulTimeLow;
// 		} while( ulCurrentTimeHigh != *pulTimeHigh );

// 		ullNextTime = ( uint64_t ) ulCurrentTimeHigh;
// 		ullNextTime <<= 32ULL; /* High 4-byte word is 32-bits up. */
// 		ullNextTime |= ( uint64_t ) ulCurrentTimeLow;
// 		ullNextTime += ( uint64_t ) uxTimerIncrementsForOneTick;
// 		*pullMachineTimerCompareRegister = ullNextTime;

// 		/* Prepare the time to use after the next tick interrupt. */
// 		ullNextTime += ( uint64_t ) uxTimerIncrementsForOneTick;
// 	}

// #endif /* ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIME_BASE_ADDRESS != 0 ) */
/*-----------------------------------------------------------*/
extern TCB_t *pxCurrentTCB;	//* added by ljn;
BaseType_t xPortStartScheduler( void )
{
// extern void xPortStartFirstTask( void );

	// #if( configASSERT_DEFINED == 1 )
	// {
	// 	volatile uint32_t mtvec = 0;

	// 	//  Check the least significant two bits of mtvec are 00 - indicating
	// 	// single vector mode. 
	// 	__asm volatile( "csrr %0, mtvec" : "=r"( mtvec ) );
	// 	configASSERT( ( mtvec & 0x03UL ) == 0 );

	// 	//  Check alignment of the interrupt stack - which is the same as the
	// 	// stack that was being used by main() prior to the scheduler being
	// 	// started. 
	// 	configASSERT( ( xISRStackTop & portBYTE_ALIGNMENT_MASK ) == 0 );

	// 	#ifdef configISR_STACK_SIZE_WORDS
	// 	{
	// 		memset( ( void * ) xISRStack, portISR_STACK_FILL_BYTE, sizeof( xISRStack ) );
	// 	}
	// 	#endif	 /* configISR_STACK_SIZE_WORDS */
	// }
	// #endif /* configASSERT_DEFINED */

	// //  If there is a CLINT then it is ok to use the default implementation
	// // in this file, otherwise vPortSetupTimerInterrupt() must be implemented to
	// // configure whichever clock is to be used to generate the tick interrupt. 
	// vPortSetupTimerInterrupt();

	// #if( ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIMECMP_BASE_ADDRESS != 0 ) )
	// {
	// 	//  Enable mtime and external interrupts.  1<<7 for timer interrupt, 1<<11
	// 	// for external interrupt.  _RB_ What happens here when mtime is not present as
	// 	// with pulpino? 
	// 	__asm volatile( "csrs mie, %0" :: "r"(0x880) );
	// }
	// #else
	// {
	// 	/* Enable external interrupts. */
	// 	__asm volatile( "csrs mie, %0" :: "r"(0x800) );
	// }
	// #endif /* ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIMECMP_BASE_ADDRESS != 0 ) */

	//* xPortStartFirstTask();	added by ljn;
	vTaskSwitchContext();
	uint32_t (*func)(void) = (uint32_t (*)(void))*(pxCurrentTCB->pxTopOfStack+32);
	func();

	/* Should not get here as after calling xPortStartFirstTask() only tasks
	should be executing. */
	return pdFAIL;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* Not implemented. */
	for( ;; );
}

StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
	/* save cpuregs 0-31, pxCode, parameters? and an empty,  */
	pxTopOfStack -= 5;
	*pxTopOfStack = ( StackType_t ) pvParameters;
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) pxCode;
	for (int i = 0; i < 32; ++i)
	{
		pxTopOfStack--;
		*pxTopOfStack = 0;
	}
	/** initial register */
	*((uint32_t *)0x20000004) = (uint32_t) pxTopOfStack;
	*pxTopOfStack = (uint32_t) pxCode;
	*(pxTopOfStack+2) = (uint32_t) (pxTopOfStack-1);

	__asm volatile( "addi sp,sp,-16" );			// store ra,a1,a2
	__asm volatile( "sw a1,8(sp)" );
	__asm volatile( "sw a2,4(sp)" );
	__asm volatile( "li a1,0x20000004" );	// ready to read TEMP_M0
	__asm volatile( "lw a2,0(a1)" );		// TEMP_M0: value of TopOfstack point

	// __asm volatile( "li a1,0x10000001" );
	// __asm volatile( "sw a2,0(a1)" );	
	// __asm volatile( "li a1,0x20000008" );	// ready to read TEMP_M0
	// __asm volatile( "lw a1,0(a1)" );		// TEMP_M0: value of TopOfstack point

	// __asm volatile( "sw a1,0(a2)" );		// value of stack
	__asm volatile( "sw x1,4(a2)" );
	// __asm volatile( "sw x2,8(a2)" );		// value of (pxTopOfStack-1), a new stack
	__asm volatile( "sw x3,12(a2)" );
	__asm volatile( "sw x4,16(a2)" );
	__asm volatile( "sw x5,20(a2)" );
	__asm volatile( "sw x6,24(a2)" );
	__asm volatile( "sw x7,28(a2)" );
	__asm volatile( "sw x8,32(a2)" );
	__asm volatile( "sw x9,36(a2)" );
	__asm volatile( "sw x10,40(a2)" );
	__asm volatile( "lw x0,44(a2)" );
	__asm volatile( "lw x0,48(a2)" );
	__asm volatile( "sw x13,52(a2)" );
	__asm volatile( "sw x14,56(a2)" );
	__asm volatile( "sw x15,60(a2)" );
	__asm volatile( "sw x16,64(a2)" );
	__asm volatile( "sw x17,68(a2)" );
	__asm volatile( "sw x18,72(a2)" );
	__asm volatile( "sw x19,76(a2)" );
	__asm volatile( "sw x20,80(a2)" );
	__asm volatile( "sw x21,84(a2)" );
	__asm volatile( "sw x22,88(a2)" );
	__asm volatile( "sw x23,92(a2)" );
	__asm volatile( "sw x24,96(a2)" );
	__asm volatile( "sw x25,100(a2)" );
	__asm volatile( "sw x26,104(a2)" );
	__asm volatile( "sw x27,108(a2)" );
	__asm volatile( "sw x28,112(a2)" );
	__asm volatile( "sw x29,116(a2)" );
	__asm volatile( "sw x30,120(a2)" );
	__asm volatile( "sw x31,124(a2)" );

	__asm volatile( "lw a2,4(sp)" );			// restore a1,a2
	__asm volatile( "lw a1,8(sp)" );
	__asm volatile( "addi sp,sp,16" );
// print_hex(((uint32_t)pxTopOfStack),8);print_str("\tstack in port\n");	
// print_hex(*((uint32_t *)pxTopOfStack),8);print_str("\tfunc in port\n");	
	return pxTopOfStack;
}


//*	added by ljn
void vPortGenSoftwareInterrupt(void){
	// *((uint32_t *)TEMP_M0) = (uint32_t) (pxCurrentTCB->pxTopOfStack);
	__asm volatile( "csrwi sip, 2" );
}

void vPortReleaseTIE(void){
	uint32_t stimer = *((uint32_t *)TIMER_L_ADDR);
	uint32_t stimercmp = configCPU_CLOCK_HZ/configTICK_RATE_HZ + stimer;
	*((uint32_t *)TIMERCMP_L_ADDR) = stimercmp;
	if(stimer > stimercmp){
		*((uint32_t *)TIMER_L_ADDR) = 0;
	}
}

// uint32_t CPU_RunTime;

// #ifdef portCONFIGURE_TIMER_FOR_RUN_TIME_STATS
// 	#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() (CPU_RunTime = 0ul)
// #endif

// #ifdef portGET_RUN_TIME_COUNTER_VALUE
// 	#define portGET_RUN_TIME_COUNTER_VALUE() CPU_RunTime
// #endif


