#include "service.h"
#include "telemetry.h"
#include "data_structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <math.h>
#include <unistd.h>

static uint64_t now_ms(void) {
    return (uint64_t)GetTickCount64();
}

//MotionControlService (100 Hz)
void MotionControlHandler(Service* svc, const ServiceEvent* ev) {
    static float last_pid = 0.0f;
    (void)svc;
    if (ev->type == EVENT_VELOCITY) {
        if (ev->payload && ev->payload_size == sizeof(VelocityCommand_t)) {
            VelocityCommand_t cmd;
            memcpy(&cmd, ev->payload, sizeof(cmd));
            float pid = cmd.linear * 10.0f + cmd.angular * 2.0f;
            last_pid = pid;
            Telemetry_UpdateCurrent(pid);
            printf("[MotionControl] cmd: lin=%.3f ang=%.3f -> pid=%.3f\n",
                   cmd.linear, cmd.angular, pid);
            if (pid > 100.0f) {
                printf("[MotionControl] WARNING: pid exceeds safe threshold\n");
            }
        }
    } else if (ev->type == EVENT_SHUTDOWN) {
    }
}

//SensorFusionService (50 Hz)
void SensorFusionHandler(Service* svc, const ServiceEvent* ev) {
    static Pose_t pose = {0};
    (void)svc;
    if (ev->type == EVENT_TICK) {
        float dt = 0.02f; // 50 Hz tick
        float v = 0.1f; // simulated forward speed
        pose.x += v * dt * cosf(pose.theta);
        pose.y += v * dt * sinf(pose.theta);
        pose.theta += 0.01f * dt;
        Telemetry_UpdatePose(&pose);
        Telemetry_UpdateMinLiDAR(1.0f + 0.2f * sinf(pose.theta));
    }
}

//PathPlannerService (10 Hz)
Service* g_motion_service = NULL; // will be set from main to post velocity commands

void PathPlannerHandler(Service* svc, const ServiceEvent* ev) {
    (void)svc;
    if (ev->type == EVENT_POSE && ev->payload && ev->payload_size == sizeof(Pose_t)) {
        Pose_t p;
        memcpy(&p, ev->payload, sizeof(p));
        VelocityCommand_t cmd;
        cmd.linear = 0.2f;
        cmd.angular = 0.1f * (0.5f - sinf(p.theta));
        printf("[PathPlanner] Received pose x=%.3f y=%.3f th=%.3f -> cmd lin=%.3f ang=%.3f\n",
               p.x, p.y, p.theta, cmd.linear, cmd.angular);
        if (g_motion_service) {
            ServiceEvent ev_cmd = { .type = EVENT_VELOCITY, .payload = &cmd, .payload_size = sizeof(cmd) };
            service_post(g_motion_service, &ev_cmd);
        }
    } else if (ev->type == EVENT_SHUTDOWN) {
    }
}

//TelemetryService (1 Hz) 
void TelemetryHandler(Service* svc, const ServiceEvent* ev) {
    (void)svc;
    if (ev->type == EVENT_TICK) {
        Telemetry_ReadAndPrintPacket();
    }
}
