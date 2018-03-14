#include "MemoryManager.h"

//Memory Manager manages ram and eeprom usage
//since memory management on arduino is a bit tricky, we included a lot of debug messages

Sequence **sequences = NULL;
Sequence *instantSequence = NULL;
MemoryManager *MemoryManager::_instance = NULL;
ErrorReport *report = NULL;

//Delete Sequence by pointer
void MemoryManager::DeleteSequence(Sequence *sequence)
{
    if (sequence == NULL)
        return;

    for (int i = 0; i < sequence->actual_length; i++)
    {
        delete sequence->data[i];
    }
    free(sequence->data);

    Serial.print(F("Deleting sequence: "));
    Serial.println((int)sequence);
    delete sequence;
    Serial.println(F("Successfully deleted sequence"));
}

//Delte Sequence by index
void MemoryManager::DeleteSequence(int index)
{
    DeleteSequence(GetSequence(index));
    sequences[index] = NULL;
}

//Allocate memory for instant sequence (Sequence with just one sequence item)
void MemoryManager::AllocateInstantSequence()
{
    this->instantSequence = this->AllocateSequence(1);
}

//Allocate Data for Sequence here, just to be safe
Sequence *MemoryManager::AllocateSequence(unsigned int length)
{
    // Print some debug memory information
    Serial.print(F("Free Memory: "));
    Serial.println(freeMemory());

    Serial.print(F("Sizeof Meta: "));
    Serial.println(sizeof(*instantSequence)) +
        Serial.print(F("Sizeof Datum: "));
    Serial.println(sizeof(*(instantSequence->data[0])));

    // +4 and + 2 compensate for malloc overhead (empirical value)
    int estimatedSize = length * ((int)sizeof(*(instantSequence->data[0])) + 4) + sizeof(*instantSequence) + 2;

    Serial.print(F("Estimated size: "));
    Serial.println(estimatedSize);

    if (estimatedSize >= freeMemory())
    {
        Serial.println(F("Not enough memory available, try again with a shorter sequence"));
        return NULL;
    }

    // Create Meta Object and assign default values
    auto sequence = new Sequence();
    sequence->id = 0;
    sequence->tracks = 40; //We use 40 tracks by default just to be safe
    sequence->loop = false;
    sequence->test = false;
    sequence->length = length;
    sequence->data = SequenceData;
    sequence->actual_length = 0;

    SequenceDatum **SequenceData = (SequenceDatum **)malloc(length * sizeof(SequenceDatum *));

    if (SequenceData == NULL)
    {
        Serial.println(F("Error allocating sequence memory"));
        return NULL;
    }

    // Pre-Create Objects
    for (int i; i < length; i++)
    {
        SequenceData[i] = new SequenceDatum();
    }

    //Print some debug info
    Serial.print(F("Successfully allocated memory "));
    Serial.println((int)sequence);
    Serial.print(F("Free Memory: "));
    Serial.println(freeMemory());

    return sequence;
}

//Save sequence in memory and if flag is set to eeprom
int MemoryManager::SaveSequence(Sequence *sequence, bool persistent = false)
{
    //Check if id is valid
    if (sequence->id < 0 || sequence->id > NUM_SEQUENCES - 1)
    {
        Serial.println(F("Invalid Id"));
        return 1;
    }

    //Sequence on this index is not empty
    if (sequences[sequence->id] != NULL)
    {
        Serial.print(F("Overwriting Sequence "));
        Serial.println((int)sequences[sequence->id]);
        DeleteSequence(sequences[sequence->id]);
        sequences[sequence->id] = NULL;
    }

    //Check if as much elements have been transferred as expected
    if (sequence->actual_length < sequence->length)
    {
        Serial.println(F("Warning: transfered sequence shorter than specified"));
    }

    Serial.println(F("Successfully saved sequence"));
    sequences[sequence->id] = sequence;

    if (persistent)
        this->SaveToEEPROM(sequence);
    return 0;
}

Sequence *MemoryManager::GetSequence(int index)
{
    if (index < 0 || index > NUM_SEQUENCES - 1)
        return NULL;
    return sequences[index];
}

//Save sequence persistently to eeprom
void MemoryManager::SaveToEEPROM(Sequence *sequence)
{
    //Check if data is valid, only id 0 and 1 are allowed to save to memory
    if (sequence == NULL)
        return;
    if (sequence->id < 0 || sequence->id > 1)
        return;

    //Calculate addresses based on available eeprom
    unsigned int endpos = E2END - ERROR_REPORT_RESERVED_BYTES;
    unsigned int startpos = sequence->id * endpos / 2;
    unsigned int seqspace = startpos + endpos / 2;

    //Print some debug info about neccessary memory size
    Serial.print(F("Size of meta "));
    Serial.println(sizeof(*sequence));
    Serial.print(F("Size of datum "));
    Serial.println(sizeof(*sequence->data[0]));
    
    //Start writing things to eeprom
    Serial.println(F("Write header"));
    startpos += EEPROM_writeAnything(startpos, *sequence);
    Serial.print(F("New Startpos "));
    Serial.println(startpos);

    //Write sequence steps to eeprom
    int i = 0;
    for (; i < sequence->actual_length; i++)
    {
        startpos += EEPROM_writeAnything(startpos, *sequence->data[i]);
        if (startpos + sizeof(*sequence->data[i]) > seqspace)
        {
            Serial.println(F("Could not save full sequence, eeprom too small"));
            break;
        }
    }

    //Print some more info about memory
    Serial.print(F("Wrote "));
    Serial.print(i);
    Serial.print(F(" elements with total size "));
    Serial.println(i * sizeof(*sequence->data[0]) + sizeof(*sequence));
    Serial.print(F("Estimated maximum sequence length: "));
    Serial.println(((endpos / 2) - sizeof(*sequence)) / sizeof(*sequence->data[0]));
}

