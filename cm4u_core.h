#ifndef CM4U_CORE_H
#define CM4U_CORE_H

/*
 * Cortex-M4 low-level core utilities (not a full HAL/BSP).
 * Prefix: cm4u_  (Cortex-M4 Utilities)
 *
 * Requires:
 *   - CMSIS-Core for Cortex-M4 (core_cm4.h)
 *   - Your device header should be included before this, or make sure core_cm4.h is available.
 */

#include <stdint.h>
#include <stdbool.h>
#include "core_cm4.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------------------
 *  Modes & basic types
 * -------------------------------------------------------------------------- */

typedef enum {
    CM4U_MODE_THREAD  = 0,
    CM4U_MODE_HANDLER = 1
} cm4u_mode_t;

/* --------------------------------------------------------------------------
 *  Core mode / exception helpers
 * -------------------------------------------------------------------------- */

/* Get current IPSR (exception number, 0 = Thread mode) */
static inline uint32_t cm4u_get_ipsr(void)
{
    return __get_IPSR();
}

/* Are we currently in Thread mode (normal code, not an ISR)? */
static inline bool cm4u_in_thread_mode(void)
{
    return (__get_IPSR() == 0u);
}

/* Are we in Handler mode (inside an exception / ISR)? */
static inline bool cm4u_in_handler_mode(void)
{
    return (__get_IPSR() != 0u);
}

/* Get current exception number (0 = Thread, 1 = Reset, 2 = NMI, 3 = HardFault, ...) */
static inline uint32_t cm4u_get_exception_number(void)
{
    return (__get_IPSR() & 0x1FFu);
}

/* --------------------------------------------------------------------------
 *  Interrupt / critical section helpers
 * -------------------------------------------------------------------------- */

/* Globally disable interrupts (PRIMASK = 1) */
static inline void cm4u_global_irq_disable(void)
{
    __disable_irq();
}

/* Globally enable interrupts (PRIMASK = 0) */
static inline void cm4u_global_irq_enable(void)
{
    __enable_irq();
}

/* Enter critical section: returns previous PRIMASK value, then disables IRQs */
static inline uint32_t cm4u_critical_enter(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

/* Exit critical section: restore previous PRIMASK */
static inline void cm4u_critical_exit(uint32_t primask)
{
    __set_PRIMASK(primask);
}

/* Get / set PRIMASK (global IRQ mask) */
static inline uint32_t cm4u_get_primask(void)
{
    return __get_PRIMASK();
}

static inline void cm4u_set_primask(uint32_t primask)
{
    __set_PRIMASK(primask);
}

/* Get / set BASEPRI (priority mask for configurable interrupts) */
static inline uint32_t cm4u_get_basepri(void)
{
    return __get_BASEPRI();
}

static inline void cm4u_set_basepri(uint32_t basepri)
{
    __set_BASEPRI(basepri);
}

/* Get / set FAULTMASK (mask all except NMI and HardFault) */
static inline uint32_t cm4u_get_faultmask(void)
{
    return __get_FAULTMASK();
}

static inline void cm4u_set_faultmask(uint32_t faultmask)
{
    __set_FAULTMASK(faultmask);
}

/* --------------------------------------------------------------------------
 *  CONTROL register helpers (SP selection, privilege)
 * -------------------------------------------------------------------------- */

/* Get CONTROL register */
static inline uint32_t cm4u_get_control(void)
{
    return __get_CONTROL();
}

/* Set CONTROL register (be careful: may change SP and privilege) */
static inline void cm4u_set_control(uint32_t control)
{
    __set_CONTROL(control);
    __ISB(); /* ensure new CONTROL takes effect immediately */
}

/* Are we using MSP (Main Stack Pointer)? */
static inline bool cm4u_using_msp(void)
{
    return ((__get_CONTROL() & 0x2u) == 0u);
}

/* Are we using PSP (Process Stack Pointer)? */
static inline bool cm4u_using_psp(void)
{
    return ((__get_CONTROL() & 0x2u) != 0u);
}

/* Get MSP / PSP raw values */
static inline uint32_t cm4u_get_msp(void)
{
    return __get_MSP();
}

static inline uint32_t cm4u_get_psp(void)
{
    return __get_PSP();
}

/* --------------------------------------------------------------------------
 *  Barriers & NOPs
 * -------------------------------------------------------------------------- */

static inline void cm4u_dmb(void) { __DMB(); }
static inline void cm4u_dsb(void) { __DSB(); }
static inline void cm4u_isb(void) { __ISB(); }

static inline void cm4u_nop(void) { __NOP(); }

/* --------------------------------------------------------------------------
 *  System control (SCB / SYSTICK / PendSV / SVC)
 * -------------------------------------------------------------------------- */

/* Trigger a system reset via SCB */
static inline void cm4u_system_reset(void)
{
    NVIC_SystemReset();
}

/* Trigger PendSV for context switch, etc. */
static inline void cm4u_trigger_pendsv(void)
{
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    __ISB();
}

/*
 * Trigger an SVC with an immediate value (0..255).
 * Example: cm4u_trigger_svc(0);
 */
static inline void cm4u_trigger_svc(uint8_t imm8)
{
#if defined(__GNUC__) || defined(__clang__)
    __asm volatile ("svc %0" :: "I"(imm8) : "memory");
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
    __svc(imm8)();
#else
    (void)imm8; /* Unsupported compiler; provide your own implementation */
#endif
}

/* Read SysTick current value (for simple profiling/timebase) */
static inline uint32_t cm4u_systick_get_value(void)
{
    return SysTick->VAL;
}

/* --------------------------------------------------------------------------
 *  DWT-based cycle counter & delays (high resolution)
 * -------------------------------------------------------------------------- */

/*
 * Initialize DWT CYCCNT for timing.
 * Returns true if DWT is present and enabled, false otherwise.
 *
 * Call this once at startup before using cycle counter / delay functions.
 */
static inline bool cm4u_dwt_init(void)
{
#if defined(DWT) && defined(CoreDebug)
    /* Enable trace in DEMCR (needed for DWT on many MCUs) */
    if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0u) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }

    /* Reset and enable cycle counter */
    DWT->CYCCNT = 0u;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

    return true;
