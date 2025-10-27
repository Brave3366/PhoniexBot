#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "data_structs.h"

void Telemetry_Init(void);
void Telemetry_UpdateCurrent(float current);
void Telemetry_UpdatePose(const Pose_t* p);
void Telemetry_UpdateBattery(float v);
void Telemetry_UpdateMinLiDAR(float m);
void Telemetry_ReadAndPrintPacket(void);

#endif
