#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "service.h"
#include "telemetry.h"
#include "data_structs.h"

/* service factory prototypes from services.c */
extern void MotionControlHandler(Service*, const ServiceEvent*);
extern void SensorFusionHandler(Service*, const ServiceEvent*);
extern void PathPlannerHandler(Service*, const ServiceEvent*);
extern void TelemetryHandler(Service*, const ServiceEvent*);

/* global service pointers */
static Service* MotionControlService = NULL;
static Service* SensorFusionService = NULL;
static Service* PathPlannerService = NULL;
static Service* TelemetryService = NULL;

/* link path planner to motion service (used in services.c) */
extern Service* g_motion_service;

static volatile int keep_running = 1;
void sigint_handler(int s) { (void)s; keep_running = 0; }

int main(void) {
    signal(SIGINT, sigint_handler);
    printf("PhoenixBot Service Framework starting...\n");

    Telemetry_Init();

    /* Create services:
       max_event_size must accommodate ServiceEvent + largest payload (Pose_t or VelocityCommand_t).
    */
    size_t max_ev = sizeof(ServiceEvent) + sizeof(Pose_t) + 16;

    MotionControlService = service_create("MotionControlService", 8, max_ev, MotionControlHandler);
    SensorFusionService = service_create("SensorFusionService", 8, max_ev, SensorFusionHandler);
    PathPlannerService = service_create("PathPlannerService", 8, max_ev, PathPlannerHandler);
    TelemetryService = service_create("TelemetryService", 4, max_ev, TelemetryHandler);

    if (!MotionControlService || !SensorFusionService || !PathPlannerService || !TelemetryService) {
        fprintf(stderr, "Failed to create services\n");
        return -1;
    }

    /* Link global ref for path->motion */
    g_motion_service = MotionControlService;

    /* Start services (each runs in its own thread) */
    service_start(MotionControlService);
    service_start(SensorFusionService);
    service_start(PathPlannerService);
    service_start(TelemetryService);

    /* Main loop: drive periodic ticks and forward pose messages from sensor to path planner at specified rates */
    const int motor_period_ms = 10;   // 100 Hz
    const int sensor_period_ms = 20;  // 50 Hz
    const int path_period_ms = 100;   // 10 Hz
    const int telemetry_period_ms = 1000; // 1 Hz

    uint64_t t0 = 0;
    uint64_t now;
    uint64_t last_sensor = 0, last_path = 0, last_tele = 0;

    // Simulated pose copy so main can post to path at path rate
    Pose_t sim_pose = {0};

    while (keep_running) {
        usleep(5 * 1000); // small sleep so we don't busy-loop
        now = (uint64_t)time(NULL) * 1000; // coarse ms is fine for this demo

        // SENSOR tick @50Hz -> post EVENT_TICK to SensorFusionService
        static uint64_t acc_sensor = 0;
        acc_sensor += 5;
        if (acc_sensor >= sensor_period_ms) {
            acc_sensor = 0;
            service_post_simple(SensorFusionService, EVENT_TICK);
            // simulate same pose calculation that SensorFusionHandler does (so we can post to path)
            float dt = sensor_period_ms / 1000.0f;
            sim_pose.x += 0.1f * dt * cosf(sim_pose.theta);
            sim_pose.y += 0.1f * dt * sinf(sim_pose.theta);
            sim_pose.theta += 0.01f * dt;
        }

        // PATH planner: post latest pose at 10Hz (path_period_ms)
        static uint64_t acc_path = 0;
        acc_path += 5;
        if (acc_path >= path_period_ms) {
            acc_path = 0;
            ServiceEvent ev = { .type = EVENT_POSE, .payload = &sim_pose, .payload_size = sizeof(sim_pose) };
            service_post(PathPlannerService, &ev);
        }

        // TELEMETRY @1Hz -> post tick
        static uint64_t acc_tele = 0;
        acc_tele += 5;
        if (acc_tele >= telemetry_period_ms) {
            acc_tele = 0;
            service_post_simple(TelemetryService, EVENT_TICK);
        }
    }

    /* Shutdown services */
    printf("\nShutting down services...\n");
    service_stop(TelemetryService);
    service_stop(PathPlannerService);
    service_stop(SensorFusionService);
    service_stop(MotionControlService);

    service_destroy(TelemetryService);
    service_destroy(PathPlannerService);
    service_destroy(SensorFusionService);
    service_destroy(MotionControlService);

    printf("PhoenixBot exited cleanly.\n");
    return 0;
}
