#pragma once
#include "Arduino.h"
#include "./RemoteControl.h"
#include "./structs/Sequence.h"
#include "./MemoryManager.h"
#include "./MemoryFree.h"
#include "./Config.h"

#define BUFFERSIZE 64

class Com
{
  private:
    //Variables
    static Com* _instance;

    //Variables for reading and parsing commands
    char *_buffer;
    char _read_index;
    char _selected_sequence;
    char _command_valid;

    //Temporary sequence
    Sequence *_tmp_sequence;

    //Read data from serial
    void Read();

    //Parse data and invoke 
    void Parse();

    //Constructor
    Com();

  public:
    //Get singleton instance
    static Com* getInstance();

    //Basic destructor
    ~Com();

    //Call this frequently
    void Update();
};