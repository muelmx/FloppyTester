#include <TimerOne.h>
#include "Com.h"
#include "MotorControl.h"

MotorControl * MotorControl::_instance = NULL;

 MotorControl::MotorControl()
 {
    f = new Floppy();
 }

MotorControl::~MotorControl()
{
    delete f;
    f=NULL;
}

//Implement as Singleton
static MotorControl * MotorControl::getInstance(){
    if(_instance == NULL){
        MotorControl::_instance = new MotorControl();
    }
    return MotorControl::_instance;
}

void MotorControl::Init()
{
    //Pin meta for Floppy
    pinMode(STPPIN, OUTPUT); // Step control 
    pinMode(DIRPIN, OUTPUT); // Direction 
    pinMode(MONPIN, OUTPUT); // MON 
    pinMode(TRKPIN, INPUT_PULLUP); // TRK00 

    pinMode(ERRPIN, OUTPUT); //Error

    //Back to TRK00
    InitialRestore();

    //Default motor off
    MotorOff();

    //Init Timer1
    // Set up a timer at the defined resolution
    Timer1.initialize(RES); 
    // Attach the Tick method
    Timer1.attachInterrupt(Tick); 
}

static void MotorControl::Tick()
{
    Floppy* f = MotorControl::getInstance()->f;

    //Handle sequence data
    if (f->run && f->sequence != NULL)
    {
        if (f->currentState == LOW)
        {  
            f->currentTick++;

            //Low duration over?
            //Resolution 50us -> duration in ms -> every tick after 50us -> duration has to be multiplicated with 2
            if (f->currentTick >= f->sequence->data[f->dataidx]->low_duration*2 || f->next)
            {
                f->next = false;
                f->currentSteps++;
                
                //Done with a data sequence?
                if (f->currentSteps > f->sequence->data[f->dataidx]->steps)
                {
                    f->currentSteps = 0;
                    f->dataidx++;

                    //Done with all sequences?
                    if(f->dataidx < f->sequence->length)
                    {
                        f->next = true;
                        MotorControl::_instance->GetMotorStateOfNextSequence();
                    }
                    else
                    {
                        //Loop?
                        if(f->sequence->loop)
                        {
                            MotorControl::_instance->Loop();
                        }
                        //Test?
                        else if(f->sequence->test)
                        {
                            f->resetAndRestoreAfterTest = true;
                        }
                        //No Loop, no Test -> Stay at actual position, i.e. no Restore
                        else
                        {
                            MotorControl::_instance->ResetActualSequence();
                        }
                    }
                }
                else
                {
                    //Do a step only if there is high duration
                    if (f->sequence->data[f->dataidx]->high_duration > 0)
                    {
                        MotorControl::_instance->HighImpulse();

                        //Automatic, i.e. no Test, i.e. synchronize currentPosition when TRK00 is reached
                        if(digitalRead(TRKPIN) == LOW && f->sequence->data[f->dataidx]->direction == AUTOMATIC && !f->sequence->test)
                            f->currentPosition = 0;
                    }
                }
                
                f->currentTick=0;
            }
        }
        else
        {
            f->currentTick++;

            //High duration over?
            //Resolution 50us -> duration in ms -> every tick after 50us -> duration has to be multiplicated with 2
            if (f->currentTick >= f->sequence->data[f->dataidx]->high_duration*2)
            {
                if (f->sequence->data[f->dataidx]->low_duration > 0)
                    MotorControl::_instance->LowImpulse();

                f->currentState = LOW;
                f->currentTick=0;
            }
        }
    }   
}

