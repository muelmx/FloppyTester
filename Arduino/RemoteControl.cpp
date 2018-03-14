#include "RemoteControl.h"

RemoteControl * RemoteControl::_instance = NULL;

//Implement as Singleton
static RemoteControl* RemoteControl::getInstance(){
    if(_instance == NULL){
        RemoteControl::_instance = new RemoteControl();
    }
    return RemoteControl::_instance;
}

void RemoteControl::Init(unsigned int recvPin)
{
    irrecv = IRrecv(recvPin);
    EnableIRIn();
}

RemoteControl::RemoteControl(){}

void RemoteControl::TranslateIR()
{
    //Serial.println(results.value, HEX);
    switch(results.value)
    {
        case 0xFF10EF:  
        //Left Arrow
        break;

        case 0xFF18E7:  
        //Up Arrow
        break;

        case 0xFF5AA5:  
        //Right Arrow
        break;

        case 0xFF4AB5:  
        //Down Arrow
        break;

        case 0xFF6897:  
        //*
        break;

        case 0xFFB04F:  
        //#
        if (MC != NULL)
            MC->ResetAndRestore();
        break;

        case 0xFF38C7:  
        //OK == Play/Pause
        if (MC != NULL)
            MC->PlayPause();
        break;

        case 0xFF9867:  
        //0
        break;

        case 0xFFA25D:  
        //1
        if (MC != NULL)
            MC->GetSequence(0);
        break;

        case 0xFF629D:  
        //2
        if (MC != NULL)
            MC->GetSequence(1);
        break;

        case 0xFFE21D:  
        //3 
        if (MC != NULL)
            MC->GetSequence(2);
        break;

        case 0xFF22DD:  
        //4
        if (MC != NULL)
            MC->GetSequence(3);
        break;

        case 0xFF02FD:  
        //5
        break;

        case 0xFFC23D:  
        //6
        break;

        case 0xFFE01F:  
        //7
        break;

        case 0xFFA857:  
        //8
        break;

        case 0xFF906F:  
        //9
        break;   
    }
}

void RemoteControl::Recv()
{
    //Decode received values
    if (irrecv.decode(&results)) 
    {
        TranslateIR();
        // Receive the next value
        irrecv.resume(); 
    }
}

void RemoteControl::EnableIRIn()
{
    irrecv.enableIRIn();
}