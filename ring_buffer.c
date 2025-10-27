#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct ring_buffer_t {
    void* buf;
    size_t head, tail, capacity, item_size, count;
    pthread_mutex_t m;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};

ring_buffer_t* ring_create(size_t capacity, size_t item_size) {
    ring_buffer_t* r = malloc(sizeof(ring_buffer_t));
    if (!r) return NULL;
    r->buf = malloc(capacity * item_size);
    if (!r->buf) { free(r); return NULL; }
    r->capacity = capacity;
    r->item_size = item_size;
    r->head = r->tail = r->count = 0;
    pthread_mutex_init(&r->m, NULL);
    pthread_cond_init(&r->not_empty, NULL);
    pthread_cond_init(&r->not_full, NULL);
    return r;
}

void ring_destroy(ring_buffer_t* r) {
    if (!r) return;
    pthread_mutex_destroy(&r->m);
    pthread_cond_destroy(&r->not_empty);
    pthread_cond_destroy(&r->not_full);
    free(r->buf);
    free(r);
}

bool ring_push(ring_buffer_t* r, const void* item) {
    if (!r || !item) return false;
    pthread_mutex_lock(&r->m);
    while (r->count == r->capacity) {
        pthread_cond_wait(&r->not_full, &r->m);
    }
    void* dst = (char*)r->buf + (r->tail * r->item_size);
    memcpy(dst, item, r->item_size);
    r->tail = (r->tail + 1) % r->capacity;
    r->count++;
    pthread_cond_signal(&r->not_empty);
    pthread_mutex_unlock(&r->m);
    return true;
}

bool ring_pop(ring_buffer_t* r, void* out) {
    if (!r || !out) return false;
    pthread_mutex_lock(&r->m);
    while (r->count == 0) {
        pthread_cond_wait(&r->not_empty, &r->m);
    }
    void* src = (char*)r->buf + (r->head * r->item_size);
    memcpy(out, src, r->item_size);
    r->head = (r->head + 1) % r->capacity;
    r->count--;
    pthread_cond_signal(&r->not_full);
    pthread_mutex_unlock(&r->m);
    return true;
}

bool ring_try_pop(ring_buffer_t* r, void* out) {
    if (!r || !out) return false;
    bool ok = false;
    pthread_mutex_lock(&r->m);
    if (r->count > 0) {
        void* src = (char*)r->buf + (r->head * r->item_size);
        memcpy(out, src, r->item_size);
        r->head = (r->head + 1) % r->capacity;
        r->count--;
        pthread_cond_signal(&r->not_full);
        ok = true;
    }
    pthread_mutex_unlock(&r->m);
    return ok;
}

size_t ring_count(ring_buffer_t* r) {
    if (!r) return 0;
    pthread_mutex_lock(&r->m);
    size_t c = r->count;
    pthread_mutex_unlock(&r->m);
    return c;
}
