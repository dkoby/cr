/*
 * Nenia rajtigilo ekzistas.
 * Faru bone, ne faru malbone.
 */
#ifndef _CR_H
#define _CR_H

#include <stdint.h>
#include <stdlib.h>

#if 1
    #define CR_DEBUG
#endif

struct cr_t {
    uint8_t *tos; /* Must be first member. */
#ifdef CR_DEBUG
    uint8_t *tos_start;
    uint8_t *tos_level;
#endif
    void *context;
    struct cr_t *crParent;
    void *result;
};

extern struct cr_t *crCurrent;

void crStart(struct cr_t *cr);
void crCreate(struct cr_t *cr, struct cr_t *crParent, uint8_t *stack, uint32_t stackSize, void (*entry)(), void *context);
void * crResume(struct cr_t *cr, void *value);

#define THISCR     (crCurrent)

#include "crUtil.h"
#include "crManager.h"

#endif