#else
    return false;
#endif
}

/* Get current DWT cycle counter (wraps at 2^32) */
static inline uint32_t cm4u_dwt_get_cycles(void)
{
#if defined(DWT)
    return DWT->CYCCNT;
#else
    return 0u;
#endif
}

/* Busy-wait for a given number of CPU cycles (requires cm4u_dwt_init) */
static inline void cm4u_delay_cycles(uint32_t cycles)
{
#if defined(DWT)
    uint32_t start = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
        __NOP();
    }
#else
    (void)cycles;
    /* DWT not available; implement a fallback if needed */
#endif
}

/* Convert microseconds to cycles (no overflow protection) */
static inline uint32_t cm4u_us_to_cycles(uint32_t us, uint32_t core_clock_hz)
{
    /* core_clock_hz * us / 1e6 */
    uint64_t tmp = (uint64_t)core_clock_hz * (uint64_t)us;
    tmp /= 1000000u;
    return (uint32_t)tmp;
}

/* Convert milliseconds to cycles (no overflow protection) */
static inline uint32_t cm4u_ms_to_cycles(uint32_t ms, uint32_t core_clock_hz)
{
    /* core_clock_hz * ms / 1000 */
    uint64_t tmp = (uint64_t)core_clock_hz * (uint64_t)ms;
    tmp /= 1000u;
    return (uint32_t)tmp;
}

/* Delay for given microseconds using DWT (requires cm4u_dwt_init + core_clock_hz) */
static inline void cm4u_delay_us(uint32_t us, uint32_t core_clock_hz)
{
    uint32_t cycles = cm4u_us_to_cycles(us, core_clock_hz);
    cm4u_delay_cycles(cycles);
}

/* Delay for given milliseconds using DWT */
static inline void cm4u_delay_ms(uint32_t ms, uint32_t core_clock_hz)
{
    uint32_t cycles = cm4u_ms_to_cycles(ms, core_clock_hz);
    cm4u_delay_cycles(cycles);
}

/* --------------------------------------------------------------------------
 *  Simple NVIC helpers
 * -------------------------------------------------------------------------- */

/* Set priority of an IRQ (IRQn from your device header) */
static inline void cm4u_nvic_set_priority(IRQn_Type irqn, uint32_t priority)
{
    NVIC_SetPriority(irqn, priority);
}

/* Enable / disable / clear pending for IRQ */
static inline void cm4u_nvic_enable_irq(IRQn_Type irqn)
{
    NVIC_EnableIRQ(irqn);
}

static inline void cm4u_nvic_disable_irq(IRQn_Type irqn)
{
    NVIC_DisableIRQ(irqn);
}

static inline void cm4u_nvic_set_pending(IRQn_Type irqn)
{
    NVIC_SetPendingIRQ(irqn);
}

static inline void cm4u_nvic_clear_pending(IRQn_Type irqn)
{
    NVIC_ClearPendingIRQ(irqn);
}

/* --------------------------------------------------------------------------
 *  Small profiling helper
 * -------------------------------------------------------------------------- */

/*
 * Measure cycles taken by a function-like block.
 *
 * Usage:
 *   uint32_t cycles = cm4u_profile_cycles_start();
 *   // ... code ...
 *   cycles = cm4u_profile_cycles_end(cycles);
 */
static inline uint32_t cm4u_profile_cycles_start(void)
{
    return cm4u_dwt_get_cycles();
}

static inline uint32_t cm4u_profile_cycles_end(uint32_t start_cycles)
{
    uint32_t now = cm4u_dwt_get_cycles();
    return (uint32_t)(now - start_cycles);
}

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* CM4U_CORE_H */