//Load sequence by index from eeprom
Sequence *MemoryManager::LoadFromEEPROM(unsigned char index)
{
    //Calculate addresses based on available eeprom
    unsigned int endpos = E2END - ERROR_REPORT_RESERVED_BYTES;
    unsigned int startpos = index * endpos / 2;
    unsigned int seqspace = startpos + endpos / 2;

    auto sequence = new Sequence(); //Peek at the memory with empty sequence object

    startpos += EEPROM_readAnything(startpos, *sequence);

    //Sanity checks for sequence. Id and pointer to data array must be valid
    if (sequence->id < 0 || sequence->id > 1)
        return NULL;
    if (sequence->data == NULL)
        return NULL;

    //Use standard memory allocator for sequence data
    Sequence *alloc_sequence = this->AllocateSequence(sequence->length);
    sequence->data = alloc_sequence->data;

    //Delete allocation object, allocated memory is not affected
    delete alloc_sequence;

    //Reat Sequences from eeprom
    int i = 0;
    for (; i < sequence->actual_length; i++)
    {
        startpos += EEPROM_readAnything(startpos, *sequence->data[i]);
    }

    //Print some debug information
    Serial.print(F("Read "));
    Serial.print(i);
    Serial.print(F(" elements with total size "));
    Serial.println(i * sizeof(*sequence->data[0]));

    return sequence;
}

//Load data from eeprom on boot
void MemoryManager::LoadOnBoot()
{
    Serial.println(F("Trying to load Sequence 1 from EEPROM"));
    Sequence *s1 = LoadFromEEPROM(0);
    if (s1 == NULL)
        Serial.println(F("EEPROM S1 Empty"));
    else
        this->SaveSequence(s1);

    Serial.println(F("Trying to load Sequence 2 from EEPROM"));
    Sequence *s2 = LoadFromEEPROM(1);
    if (s2 == NULL)
        Serial.println(F("EEPROM S2 Empty"));
    else
        this->SaveSequence(s2);

    Serial.println(F("Loading error report"));
    LoadReportFromEEPROM();
}

//Write error report data to eeprom
void MemoryManager::SaveErrorReportToEEPROM()
{
    unsigned int pos = E2END - ERROR_REPORT_RESERVED_BYTES + 1;
    EEPROM_writeAnything(pos, *report);
}

//Load error report data from eeprom
void MemoryManager::LoadReportFromEEPROM()
{
    unsigned int pos = E2END - ERROR_REPORT_RESERVED_BYTES + 1;
    EEPROM_readAnything(pos, *report);
    Serial.println(F("Last Step diff"));
    Serial.println(report->step_difference);
}

//Allocate error report object
void MemoryManager::AllocateErrorReport()
{
    this->report = new ErrorReport();
}

//Delete/overwrite everything
void MemoryManager::ResetAll()
{
    // Calculate positions in memory
    unsigned int endpos = E2END - ERROR_REPORT_RESERVED_BYTES;

    // Generate empty sequence to overwrite eeprom
    Sequence *empty_seq = new Sequence();
    empty_seq->actual_length = 0;

    EEPROM_writeAnything(0, *empty_seq);
    EEPROM_writeAnything(endpos / 2, *empty_seq);

    // Overwrite error report with zero values
    this->report->step_difference = 0;
    this->report->max_position = false;
    this->report->min_position = false;
    this->SaveErrorReportToEEPROM();

    // Delete sequences in memory
    for (int i = 0; i < 4; i++)
    {
        DeleteSequence(i);
    }
}

//Implement as Singleton
static MemoryManager *MemoryManager::getInstance()
{
    // Allocate memory on boot
    if (_instance == NULL)
    {
        MemoryManager::_instance = new MemoryManager();
        MemoryManager::_instance->AllocateInstantSequence();
        MemoryManager::_instance->AllocateErrorReport();
        MemoryManager::_instance->LoadOnBoot();
    }
    return MemoryManager::_instance;
}

//Static Class
MemoryManager::MemoryManager()
{
    sequences = (Sequence **)calloc(NUM_SEQUENCES, sizeof(Sequence *));
    if (sequences == NULL)
    {
        Serial.println(F("Allocation of sequences array failed"));
    }
};