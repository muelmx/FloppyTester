#include "RemoteControl.h"
#include "Com.h"
#include "MotorControl.h"

void setup() 
{
    Serial.begin(9600);
    // while (!Serial);

    //Init Remote Control
    unsigned int RECV_PIN = 12;  

    //Init Remote Control and add Motor Control
    RemoteControl::getInstance()->MC = MotorControl::getInstance();
    RemoteControl::getInstance()->Init(RECV_PIN);
    MotorControl::getInstance()->Init();
    MemoryManager::getInstance(); //Get Instance once, to load data from eeprom
}

void loop()
{
    RemoteControl::getInstance()->Recv();
    
    Com::getInstance()->Update();

    MotorControl::getInstance()->ResetAndRestoreAfterTest();
}

