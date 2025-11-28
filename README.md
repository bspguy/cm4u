```text
   _____  __  __  _  _   _   _   _   _   _   _ 
  / ____||  \/  || || | | | | | | \ | | | | | |
 | |     | \  / || || |_| | | | |  \| | | |_| |
 | |     | |\/| ||__   _| | | | | . ` | |  _  |
 | |____ | |  | |   | | | |_| | | |\  | | | | |
  \_____||_|  |_|   |_|  \___/  |_| \_| |_| |_|

        C O R T E X - M 4   U T I L I T I E S
```

# cm4u – Cortex‑M4 Core Utilities

Tiny, header‑only, **not‑a-HAL** core utilities for Cortex‑M4.

Prefix is `cm4u_` (Cortex‑M4 Utilities).  
Focus is on **register‑level goodies** you always end up re‑writing:

- DWT‑based cycle counter + delays (us / ms)
- Check if you’re in **Thread** or **Handler** mode
- CONTROL / PRIMASK / BASEPRI / FAULTMASK helpers
- MSP / PSP helpers
- Barriers (`DMB/DSB/ISB`) and `NOP`
- PendSV + SVC triggers
- Light NVIC helpers
- Tiny profiling helpers

No drivers, no peripherals, no HAL/BSP bloat.  
Just the **core‑core** bits.

---

## Files

- `cm4u_core.h` – header‑only CM4 utilities (all `static inline`).
- `example_main.c` – tiny usage example.

Drop `cm4u_core.h` next to your CMSIS device headers and go.

---

## Quick Start

```c
#include "stm32f4xx.h"   // or your device
#include "core_cm4.h"
#include "cm4u_core.h"

int main(void)
{
    const uint32_t core_hz = 168000000u;   // your core clock

    // Enable DWT cycle counter once at startup
    cm4u_dwt_init();

    // Simple 10 us delay
    cm4u_delay_us(10u, core_hz);

    while (1) {
        uint32_t start = cm4u_profile_cycles_start();

        // Your low‑jitter, real‑time magic here
        __NOP();

        uint32_t spent_cycles = cm4u_profile_cycles_end(start);
        (void)spent_cycles;
    }
}
```

---

## Thread vs Handler Mode

```c
void debug_where_am_i(void)
{
    if (cm4u_in_thread_mode()) {
        // main / task / normal code
    } else if (cm4u_in_handler_mode()) {
        uint32_t exc = cm4u_get_exception_number();
        (void)exc; // e.g. log or blink a different LED
    }
}
```

---

## Critical Sections

```c
uint32_t primask = cm4u_critical_enter();
/* non‑interruptible section */
cm4u_critical_exit(primask);
```

This uses **PRIMASK** so you can safely nest different critical sections
(if each stores and restores its own previous PRIMASK).

---

## DWT‑Based Delays

```c
void delay_example(uint32_t core_hz)
{
    cm4u_dwt_init();              // once is enough, but cheap if called again

    cm4u_delay_us(5, core_hz);    // 5 microseconds
    cm4u_delay_ms(1, core_hz);    // 1 millisecond
}
```

This is **busy‑wait** and fully synchronous:
- jitter is basically just the loop and `NOP`
- ideal for short, deterministic waits
- not for multi‑millisecond “sleep the system” style delays

---

## System Tricks

```c
// Trigger PendSV (e.g. your own context switcher)
cm4u_trigger_pendsv();

// Trigger SVC #0 (supervisor call)
cm4u_trigger_svc(0);

// Software reset
cm4u_system_reset();
```

---

## License

MIT
