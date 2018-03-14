#pragma once

#include <IRremote.h>  
#include "Arduino.h"
#include "MotorControl.h"

class RemoteControl
{
private:
    //Singleton instance
    static RemoteControl* _instance;

    //C'tor
    RemoteControl();

    //key result
    decode_results results;

    //IRrecv
    IRrecv irrecv = NULL;

public:
    //Get singleton instance
    static RemoteControl* getInstance();

    //Pointer to MotorControl instance
    MotorControl* MC = NULL;

    //Decode key values
    void TranslateIR();

    //Method to call for new event
    void Recv();

    //Enable interrupt
    void EnableIRIn();

    //Init method for recvPin and remote control type
    void Init(unsigned int recvPin);
};
