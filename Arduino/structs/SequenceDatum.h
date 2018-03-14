#pragma once

// Struct for one SequenceDatum
struct SequenceDatum{
    unsigned int steps;
    unsigned int high_duration;
    unsigned int low_duration;
    unsigned char direction;
    bool motor_on;
};