#include "Com.h"

Com *Com::_instance = NULL;

Com::Com()
{
    _selected_sequence = 0;
    _read_index = 0;
    _tmp_sequence = NULL;
    this->_buffer = (char *)calloc(BUFFERSIZE, sizeof(char));

    // Fill Buffer with invalid characters, so we have a defined state
    for (int i = 0; i < BUFFERSIZE; i++)
    {
        this->_buffer[i] = 'a';
    }

    _command_valid = false;
    if (_buffer == NULL)
    {
        Serial.println(F("Allocation of com buffer failed"));
    }
}

//Implement as Singleton
Com *Com::getInstance()
{
    if (_instance == NULL)
    {
        Com::_instance = new Com();
    }
    return Com::_instance;
}

Com::~Com()
{
    delete _buffer;
}

// Read and save *,* seperated int values from char*. A maximum of 16 values can be read with 5 digits each
// based on https://stackoverflow.com/questions/9072320/split-string-into-string-array
// uses only static memory
int getCommandValues(char *data, int result[])
{
    int strIndex[] = {0, -1};
    int i = 0, j = 0, k = 0, length = 0, found = 0;
    for (; data[i] != '\0'; i++) //Count chars
    {
    }
    length = i;

    int maxIndex = length - 1;

    char buffer[6]; //max 5 digits + null character

    for (i = 0; i <= maxIndex; i++)
    {
        if (data[i] == ',' || i == maxIndex)
        {
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;

            for (j = strIndex[0], k = 0; j < strIndex[1] && k < 16; j++, k++)
            {
                buffer[k] = data[j];
            }
            buffer[k] = '\0';
            result[found] = strtoul(buffer, NULL, 10);
            found++;
        }
    }
}

// Parse buffer
void Com::Parse()
{
    //Parse command to integer values
    unsigned int values[16] = {0};
    getCommandValues(this->_buffer, values);

    int command = values[0];

    Serial.print(F("cmd: "));
    Serial.print(command);
    Serial.print(F(" content: "));
    Serial.println(this->_buffer);

    switch (command)
    {
    case 0: //SELECT SEQUENCE
        _selected_sequence = values[1];
        MotorControl::getInstance()->GetSequence(_selected_sequence);
        break;
    case 1: //New Sequence
        //Delete old temporary sequence
        if (_tmp_sequence != NULL)
        {
            MemoryManager::getInstance()->DeleteSequence(_tmp_sequence);
            _tmp_sequence = NULL;
        }
        //Delete sequence in memory at that index
        MemoryManager::getInstance()->DeleteSequence(values[1]); //Delete Sequence at index
        
        //Allocate sequence
        _tmp_sequence = MemoryManager::getInstance()->AllocateSequence(values[2]);
        _tmp_sequence->id = values[1];
        _tmp_sequence->test = (bool)(values[4] > 0);
        _tmp_sequence->loop = _tmp_sequence->test > 0 ? 0 : values[3]; //Loop is not allowed in test sequences, test wins over loop
        _tmp_sequence->tracks = values[5] != "" ? values[5] : 40;      //Fallback if Trackvalue not assigned
        break;
    case 2: //New Sequence Datum
        if (_tmp_sequence == NULL)
        {
            Serial.println(F("Tmp sequence not set, ignoring"));
            break;
        }
        //Assign values
        _tmp_sequence->data[_tmp_sequence->actual_length]->steps = values[1];
        _tmp_sequence->data[_tmp_sequence->actual_length]->high_duration = values[2];
        _tmp_sequence->data[_tmp_sequence->actual_length]->low_duration = values[3];
        _tmp_sequence->data[_tmp_sequence->actual_length]->direction = _tmp_sequence->loop > 0 ? 0 : (char)values[4]; //If loop only auto direction is allowed
        _tmp_sequence->data[_tmp_sequence->actual_length]->motor_on = (bool)values[5] > 0;
        _tmp_sequence->actual_length++;
        break;
    case 3: //Save Sequence
        MemoryManager::getInstance()->SaveSequence(_tmp_sequence, values[1] > 0);
        _tmp_sequence = NULL; //Delete pointer to tmp sequence to prevent deletion on New Sequence command
        break;
    case 4: // Play Sequence
        MotorControl::getInstance()->GetSequence(values[1]);
        break;
    case 5: // Restore
        MotorControl::getInstance()->ResetAndRestore();
        break;
    case 6: // Motor On/Off
        if (values[1] > 0)
            MotorControl::getInstance()->MotorOn();
        else
            MotorControl::getInstance()->MotorOff();
        break;
    case 7: // Instant playback
        MemoryManager::getInstance()->instantSequence->data[0]->steps = values[1];
        MemoryManager::getInstance()->instantSequence->data[0]->high_duration = values[2];
        MemoryManager::getInstance()->instantSequence->data[0]->low_duration = values[3];
        MemoryManager::getInstance()->instantSequence->data[0]->direction = 0; //If loop only auto direction is allowed
        MemoryManager::getInstance()->instantSequence->data[0]->motor_on = 0;
        MemoryManager::getInstance()->instantSequence->tracks = values[4] > 0 ? values[4] : MemoryManager::getInstance()->instantSequence->tracks;
        MotorControl::getInstance()->Play(MemoryManager::getInstance()->instantSequence);
        break;
    case 8:
        //Print Error Report
        Serial.print(F("+Step Difference:"));
        Serial.print(MemoryManager::getInstance()->report->step_difference);
        Serial.print(F(" Max Position Reached:"));
        Serial.print(MemoryManager::getInstance()->report->max_position);
        Serial.print(F(" Min Position Reached:"));
        Serial.print(MemoryManager::getInstance()->report->min_position);
        Serial.println(F(";"));
        break;
    case 9: //Reset all
        MemoryManager::getInstance()->ResetAll();
    default:
        // Invalid command id, we simply ignore that
        break;
    }
}

