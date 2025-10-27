#include "telemetry.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

static Telemetry_Data_t gTelemetry;
static pthread_mutex_t gTelemetryMutex;

void Telemetry_Init(void) {
    memset(&gTelemetry, 0, sizeof(gTelemetry));
    gTelemetry.battery_voltage = 12.6f;
    pthread_mutex_init(&gTelemetryMutex, NULL);
}

void Telemetry_UpdateCurrent(float current) {
    pthread_mutex_lock(&gTelemetryMutex);
    gTelemetry.current_draw = current;
    if (current > gTelemetry.max_pid_jitter_ms) gTelemetry.max_pid_jitter_ms = current;
    pthread_mutex_unlock(&gTelemetryMutex);
}

void Telemetry_UpdatePose(const Pose_t* p) {
    pthread_mutex_lock(&gTelemetryMutex);
    gTelemetry.pose = *p;
    pthread_mutex_unlock(&gTelemetryMutex);
}

void Telemetry_UpdateBattery(float v) {
    pthread_mutex_lock(&gTelemetryMutex);
    gTelemetry.battery_voltage = v;
    pthread_mutex_unlock(&gTelemetryMutex);
}

void Telemetry_UpdateMinLiDAR(float m) {
    pthread_mutex_lock(&gTelemetryMutex);
    gTelemetry.min_lidar_distance = m;
    pthread_mutex_unlock(&gTelemetryMutex);
}

static uint8_t calculate_xor(const Telemetry_Data_t* t) {
    const uint8_t *p = (const uint8_t*)t;
    size_t n = sizeof(Telemetry_Data_t);
    uint8_t x = 0;
    for (size_t i = 0; i < n - 1; ++i) x ^= p[i];
    return x;
}

void Telemetry_ReadAndPrintPacket(void) {
    Telemetry_Data_t copy;
    pthread_mutex_lock(&gTelemetryMutex);
    copy = gTelemetry;
    copy.sequence_number++;
    copy.checksum = 0;
    copy.checksum = calculate_xor(&copy);
    gTelemetry.sequence_number = copy.sequence_number;
    gTelemetry.checksum = copy.checksum;
    pthread_mutex_unlock(&gTelemetryMutex);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
           tm.tm_hour, tm.tm_min, tm.tm_sec, (unsigned)copy.sequence_number, (unsigned)copy.checksum);
    printf("Pose: x=%.3f y=%.3f th=%.3f | Batt:%.2fV Cur:%.2fA MinLiDAR:%.2fm\n",
           copy.pose.x, copy.pose.y, copy.pose.theta,
           copy.battery_voltage, copy.current_draw, copy.min_lidar_distance);
}
