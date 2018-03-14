#pragma once
#include <Arduino.h>
#include "Sequence.h"

//Resolution of Timer1
#define RES 50 // equals Period time of 100us -> 10000Hz
#define ERRPIN 3
#define STPPIN 4
#define DIRPIN 5
#define MONPIN 6
#define TRKPIN 7

enum Direction {AUTOMATIC = 0, INWARD = 1, OUTWARD = 2};

struct Floppy
{
    //Control arrays for two floppies
    unsigned char MAX_POSITION;
    int currentPosition = 0;
    volatile byte currentState = LOW;
    unsigned long currentTick = 0;
    unsigned int currentSteps = 0;
    Direction currentDirection;

    //Sequence data
    Sequence* sequence;
    unsigned int dataidx = 0;

    //Run sequence
    bool run = false;
    bool next = true;

    //Rest after test
    bool resetAndRestoreAfterTest = false;

    //MIN or MAX Position reached
    bool min_position = false;
    bool max_position = false;
};