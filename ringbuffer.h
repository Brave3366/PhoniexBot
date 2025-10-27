#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>
#include <stdbool.h>

typedef struct ring_buffer_t ring_buffer_t;

ring_buffer_t* ring_create(size_t capacity, size_t item_size);
void ring_destroy(ring_buffer_t* r);
bool ring_push(ring_buffer_t* r, const void* item);
bool ring_pop(ring_buffer_t* r, void* out);      // blocks if empty
bool ring_try_pop(ring_buffer_t* r, void* out);  // non-blocking
size_t ring_count(ring_buffer_t* r);

#endif
