/*
 *
 */
#include "crManager.h"

#define _SLOT_FREE    0
#define _SLOT_USED    1

/*
 * Initialize coroutine manager.
 *
 * size of pool must be equal to (nslots * (sizeof(struct crManagerSlot_t) + ssize)). 
 *
 * ARGS
 *     cm
 *     pool
 *     nslots
 *     ssize
 */
void crManagerInit(struct crManager_t *cm,
        uint8_t *pool, size_t nslots, size_t ssize)
{
    size_t i;

    cm->nslots = nslots;
    cm->slot   = (struct crManagerSlot_t *)pool;
    cm->ssize = ssize;

    cm->lastSlotIndex = 0;

    for (i = 0; i < nslots; i++)
    {
        cm->slot[i].state = _SLOT_FREE;
        cm->slot[i].stack = pool + nslots * sizeof(struct crManagerSlot_t) + i * ssize;
    }
}
/*
 * RETURN
 *     Pointer to coroutine or NULL if no slots remain.
 */
struct crManagerSlot_t * crManagerNewCr(struct crManager_t *cm, void (*entry)(), void *context)
{
    size_t n;
    size_t i;
    struct crManagerSlot_t *slot;

    n = cm->nslots;
    i = cm->lastSlotIndex;

    while (n--)
    {
        slot = &cm->slot[i];

        if (slot->state == _SLOT_FREE)
        {
            cm->lastSlotIndex = i; /* NOTE */

            slot->state = _SLOT_USED;
            crCreate(&slot->cr, THISCR, slot->stack, cm->ssize, entry, context);
            return slot;
        }

        i++;
        if (i >= cm->nslots)
            i = 0;
    }

    return NULL;
}
/*
 *
 */
void crManagerLookup(struct crManager_t *cm)
{
    cm->index = 0;
    cm->nlook = cm->nslots;
}
/*
 *
 */
struct crManagerSlot_t * crManagerNext(struct crManager_t *cm)
{
    struct crManagerSlot_t *slot;

    while (cm->nlook--)
    {
        slot = &cm->slot[cm->index++];
        if (cm->index >= cm->nslots)
            cm->index = 0;

        if (slot->state == _SLOT_USED)
            return slot;
    }

    return NULL;
}
struct crManagerSlot_t * crManagerNext2(struct crManager_t *cm)
{
    struct crManagerSlot_t *slot;

    while (cm->nlook--)
    {
        slot = &cm->slot[cm->index++];
        return slot;
    }
    return NULL;
}

/*
 *
 */
void crManagerFreeSlot(struct crManager_t *cm, struct crManagerSlot_t *slot)
{
    slot->state = _SLOT_FREE;
}

