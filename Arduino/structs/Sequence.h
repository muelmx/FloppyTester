#pragma once
#include "SequenceDatum.h"

//Meta struct for sequence
struct Sequence{
    unsigned int length;
    unsigned int actual_length;
    unsigned char id;
    unsigned char tracks;
    bool loop;
    bool test;
    SequenceDatum ** data;
};