void MotorControl::HighImpulse()
{
    //Proof direction
    if (f->sequence->data[f->dataidx]->direction == INWARD)
    {
        digitalWrite(DIRPIN,LOW);

        //Max position reached? -> stop and error light true
        if (f->currentPosition >= f->MAX_POSITION)
        {
            f->currentState = LOW;
            f->run = false;

            f->max_position = true;

            Serial.println(F(""));
            Serial.println(F("Error: Maximum position reached"));
            
            digitalWrite(ERRPIN,HIGH);

            f->resetAndRestoreAfterTest = true;
        }
        else
        {
            //Make step
            f->currentPosition++;
            f->currentState = HIGH;
            digitalWrite(STPPIN,LOW);
        }
    }
    else if (f->sequence->data[f->dataidx]->direction == OUTWARD)
    {
        digitalWrite(DIRPIN,HIGH);

        //Min position reached? -> stop and error light true
        if (f->currentPosition <= 0 || digitalRead(TRKPIN) == LOW)
        {
            f->currentState = LOW;
            f->run = false;

            f->min_position = true;

            Serial.println(F("Error: Minimum position reached"));
            Serial.println(F(""));

            digitalWrite(ERRPIN,HIGH);

            f->resetAndRestoreAfterTest = true;
        }
        else
        {
            //Make step
            f->currentPosition--;
            f->currentState = HIGH;
            digitalWrite(STPPIN,LOW);
        }
    }
    else if (f->sequence->data[f->dataidx]->direction == AUTOMATIC)
    {
        //Set direction automatic if min or max position is reached
        if (f->currentPosition >= f->MAX_POSITION)
        { 
            f->currentDirection = OUTWARD;
            digitalWrite(DIRPIN,HIGH);
        }
        
        else if (f->currentPosition <= 0)
        { 
            f->currentDirection = INWARD;
            digitalWrite(DIRPIN,LOW);
        }

        //Set current position
        if (f->currentDirection == OUTWARD)
        {
            f->currentPosition--;
        } 
        else 
        {
            f->currentPosition++;
        }

        //Make a step
        f->currentState = HIGH;
        digitalWrite(STPPIN,LOW);
    }
}

void MotorControl::LowImpulse()
{
    digitalWrite(STPPIN,HIGH);
}

void MotorControl::GetMotorStateOfNextSequence()
{
    //Set the Motor Pin
    digitalWrite(MONPIN, !f->sequence->data[f->dataidx]->motor_on);
}

void MotorControl::GetSequence(int num)
{
    //Stop actual sequence to overwrite it
    if(f->sequence != NULL)
    {
        if(!f->sequence->test)
        {
            ResetActualSequence();
            delay(50);
        }
    }

    //Get a sequence
    if (!f->run)
    {
        //Reset red error light
        digitalWrite(ERRPIN, LOW);

        //Get sequence and calculate MAX_POSITION depending on floppy type
        f->sequence = MemoryManager::getInstance()->GetSequence(num);
        
        if (f->sequence != NULL)
        {
            //If actual sequence is a test, reset all before moving on
            if(f->sequence->test)
            {
                ResetAndRestore();
            }

            //Calculate MAX_POSITION 
            f->MAX_POSITION = f->sequence->tracks;

            //Set Motor pin for next sequence
            GetMotorStateOfNextSequence();
            f->run = true;
        }
        else
        {
            Serial.print(F("No sequence data for index "));
            Serial.println(num);
        }
    }
    else
    {
        Serial.println(F("Wait until actual test is done..."));
    }
}

void MotorControl::InitialRestore()
{
    //Direction outwards
    digitalWrite(DIRPIN, HIGH);

    //Step until TRK00 is set
    int i = 0;
    while(digitalRead(TRKPIN) == HIGH && i < 80)
    {
        delay(10);
        digitalWrite(STPPIN, LOW);

        delay(10);
        digitalWrite(STPPIN, HIGH);
        i++;
    }

    digitalWrite(ERRPIN, LOW);
}

void MotorControl::ResetActualSequence()
{
    f->run = false;
    
    //Reset state variables
    f->dataidx = 0;
    f->next = true;
    f->currentState = LOW;
    f->currentTick = 0;
}

