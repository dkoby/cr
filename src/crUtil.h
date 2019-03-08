/*
 *
 */
#ifndef _CR_UTIL_H
#define _CR_UTIL_H

#include "cr.h"

#define CR_UTIL_WAIT_FOREVER    0

#define CR_UTIL_FLAG_NONE    0
#define CR_UTIL_FLAG_NOWAIT  (1 << 0)
#define CR_UTIL_FLAG_CLEAR   (1 << 1)

#define CR_UTIL_ERR_NONE          0
#define CR_UTIL_ERR_TIMEOUT       1
#define CR_UTIL_ERR_WOULDLOCK     2
#define CR_UTIL_ERR_NOTREACHED  100

struct crQueue_t {
    uint8_t *pool;
    uint32_t nslots;
    uint32_t msize;
    uint32_t wp;
    uint32_t cnt;
};
struct crQeueuMessageHead_t {
    size_t size;
};
struct crRMutex_t {
    struct cr_t *owner;
    int lockCnt;
};

void crUtilWait(struct cr_t *cr, void *result, uint32_t timeout);
#define crUtilWait2(timeout) crUtilWait(NULL, NULL, timeout)
int crUtilWaitEvent(struct cr_t *cr, void *result, uint32_t *bits, uint32_t mask, uint32_t flags, uint32_t timeout);
#define crUtilWaitEvent2(bits, mask, flags, timeout) crUtilWaitEvent(NULL, NULL, bits, mask, flags, timeout)
void crUtilRaiseEvent(struct cr_t *cr, void *result, uint32_t *bits, uint32_t mask);
#define crUtilRaiseEvent2(bits, mask) crUtilRaiseEvent(NULL, NULL, bits, mask)
void crUtilClearEvent(uint32_t *bits, uint32_t mask);

int crUtilMutexLock(struct cr_t *cr, void *result, uint32_t *bits, uint32_t mutex, uint32_t flags, uint32_t timeout);
#define crUtilMutexLock2(mutex, mask, flags, timeout) crUtilMutexLock(NULL, NULL, mutex, mask, flags, timeout)
#define crUtilMutexLock3(mutex, mask) crUtilMutexLock(NULL, NULL, mutex, mask, CR_UTIL_FLAG_NONE, CR_UTIL_WAIT_FOREVER)
void crUtilMutexUnlock(uint32_t *mutex, uint32_t mask);

void crUtilRMutexInit(struct crRMutex_t *rmutex);
int crUtilRMutexLock(struct cr_t *cr, void *result, struct crRMutex_t *rmutex,
        uint32_t flags, uint32_t timeout);
#define crUtilRMutexLock2(rmutex, flags, timeout) crUtilRMutexLock(NULL, NULL, rmutex, flags, timeout)
void crUtilRMutexUnlock(struct crRMutex_t *rmutex);

#define CR_QUEUE_POOL_SIZE(nslots, msize) ((nslots) * (sizeof(struct crQeueuMessageHead_t) + (msize)))
#define CR_QUEUE_POOL(name, nslots, msize) static uint8_t name[CR_QUEUE_POOL_SIZE(nslots, msize)]
void crUtilQueueInit(struct crQueue_t *q, uint8_t *pool, size_t nslots, size_t msize);
int crUtilQueueAdd(struct crQueue_t *q, void *data, size_t size, uint32_t flags, uint32_t timeout);
int crUtilQueueRemove(struct crQueue_t *q, void *data, size_t *size, uint32_t flags, uint32_t timeout);

#endif

