#pragma once
#include <Arduino.h>
#include "structs/Sequence.h"
#include "structs/ErrorReport.h"
#include "EEPromAnything.h"
#include "MemoryFree.h"

#define NUM_SEQUENCES 4
#define ERROR_REPORT_RESERVED_BYTES 32

class MemoryManager
{
  public:
    //Delete Sequence by index
    // void DeleteSequence(int index);

    //Delete Sequence by sequence pointer
    void DeleteSequence(Sequence * sequence);

        //Delete Sequence by sequence pointer
    void DeleteSequence(int index);

    //Allocate Memory for instant sequence
    void AllocateInstantSequence();

    //Allocate Memory for sequence
    Sequence* AllocateSequence(unsigned int length);

    //Allocate Memory for error report
    void AllocateErrorReport();

    //Get Sequence by Index
    Sequence* GetSequence(int index);

    //Save Sequence
    int SaveSequence(Sequence * sequence, bool persistent = false);

    //Reset All
    void ResetAll();

    //Implement as Singleton
    static MemoryManager * getInstance();
    
    //Instant Sequence //ToDo: Refactor!
    Sequence * instantSequence;

    //ErrorReport
    ErrorReport * report;

    void SaveErrorReportToEEPROM();
  private:
    Sequence ** sequences;
    
    static MemoryManager * _instance;

    //Save sequence to persistent eeprom
    void SaveToEEPROM(Sequence * sequence);

    //Load sequence from eeprom
    Sequence * LoadFromEEPROM(unsigned char index);

    void LoadReportFromEEPROM();

    //Load sequences and error report from eeprom on startup
    void LoadOnBoot();

    //Static Class
    MemoryManager();
};