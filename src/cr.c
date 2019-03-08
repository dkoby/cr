/*
 * Nenia rajtigilo ekzistas.
 * Faru bone, ne faru malbone.
 */
#include "cr.h"
#if PORT_ARMV7M
    #include "port/ARMv7-M/port.h"
#endif

struct cr_t *crCurrent;
struct cr_t *crNext;
/*
 *
 */
void crStart(struct cr_t *cr)
{
    crCurrent = cr;
    crPortInit();
}
/*
 * NOTE
 *     Stack size must be not less then sizeof(struct crPortContext_t), and even bigger.
 *     Good choice is to make it at least double size of sizeof(struct crPortContext_t).
 */
void crCreate(struct cr_t *cr, struct cr_t *crParent, uint8_t *stack, uint32_t stackSize, void (*entry)(), void *context)
{
    struct crPortContext_t *portContext;

    cr->context = context;
    cr->crParent = crParent;


#ifdef PORT_STACK_DIR_DECREASE
    cr->tos = (uint8_t*)((uint32_t)stack + stackSize);
    cr->tos -= sizeof(struct crPortContext_t);
#else
    cr->tos = stack;
#endif

#ifdef CR_DEBUG
    cr->tos_start = cr->tos;
    cr->tos_level = cr->tos;
#endif

    portContext = (struct crPortContext_t*)cr->tos;

#if PORT_ARMV7M
    portContext->xpsr = 0x01000000; /* set T bit */
    portContext->pc = ((uint32_t)entry) | 0x01;
    portContext->lr = portContext->pc;
#endif
}
/*
 * Resume coroutine.
 * Can be used from IRQ, but interrupts must be disabled.
 *
 * ARGS
 *     cr        Coroutine to resume. If NULL then resume parent coroutine (yield).
 *
 */
void * crResume(struct cr_t *cr, void *value)
{
    if (cr)
        crNext = cr;
    else if (crCurrent->crParent)
        crNext = crCurrent->crParent;
    else
        return value;

    crNext->result = value;

    crPortSwitch();

    return crCurrent->result;
}

