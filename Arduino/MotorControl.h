#pragma once

#include <Arduino.h>
#include "structs/Floppy.h"

class MotorControl
{
private:
    //Singleton instance
    static MotorControl * _instance;

    //C'tor
    MotorControl();

    //D'tor
    ~MotorControl();

    //Floppy
    Floppy* f = NULL;

    //Restore -> back to TRK00
    void RestoreAfterTest();

    //Initial Restore after upload/start
    void InitialRestore();

public:
    //Implement as Singleton
    static MotorControl * getInstance();

    //Init Timer1 and global variables
    void Init();

    //Function attached to interrupt
    static void Tick();

    //Make a step
    void HighImpulse();

    //No step -> just for LED to turn off
    void LowImpulse();

    //Next datum in sequence
    void GetMotorStateOfNextSequence();

    //Get a selected sequence via IRremote
    void GetSequence(int num);

    //Play and pause sequence
    void PlayPause();

    //Reset and Restore without currentPosition output
    void ResetAndRestore();

    //Reset and Restore after a test is done with currentPosition output
    void ResetAndRestoreAfterTest();

    //Reset variables information after a sequence is done
    void ResetActualSequence();

    //Start actual sequence again
    void Loop();

    //Play sequence
    void Play();

    //Play specific sequence
    void Play(Sequence* s);

    //Pause sequence
    void Pause();

    //Turn MONPIN on
    void MotorOn();

    //Turn MONPIN off
    void MotorOff();

    //Start Timer
    void StartTimer();

    //Stop Timer
    void StopTimer();
};