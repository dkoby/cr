/*
 * Nenia rajtigilo ekzistas.
 * Faru bone, ne faru malbone.
 */
#ifndef _CR_ARMV7_M_PORT_H
#define _CR_ARMV7_M_PORT_H

#include <stdint.h>

#define PORT_DISABLE_IRQ() \
        asm volatile("cpsid i\n")
#define PORT_ENABLE_IRQ()  \
        asm volatile(      \
            "cpsie i\n"    \
            "isb    \n"    \
        )

#define PORT_STACK_DIR_DECREASE
 
#pragma pack(push, 1)
struct crPortContext_t {
    /* Context stored by software. */
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    /* Context stored by ARMv7-M core. */
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;
};
#pragma pack(pop)

void crPortInit();
void __attribute__((naked)) crPortSwitch();

#endif

