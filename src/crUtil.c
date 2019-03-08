/*
 *
 */
#include <string.h>
#include <stimer.h>
#include "crUtil.h"

struct crQeueuMessage_t {
    struct crQeueuMessageHead_t head;
    uint8_t data[];
};

/*
 *
 */
void crUtilWait(struct cr_t *cr, void *result, uint32_t timeout)
{
    uint32_t to;
    to = stimer_gettime();

    do
    {
        crResume(cr, result);
    } while (stimer_deltatime(to) < timeout);
}
/*
 * ARGS
 *     bits    Variable in which monitor bits.
 *     mask    Mask of bits to monitor.
 *     flags   
 *         CR_UTIL_WAIT_ALL       Wait when all bits supplied by mask occured.
 *         CR_UTIL_FLAG_NOWAIT    Don't wait if queue is full, return
 *                                immediately with CR_UTIL_ERR_WOULDLOCK error code
 *
 * RETURN
 *     CR_UTIL_ERR_NONE       If event present.
 *     CR_UTIL_ERR_TIMEOUT    Timeout occured before event raised.
 *     CR_UTIL_ERR_WOULDLOCK  No event present and 
 */
int crUtilWaitEvent(struct cr_t *cr, void *result, uint32_t *bits, uint32_t mask, uint32_t flags, uint32_t timeout)
{
    uint32_t to;

    if (!(*bits & mask) && (flags & CR_UTIL_FLAG_NOWAIT))
        return CR_UTIL_ERR_WOULDLOCK;

    to = stimer_gettime();
    while(1)
    {
        if (*bits & mask)
        {
            if (flags & CR_UTIL_FLAG_CLEAR)
                *bits &= ~mask;
            return CR_UTIL_ERR_NONE;
        }
        if (timeout != CR_UTIL_WAIT_FOREVER && stimer_deltatime(to) >= timeout)
            return CR_UTIL_ERR_TIMEOUT;
        crResume(cr, result);
    }

    return CR_UTIL_ERR_NOTREACHED;
}
/*
 *
 */
void crUtilRaiseEvent(struct cr_t *cr, void *result, uint32_t *bits, uint32_t mask)
{
    *bits |= mask;
    crResume(cr, result);
}
/*
 *
 */
void crUtilClearEvent(uint32_t *bits, uint32_t mask)
{
    *bits &= ~mask;
}
/*
 * ARGS
 *     mutex  Variable in which monitor bits.
 *     mask    Mask of bits to monitor.
 *     flags   
 *         CR_UTIL_WAIT_ALL       Wait when all bits supplied by mask occured.
 *         CR_UTIL_FLAG_NOWAIT    Don't wait if queue is full, return
 *                                immediately with CR_UTIL_ERR_WOULDLOCK error code
 *
 * RETURN
 *     CR_UTIL_ERR_NONE       If event present.
 *     CR_UTIL_ERR_TIMEOUT    Timeout occured before event raised.
 *     CR_UTIL_ERR_WOULDLOCK  No event present and 
 */
int crUtilMutexLock(struct cr_t *cr, void *result, uint32_t *mutex, uint32_t mask, uint32_t flags, uint32_t timeout)
{
    uint32_t to;

    if ((*mutex & mask) && (flags & CR_UTIL_FLAG_NOWAIT))
        return CR_UTIL_ERR_WOULDLOCK;

    to = stimer_gettime();
    while(1)
    {
        if (*mutex & mask)
        {
            if (timeout != CR_UTIL_WAIT_FOREVER && stimer_deltatime(to) >= timeout)
                return CR_UTIL_ERR_TIMEOUT;
            crResume(cr, result);
            continue;
        }
        *mutex |= mask;
        return CR_UTIL_ERR_NONE;
    }

    return CR_UTIL_ERR_NOTREACHED;
}
/*
 *
 */
void crUtilMutexUnlock(uint32_t *mutex, uint32_t mask)
{
    *mutex &= ~mask;
}
/*
 *
 */
void crUtilRMutexInit(struct crRMutex_t *rmutex)
{
    rmutex->owner = NULL;
    rmutex->lockCnt = 0;
}
/*
 * ARGS
 *     rmutex  Recursive mutex variable.
 *     mask    Mask of bits to monitor.
 *     flags   
 *         CR_UTIL_WAIT_ALL       Wait when all bits supplied by mask occured.
 *         CR_UTIL_FLAG_NOWAIT    Don't wait if queue is full, return
 *                                immediately with CR_UTIL_ERR_WOULDLOCK error code
 *
 * RETURN
 *     CR_UTIL_ERR_NONE       If event present.
 *     CR_UTIL_ERR_TIMEOUT    Timeout occured before event raised.
 *     CR_UTIL_ERR_WOULDLOCK  No event present and 
 */
