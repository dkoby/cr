/*
 *
 */
#ifndef _CR_MANAGER_H
#define _CR_MANAGER_H

#include "cr.h"

struct crManager_t {
    struct crManagerSlot_t {
        int state;
        uint8_t *stack;
        struct cr_t cr;
    } *slot;
    size_t nslots;
    size_t ssize;
    size_t lastSlotIndex;

    size_t index;
    size_t nlook;
};

#define CR_MANAGER_POOL_SIZE(nslots, ssize) ((nslots) * (sizeof(struct crManagerSlot_t) + (ssize)))
#define CR_MANAGER_POOL(name, nslots, ssize) static uint8_t name[CR_MANAGER_POOL_SIZE(nslots, ssize)]

void crManagerInit(struct crManager_t *cm,
        uint8_t *pool, size_t nslots, size_t ssize);
struct crManagerSlot_t * crManagerNewCr(struct crManager_t *cm,
        void (*entry)(), void *context);
void crManagerFreeSlot(struct crManager_t *cm, struct crManagerSlot_t *slot);
void crManagerLookup(struct crManager_t *cm);
struct crManagerSlot_t * crManagerNext(struct crManager_t *cm);
struct crManagerSlot_t * crManagerNext2(struct crManager_t *cm);

#endif

