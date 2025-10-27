#ifndef SERVICE_H
#define SERVICE_H

#include <stddef.h>
#include <stdbool.h>

typedef enum {
    EVENT_NONE = 0,
    EVENT_TICK,
    EVENT_POSE,        // payload: Pose_t
    EVENT_VELOCITY,    // payload: VelocityCommand_t
    EVENT_SHUTDOWN
} ServiceEventType;

typedef struct {
    ServiceEventType type;
    void* payload;      // pointer to payload data (copied into mailbox)
    size_t payload_size;
} ServiceEvent;

typedef struct Service Service;

// create service: name, mailbox capacity and max event size (header+payload), handler function
typedef void (*service_handler_t)(Service* svc, const ServiceEvent* ev);

Service* service_create(const char* name, size_t mailbox_capacity, size_t max_event_size, service_handler_t handler);
void service_destroy(Service* svc);
void service_start(Service* svc);
void service_stop(Service* svc);
bool service_post(Service* svc, const ServiceEvent* ev);
void service_post_simple(Service* svc, ServiceEventType type);

const char* service_name(Service* svc);

#endif