int crUtilRMutexLock(struct cr_t *cr, void *result, struct crRMutex_t *rmutex,
        uint32_t flags, uint32_t timeout)
{
    uint32_t to;

    if (rmutex->lockCnt == 0 || rmutex->owner == crCurrent)
    {
        rmutex->lockCnt++;
        rmutex->owner = crCurrent;
        return CR_UTIL_ERR_NONE;
    }
    if (flags & CR_UTIL_FLAG_NOWAIT)
        return CR_UTIL_ERR_WOULDLOCK;

    to = stimer_gettime();
    while(1)
    {
        if (rmutex->lockCnt)
        {
            if (timeout != CR_UTIL_WAIT_FOREVER && stimer_deltatime(to) >= timeout)
                return CR_UTIL_ERR_TIMEOUT;
            crResume(cr, result);
            continue;
        }

        rmutex->lockCnt++;
        rmutex->owner = crCurrent;
        return CR_UTIL_ERR_NONE;
    }

    return CR_UTIL_ERR_NOTREACHED;
}
/*
 *
 */
void crUtilRMutexUnlock(struct crRMutex_t *rmutex)
{
    rmutex->lockCnt--;
    if (rmutex->lockCnt == 0)
        rmutex->owner = NULL;
}
/*
 * Size of pool must be (nslots * (sizeof(struct crQeueuMessageHead_t) + msize))
 */
void crUtilQueueInit(struct crQueue_t *q, uint8_t *pool, size_t nslots, size_t msize)
{
    q->pool   = pool;
    q->nslots = nslots;
    q->msize  = msize;
    q->wp     = 0;
    q->cnt    = 0;
}
/*
 * ARGS
 *     flags       
 *                 CR_UTIL_FLAG_NONE      No flag specified.
 *                 CR_UTIL_FLAG_NOWAIT    Don't wait if queue is full, return
 *                                        immediately with CR_UTIL_ERR_WOULDLOCK error code
 *
 * RETURN
 *     CR_UTIL_ERR_NONE       if message sucessuflly was added to queue
 *     CR_UTIL_ERR_TIMEOUT    timeout occured before space appeared in queue
 *     CR_UTIL_ERR_WOULDLOCK  queue was full but OS_FLAG_NOWAIT was specified
 */
int crUtilQueueAdd(struct crQueue_t *q, void *data, size_t size, uint32_t flags, uint32_t timeout)
{
    uint32_t to;
    if (size > q->msize)
        return CR_UTIL_ERR_NOTREACHED;

    to = stimer_gettime();
    while (1)
    {
        if (q->cnt < q->nslots)
        {
            struct crQeueuMessage_t *msg;
            msg = (struct crQeueuMessage_t *)(q->pool + q->wp * (sizeof(struct crQeueuMessageHead_t) + q->msize));

            msg->head.size = size;
            memcpy(msg->data, data, size);
            q->cnt++;
            q->wp++;
            if (q->wp >= q->nslots)
                q->wp = 0;
            return CR_UTIL_ERR_NONE;
        }
        if (flags & CR_UTIL_FLAG_NOWAIT)
            return CR_UTIL_ERR_WOULDLOCK;
        if (timeout != CR_UTIL_WAIT_FOREVER && stimer_deltatime(to) >= timeout)
            return CR_UTIL_ERR_TIMEOUT;
        crResume(NULL, NULL);
    }

    return CR_UTIL_ERR_NOTREACHED;
}
/*
 * ARGS
 *     flags       
 *                 CR_UTIL_FLAG_NONE      No flag specified.
 *                 CR_UTIL_FLAG_NOWAIT    Don't wait if queue is empty, return
 *                                        immediately with CR_UTIL_ERR_WOULDLOCK error code
 *
 * RETURN
 *     CR_UTIL_ERR_NONE       If message sucessuflly was added to queue.
 *     CR_UTIL_ERR_TIMEOUT    Timeout occured before space appeared in queue.
 *     CR_UTIL_ERR_WOULDLOCK  Queue was full but OS_FLAG_NOWAIT was specified.
 */
int crUtilQueueRemove(struct crQueue_t *q, void *data, size_t *size, uint32_t flags, uint32_t timeout)
{
    uint32_t to;

    to = stimer_gettime();
    while (1)
    {
        if (q->cnt)
        {
            struct crQeueuMessage_t *msg;
            uint32_t rp;

            if (q->wp >= q->cnt)
                rp = q->wp - q->cnt;
            else
                rp = q->nslots - q->cnt + q->wp;

            q->cnt--;
            msg = (struct crQeueuMessage_t *)(q->pool + rp * (sizeof(struct crQeueuMessageHead_t) + q->msize));
            if (size != NULL && msg->head.size > *size)
                return CR_UTIL_ERR_NOTREACHED;

            memcpy(data, msg->data, msg->head.size);
            if (size != NULL)
                *size = msg->head.size;
            return CR_UTIL_ERR_NONE;
        }
        if (flags & CR_UTIL_FLAG_NOWAIT)
            return CR_UTIL_ERR_WOULDLOCK;
        if (timeout != CR_UTIL_WAIT_FOREVER && stimer_deltatime(to) >= timeout)
            return CR_UTIL_ERR_TIMEOUT;
        crResume(NULL, NULL);
    }

    return CR_UTIL_ERR_NOTREACHED;
}