void Com::Read()
{
    if (Serial.available() < 1)
        return;

    if (Serial.peek() == '+')
    { //New command available, since commands begin with '+'
        this->_read_index = 0;

        // Consider command as valid, since it began with the start character
        this->_command_valid = 1;

        // Delete Ssart character
        Serial.read();
    }

    // Read Data to buffer
    while (Serial.available() > 0 && this->_read_index < BUFFERSIZE)
    {
        char next = Serial.peek();
        //Null Characters are invalid
        if (next == '\0')
        {
            // Delete null character
            Serial.read();
            Serial.println(F("Invalid null character"));
            continue;
        }

        //We lost a semikolon somewhere, current command is therefore invalid
        if (next == '+')
        {
            this->_read_index = 0;
            this->_command_valid = 0;
            return;
        }

        //Read serial data to buffer
        this->_buffer[this->_read_index] = Serial.read();
        if (this->_buffer[this->_read_index] == ';') //If command complete stop reading
        {
            // Command is complete
            this->_buffer[this->_read_index] = '\0'; //Terminate Array with 0 Character, replace ;
            break;
        }
        this->_read_index++;
    }

    if (this->_buffer[this->_read_index] == '\0') //Check if full command in buffer
    {

        if (this->_command_valid > 0) //Parse only valid commands
        {
            Parse();
        }
        else
        {
            Serial.println(F("Command invalid"));
            for (int i = 0; i < this->_read_index; i++)
            {
                Serial.write(this->_buffer[i]);
            }
            Serial.println(" ");
        }

        // Fill buffer with invalid characters to ensure a defined state
        for (int i = 0; i < this->_read_index + 1; i++)
        {
            this->_buffer[i] = 'a';
        }

        //Reset state
        this->_command_valid = 0;
        this->_read_index = 0;
    }
    else
    {
        // Incomplete command in buffer, keep waiting
    }
}

void Com::Update()
{
    Read();
}