void MotorControl::ResetAndRestore()
{
    f->run = false;

    //Back to TRK00, Direction outwards
    digitalWrite(DIRPIN, HIGH);

    //Make a step until TRK00 is set
    int i = 0;
    while(digitalRead(TRKPIN) == HIGH && i < 80)
    {
        delay(50);
        digitalWrite(STPPIN, LOW);

        delay(50);
        digitalWrite(STPPIN, HIGH);
        i++;

        delay(50);
    }

    //Reset Error LED
    digitalWrite(ERRPIN, LOW);

    //Reset state variables
    f->dataidx = 0;
    f->next = true;
    f->currentState = LOW;
    f->currentTick = 0;

    f->currentPosition = 0;
    f->currentSteps = 0;
    f->currentDirection = INWARD;
}

void MotorControl::ResetAndRestoreAfterTest()
{
    if(!f->resetAndRestoreAfterTest)
        return;

    f->run = false;

    //Back to TRK00, Direction outwards
    digitalWrite(DIRPIN, HIGH);

    Serial.print(F("CurrentPosition = "));
    Serial.println(f->currentPosition);

    //Make a step until TRK00 is set
    int i = 0;
    while(digitalRead(TRKPIN) == HIGH && i < 80)
    {
        delay(50);
        digitalWrite(STPPIN, LOW);

        delay(50);
        digitalWrite(STPPIN, HIGH);

        f->currentPosition--;
        i++;

        delay(50);
    }

    //Set/Clear Error LED
    if (f->currentPosition != 0 || f->min_position || f->max_position)
        digitalWrite(ERRPIN, HIGH);
    else
        digitalWrite(ERRPIN, LOW);

    //Fill error protocol
    MemoryManager::getInstance()->report->step_difference = f->currentPosition;
    MemoryManager::getInstance()->report->min_position = f->min_position;
    MemoryManager::getInstance()->report->max_position = f->max_position;
    MemoryManager::getInstance()->SaveErrorReportToEEPROM();

    Serial.print(F("CurrentPosition after Restore = "));
    Serial.println(f->currentPosition);

    //Reset state variables
    f->dataidx = 0;
    f->next = true;
    f->currentState = LOW;
    f->currentTick = 0;

    f->currentPosition = 0;
    f->currentSteps = 0;
    f->currentDirection = INWARD;

    f->min_position = false;
    f->max_position = false;

    f->sequence = NULL;

    f->resetAndRestoreAfterTest = false;
}

void MotorControl::Loop()
{
    //Reset state variables to play in loop
    f->run = false;

    f->dataidx = 0;
    f->next = true;
    f->currentTick = 0;
    f->currentState = LOW;

    f->run = true;
}

void MotorControl::Play()
{
    //Play sequence
    if (f->sequence != NULL)
    {
        f->run = true;
    }
}

void MotorControl::Play(Sequence* s)
{
    //Play a given sequence
    f->run = true;
    f->sequence = NULL;
    f->sequence = s;

    f->dataidx = 0;
    f->next = true;
    f->currentTick = 0;
    f->currentState = LOW;

    if(f->sequence != NULL)
        f->run = true;
    else
        Serial.println(F("No sequence data selected!"));
}

void MotorControl::Pause()
{
    //Pause sequence
    if (f->sequence != NULL)
    {
        f->run = false;
    }
}

void MotorControl::PlayPause()
{
    //Toggle play/pause sequence
    if (f->sequence != NULL && !f->sequence->test)
    {        
        f->run = !f->run;
    }
}

void MotorControl::MotorOn()
{
    digitalWrite(MONPIN, LOW);
}

void MotorControl::MotorOff()
{
    digitalWrite(MONPIN, HIGH);
}

void MotorControl::StartTimer()
{
    Timer1.attachInterrupt(Tick); 
}

void MotorControl::StopTimer()
{
    Timer1.detachInterrupt(); 
}