/**************************************************************************/
/*                                                                        */
/*   ThreadX Cortex-M33/GNU low-level initialization                      */
/*                                                                        */
/*   Board-specific low-level ThreadX bring-up (vector table relocation,  */
/*   SysTick configuration, system-handler priorities). This file is not  */
/*   part of the `azrtos::threadx` submodule build (ThreadX ships it only */
/*   under ports/cortex_m33/gnu/example_build/, as integrator-supplied    */
/*   BSP glue) -- it lives here, in Core/Src/rtos, as an app-owned file.  */
/*                                                                        */
/*                                                                        */
/*   SYSTEM_CLOCK = 160000000 matches project's own                       */
/*   SystemClock_Config() (HSI 16 MHz, PLLM=1, PLLN=10 -> VCO 160 MHz,    */
/*   PLLR=1 -> SYSCLK 160 MHz)                                 .          */
/*                                                                        */
/*   SYSTICK_CYCLES divisor: set to /1000 (1 ms tick) to match            */
/*   the project's own tx_user.h, which declares                          */
/*   TX_TIMER_TICKS_PER_SECOND 1000.                                      */
/*                                                                        */
/**************************************************************************/

// by default AzureRTOS is configured to use static byte pool for memory
// allocation, USE_DYNAMIC_MEMORY_ALLOCATION is intentionally left
// undefined.
//#define USE_DYNAMIC_MEMORY_ALLOCATION

SYSTEM_CLOCK      =   160000000
SYSTICK_CYCLES    =   ((SYSTEM_CLOCK / 1000) -1)

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_initialize_low_level                          Cortex-M33/GNU    */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is responsible for any low-level processor            */
/*    initialization, including setting up interrupt vectors, setting     */
/*    up a periodic timer interrupt source, saving the system stack       */
/*    pointer for use in ISR processing later, and finding the first      */
/*    available RAM memory address for tx_application_define.             */
/*                                                                        */
/**************************************************************************/
// VOID   _tx_initialize_low_level(VOID)
// {
    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  _tx_initialize_low_level
    .thumb_func
.type _tx_initialize_low_level, function
_tx_initialize_low_level:

    /* Disable interrupts during ThreadX initialization.  */
    CPSID   i

    /* Set base of available memory to end of non-initialised RAM area.  */
#ifdef USE_DYNAMIC_MEMORY_ALLOCATION
    LDR     r0, =_tx_initialize_unused_memory       // Build address of unused memory pointer
    LDR     r1, =__RAM_segment_used_end__           // Build first free address
    ADD     r1, r1, #4                              //
    STR     r1, [r0]                                // Setup first unused memory pointer
#endif
    /* Setup Vector Table Offset Register.  */
    MOV     r0, #0xE000E000                         // Build address of NVIC registers
    LDR     r1, =g_pfnVectors                       // Pickup address of vector table
    STR     r1, [r0, #0xD08]                        // Set vector table address

    /* Enable the cycle count register.  */
    LDR     r0, =0xE0001000                         // Build address of DWT register
    LDR     r1, [r0]                                // Pickup the current value
    ORR     r1, r1, #1                              // Set the CYCCNTENA bit
    STR     r1, [r0]                                // Enable the cycle count register

    /* Set system stack pointer from vector value.  */
    LDR     r0, =_tx_thread_system_stack_ptr        // Build address of system stack pointer
    LDR     r1, =g_pfnVectors                       // Pickup address of vector table
    LDR     r1, [r1]                                // Pickup reset stack pointer
    STR     r1, [r0]                                // Save system stack pointer

    /* Configure SysTick.  */
    MOV     r0, #0xE000E000                         // Build address of NVIC registers
    LDR     r1, =SYSTICK_CYCLES
    STR     r1, [r0, #0x14]                         // Setup SysTick Reload Value
    MOV     r1, #0x7                                // Build SysTick Control Enable Value
    STR     r1, [r0, #0x10]                         // Setup SysTick Control

    /* Configure handler priorities.  */
    LDR     r1, =0x00000000                         // Rsrv, UsgF, BusF, MemM
    STR     r1, [r0, #0xD18]                        // Setup System Handlers 4-7 Priority Registers

    LDR     r1, =0xFF000000                         // SVCl, Rsrv, Rsrv, Rsrv
    STR     r1, [r0, #0xD1C]                        // Setup System Handlers 8-11 Priority Registers
                                                    // Note: SVC must be lowest priority, which is 0xFF

    LDR     r1, =0x40FF0000                         // SysT, PnSV, Rsrv, DbgM
    STR     r1, [r0, #0xD20]                        // Setup System Handlers 12-15 Priority Registers
                                                    // Note: PnSV must be lowest priority, which is 0xFF

    /* Return to caller.  */
    BX      lr
// }

/* Define shells for each of the unused vectors.  */
    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  __tx_BadHandler
    .thumb_func
.type __tx_BadHandler, function
__tx_BadHandler:
    B       __tx_BadHandler

    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  __tx_IntHandler
    .thumb_func
.type __tx_IntHandler, function
__tx_IntHandler:
// VOID InterruptHandler (VOID)
// {
    PUSH    {r0,lr}     // Save LR (and dummy r0 to maintain stack alignment)
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    BL      _tx_execution_isr_enter             // Call the ISR enter function
#endif
    /* Do interrupt handler work here */
    /* .... */
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    BL      _tx_execution_isr_exit              // Call the ISR exit function
#endif
    POP     {r0,lr}
    BX      lr
// }

    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  SysTick_Handler
    .thumb_func
.type SysTick_Handler, function
SysTick_Handler:
// VOID TimerInterruptHandler (VOID)
// {
    PUSH    {r0,lr}     // Save LR (and dummy r0 to maintain stack alignment)
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    BL      _tx_execution_isr_enter             // Call the ISR enter function
#endif
    BL      _tx_timer_interrupt
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
    BL      _tx_execution_isr_exit              // Call the ISR exit function
#endif
    POP     {r0,lr}
    BX      lr
// }

    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  __tx_NMIHandler
    .thumb_func
.type __tx_NMIHandler, function
__tx_NMIHandler:
    B       __tx_NMIHandler

    .section .text
    .balign 4
    .syntax unified
    .eabi_attribute Tag_ABI_align_preserved, 1
    .global  __tx_DBGHandler
    .thumb_func
.type __tx_DBGHandler, function
__tx_DBGHandler:
    B       __tx_DBGHandler

    .end
