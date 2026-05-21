#pragma once
#include <cstdint>

struct alignas(4) SensorPacket {
    float    accel_x;
    float    accel_y;
    float    accel_z;
    uint32_t heart_rate;
    uint64_t timestamp_us;
};

static_assert(sizeof(SensorPacket) == 24);
