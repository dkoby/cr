/*
 * Nenia rajtigilo ekzistas.
 * Faru bone, ne faru malbone.
 */
#include <cm3.h>
#include "cr.h"
#include "port.h"

extern struct cr_t *crNext;

/*
 *
 */
inline void crPortInit()
{
    PORT_DISABLE_IRQ();

    /*
     * Set interrupt groups and prorities of interrupts.
     * Groups         8 (bits [7:5])
     * Subpriorities 32 (bits [4:0])
     *
     * LPC17xx subpriorities 4 (bits [4:3], bist [2:0] are unsupported).
     *
     * XXX
     * Check that implementation supports this values.
     */
    SCB->AIRCR = (4 << 8) | (0x05FA << 16);
    /* Set PendSV lowest priority. */
    SCB->SHP[10] = 0xff;

    /*
     *
     */
    asm volatile (
        /* Get TOS of first coroutine. */
        "ldr r0, _cr_current                     \n" /* r0 <- Address of "_cr_current". */
        "ldr r1, [r0]                            \n" /* r1 <- Value of "_cr_current" (Address of "crCurrent"). */
        "ldr r1, [r1, #0]                        \n" /* r1 <- Value of first member of "crCurrent" (tos). */
        /*
         * Get entry point address from corutine's stack.
         * TODO CM4
         */
        "ldr r0, [r1, #56]                      \n"
        "mov lr, r0                             \n"
        "add r1, #64                            \n"
        /* set PSP to point to coroutine's TOS */
        "msr psp, r1                            \n"
        /* set PSP as thread stack pointer */
        "mov r1,#2                               \n"
        "msr control, r1                         \n"
    );

    PORT_ENABLE_IRQ();

    asm volatile (
        "bx lr                                   \n"
    );

    asm volatile (
      	".align 2                             \n"
    	"_cr_current: .word crCurrent         \n"
    );

}
/*
 *
 */
void __attribute__((naked)) crPortSwitch()
{
    /* set PendSV to pending */
    asm volatile (
        ".equ ICSR, 0xE000ED04  \n"
        ".equ PENDBIT, (1 << 28)\n"
        "ldr r0,=ICSR           \n"
        "ldr r1,=PENDBIT        \n"
        "str r1,[r0]            \n"
        "dsb                    \n"
        "isb                    \n"
        "bx  lr                 \n"
    );
}
/*
 *
 */
void __attribute__((naked)) PendSV_Handler(void)
{
    PORT_DISABLE_IRQ();
    asm volatile (
        /* store software context */
        "mrs r0, psp            \n"
        "stmdb r0!,{r4-r11}     \n"
        /* save new TOS */
        "ldr r2, _cr_current    \n"
        "ldr r2, [r2, #0]       \n"
        "str r0, [r2, #0]       \n"
    );

#ifdef CR_DEBUG
    if (crCurrent->tos_level > crCurrent->tos)
        crCurrent->tos_level = crCurrent->tos;
#endif
    if (crNext)
        crCurrent = crNext;

    asm volatile (
        /* restore context of new coroutine */
        "ldr r2, _cr_current    \n"
        "ldr r2, [r2, #0]       \n"
        "ldr r0, [r2, #0]       \n"
        "ldmia r0!, {r4-r11}    \n"
        /* move PSP to new coroutine's stack */
        "msr psp, r0            \n"
    );
    PORT_ENABLE_IRQ();
    asm volatile (
        "bx lr\n"
    );
}


