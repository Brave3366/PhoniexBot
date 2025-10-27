#include "service.h"
#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

struct Service {
    char* name;
    ring_buffer_t* mailbox;
    size_t max_event_size;
    service_handler_t handler;
    pthread_t thread;
    volatile int running;
};

static void* service_thread_func(void* arg) {
    Service* s = (Service*)arg;
    void* block = malloc(s->max_event_size);
    while (s->running) {
        // blocking pop
        if (!ring_pop(s->mailbox, block)) continue;
        // extract header
        ServiceEvent header;
        memcpy(&header, block, sizeof(ServiceEvent));
        ServiceEvent local = { .type = header.type, .payload = NULL, .payload_size = header.payload_size };
        if (local.payload_size > 0) {
            local.payload = malloc(local.payload_size);
            memcpy(local.payload, (char*)block + sizeof(ServiceEvent), local.payload_size);
        }
        // call handler
        if (s->handler) s->handler(s, &local);
        if (local.payload) free(local.payload);
    }
    free(block);
    return NULL;
}

Service* service_create(const char* name, size_t mailbox_capacity, size_t max_event_size, service_handler_t handler) {
    Service* s = malloc(sizeof(Service));
    if (!s) return NULL;
    s->name = strdup(name);
    s->max_event_size = max_event_size;
    s->handler = handler;
    s->mailbox = ring_create(mailbox_capacity, max_event_size);
    s->running = 0;
    return s;
}

void service_destroy(Service* svc) {
    if (!svc) return;
    if (svc->running) service_stop(svc);
    ring_destroy(svc->mailbox);
    free(svc->name);
    free(svc);
}

void service_start(Service* svc) {
    if (!svc || svc->running) return;
    svc->running = 1;
    pthread_create(&svc->thread, NULL, service_thread_func, svc);
}

void service_stop(Service* svc) {
    if (!svc || !svc->running) return;
    svc->running = 0;
    // post a shutdown event to wake thread if blocked
    ServiceEvent ev = { .type = EVENT_SHUTDOWN, .payload = NULL, .payload_size = 0 };
    service_post(svc, &ev);
    pthread_join(svc->thread, NULL);
}

bool service_post(Service* svc, const ServiceEvent* ev) {
    if (!svc || !ev) return false;
    if (sizeof(ServiceEvent) + ev->payload_size > svc->max_event_size) return false;
    void* block = malloc(svc->max_event_size);
    memset(block, 0, svc->max_event_size);
    memcpy(block, ev, sizeof(ServiceEvent));
    if (ev->payload && ev->payload_size > 0) {
        memcpy((char*)block + sizeof(ServiceEvent), ev->payload, ev->payload_size);
    }
    bool ok = ring_push(svc->mailbox, block);
    free(block);
    return ok;
}

void service_post_simple(Service* svc, ServiceEventType type) {
    ServiceEvent ev = { .type = type, .payload = NULL, .payload_size = 0 };
    service_post(svc, &ev);
}

const char* service_name(Service* svc) {
    return svc ? svc->name : NULL;
}
