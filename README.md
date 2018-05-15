# Floppy Tester

*Floppy Tester* is a tool to save and run predefined sequences to *Arduino*-compatible microcontrollers in order to test the Floppy-Drive's stepper and spindle motors. You can also play *midi* files for fun.
This library is able to store data transiently and persistently on an Arduino. Thus, it is possible to play sequences without connection to a computer. The application consists of software for Arduino compatible microcontrollers and a Python command line interface.

A project of 
[Joachim Kist](https://github.com/achim92),
[Jonas Herzog](https://github.com/JonasHerzog),
[Maximilian Müller](https://github.com/muelmx)



----

## Table of Contents
- [Floppy Tester](#floppy-tester)
    - [Table of Contents](#table-of-contents)
    - [Features](#features)
        - [Sequences](#sequences)
        - [Test Sequences and Reports](#test-sequences-and-reports)
        - [Playing Midi Files](#playing-midi-files)
        - [Saving and Restoring Data](#saving-and-restoring-data)
        - [Remote Control](#remote-control)
    - [Installing Dependencies](#installing-dependencies)
        - [Python](#python)
        - [Arduino](#arduino)
- [Usage](#usage)
    - [Wiring things up](#wiring-things-up)
        - [Arduino Connections](#arduino-connections)
        - [IDE / Floppy Cable Connections](#ide-floppy-cable-connections)
    - [Console App](#console-app)
        - [Navigation](#navigation)
        - [Establish Connection](#establish-connection)
        - [Main Menu](#main-menu)
            - [Submenu Add Teststeps](#submenu-add-teststeps)
            - [Edit Teststeps](#edit-teststeps)
            - [Show Sequences](#show-sequences)
            - [Save Sequences as JSON / Load JSON](#save-sequences-as-json-load-json)
            - [Send Sequences to Arduino](#send-sequences-to-arduino)
            - [Receive Test Log](#receive-test-log)
            - [Load Midi](#load-midi)
            - [Reset Memory](#reset-memory)
            - [Print/Hide Debug Information](#printhide-debug-information)
            - [Clear Console](#clear-console)
            - [Exit](#exit)
    - [Remote Control](#remote-control)
        - [Playing Sequences](#playing-sequences)
        - [Pause/Resume a Sequence](#pauseresume-a-sequence)
        - [Cancel a Sequence](#cancel-a-sequence)
    - [Serial Interface](#serial-interface)
        - [Commands:](#commands)
            - [Select Sequence](#select-sequence)
            - [New Sequence](#new-sequence)
            - [New Sequence Datum](#new-sequence-datum)
            - [Save Sequence](#save-sequence)
            - [Play Sequence](#play-sequence)
            - [Restore](#restore)
            - [Motor On/Off](#motor-onoff)
            - [Instant Playback](#instant-playback)
            - [Receive Error Log](#receive-error-log)
            - [Delete all Data including EEPROM](#delete-all-data-including-eeprom)
    - [Examples](#examples)
        - [Play Sequence](#play-sequence)
        - [Test Sequence](#test-sequence)
        - [Receive Error Report / Delete Data](#receive-error-report-delete-data)
- [About](#about)
    - [Arduino Code](#arduino-code)
        - [Structs](#structs)
            - [Floppy](#floppy)
            - [Sequence](#sequence)
            - [SequenceDatum](#sequencedatum)
            - [ErrorReport](#errorreport)
        - [Com](#com)
        - [Memory Manager](#memory-manager)
        - [Motor Control](#motor-control)
        - [Remote Control](#remote-control)
        - [Main](#main)
        - [External Libraries](#external-libraries)
    - [Python Code (CLI)](#python-code-cli)
        - [Data Structures](#data-structures)
        - [JSON Files / Serializing](#json-files-serializing)
        - [Serial Connection](#serial-connection)
        - [Midi Parser](#midi-parser)
        - [External Libraries](#external-libraries)


## Features

- Define step sequences by frequencies, periods and duty cycles.
- Save, load and edit sequences from `.json` files.
- Play sequences endlessly with automatic direction changes of the stepper motor.
- Create test sequences to check the correct operation of the stepper motor.
- Get error reports that show if all steps have been performed correctly.
- Play midi files with automatic frequency adaption.
- The functionality can be controlled via a remote control.
- Manually control the functionality via the serial interface.
- Maximum output frequency: **5kHz**
- Timer resolution: **0.1ms**

### Sequences

With the help of the Floppy Tester, sequences of different step frequencies can be defined. Such are called `sequences`. An example sequence may look like this:

    Sequence 1: Id = 1, 2 Elements, Loop = false, Test = true, Tracks = 80
    40 Steps, High Duration 200, Low Duration 250, Direction In, Motor On true
    10 Steps, High Duration 100, Low Duration 150, Direction Out, Motor On false

The sequence can contain any number of step frequencies, limited just by the amount of available memory. The number of steps, the high and low phase, the direction and the status of the motor can be defined inside a so-called `sequence datum`. By specifying the number of tracks different kinds of floppy drives can be used. 

It is also possible to transfer pauses as sequence steps. For this, the high period must be set to 0. The combination of number of steps and the low period gives the pause time.

A sequence can be defined as a test sequence (see [next section](#test-sequences-and-reports)). If the loop flag is activated, the sequence will be played in an endless loop until the user cancels it, for example by Remote Control.

> **Note:** The case *Loop = true* and *Test = true* can not occur, i.e. a test sequence can not be played in an endless loop. Here, the test flag outweighs the loop flag.


### Test Sequences and Reports

A sequence can be configured as a test sequence, i.e. after completing a test sequence, a report can be read from the Arduino. The error log is read out via the Console App. Before a test sequence is started, the floppy stepper is restored, i.e. set to step position 0. The error log considers three error cases:

1. Deviation of the current step position
    
    > The current step position of the floppy stepper is followed by the program of the floppy tester. When a test sequence has been completed, the floppy stepper is restored, i.e. the floppy stepper returns to its original position (0). The test was successful if the current step position equals zero. If this is not the case, the floppy stepper behaves incorrectly or the maximum possible step frequency of the floppy stepper has been exceeded and steps have been skipped. This is indicated by the current step position unequal to zero and by the red LED in the floppy tester setup.

2. Minimum position reached
    
    > If a sequence has been defined so that the minimum step position (0) has been undershot (i.e. `TRK00` is reached but the current step position is unequal to zero), this is indicated in the error log and by the red LED in the floppy tester setup.

3. Maximum position reached
    
    > If a sequence has been defined to exceed the maximum step position (40 or 80 tracks), this will be indicated in the error log and by the red LED in the floppy tester setup. Since there is no sensor, the end position can only be set based on the counted steps. If there is a step difference here, the flag may be set although the read head does not touch the end. This can be detected by clever design of the test sequence and evaluation of the step difference.

### Playing Midi Files

It is possible to transfer and play `.mid` files by selecting the corresponding option of the command line interface. If the midi contains more than one channel you are asked to select one. Due to memory limitations, the midi is cropped automatically to the first 50 Notes/Pauses. 

### Saving and Restoring Data

Sequences with Index *0* and *1* (number 1 and 2) can be saved persistently to the Arduino's EEPROM. Different Arduinos might have different amounts of available EEPROM and it can not be guaranteed that it's possible to save whole sequences. The first two sequences can therefore each use a maximum of half of the available EEPROM. For an *Arduino Uno* this equals to around 60 steps per sequence.

### Remote Control

The functionality of the floppy tester can be handled by a *Remote Control*. Once a sequence is defined and transferred to the Arduino with the console application or via serial interface, the sequence is saved on the Arduino and it is possible to repeat a sequence by using the Remote Control. This allows the floppy tester to be used without a serial connection to a computer. The floppy drive can also be restored.

## Installing Dependencies

### Python
Install dependencies with elevated rights (superuser/administrator)

- [Pyserial](https://pythonhosted.org/pyserial/)
    
        pip install pyserial
- [Mido](https://mido.readthedocs.io/en/latest/)
    
        pip install mido

### Arduino

The necessary `TimerOne` and `IRremote` packages can be found and installed through the graphical interface of the Arduino development environment.

# Usage

## Wiring things up

### Arduino Connections

> PIN `3`: Error LED (*Output*)

> PIN `4`: STP (*Output*)

> PIN `5`: DIR (*Output*)

> PIN `6`: MON (*Output*)

> PIN `7`: TRK00 (*Output*)

> PIN `12`: Recv Remote Control (*Input*)

### IDE / Floppy Cable Connections

|   | 34 | 32 | 30 | 28 |   26  | 24 | 22 |  20 |  18 |      16      |      14      | 12 |  10 | 8 | 6 | 4 |  2  |
|---|:--:|:--:|:--:|:--:|:-----:|:--:|:--:|:---:|:---:|:------------:|:------------:|:--:|:---:|:-:|:-:|:-:|:---:|
| A |    |    |    |    | TRK00 |    |    | STP | DIR |              | Bridge to 15 |    | MON |   |   |   |     |
|   | 33 | 31 | 29 | 27 |   25  | 23 | 21 |  19 |  17 |      15      |      13      | 11 |  9  | 7 | 5 | 3 |  1  |
| B |    |    |    |    |       |    |    |     |     | Bridge to 14 |              |    |     |   |   |   | GND |

Where row *B* is on the side of the notches. The whole row *B* is connected to ground internally.

![Wiring diagram1](https://user-images.githubusercontent.com/19271533/40067831-60faf05a-5867-11e8-8a41-c66a6e115dc6.jpg)

![Wiring diagram2](https://user-images.githubusercontent.com/19271533/40067832-61157934-5867-11e8-97c3-bce709fa34ce.jpg)

## Console App  
The Console Application is a Python tool interfacing the functionality of the Arduino code. It allows you to create (test) sequences and send them to the Arduino. These sequences can be saved locally in a `.json` file. The Console Application is also able to parse midi files into sequences, allowing you to play music with your floppy drive.

### Navigation
The Console Application is able to display several menus and submenus. You can select a menu point by typing in the number in the `[ ]` and enter. Invalid input will put you back to the main menu.

### Establish Connection
As soon as you run the Console Application you are prompted to select the serial port to which the Arduino is connected. If you´re running on Windows the application automatically connects to the Arduino. After that the main menu is presented. 

> **Note:** Only one serial connection to the Arduino is allowed.

### Main Menu 
**Main Menu**
```
[ 0] : Add Teststeps
[ 1] : Edit Teststeps
[ 2] : Edit Sequence
[ 3] : Show Sequences
[ 4] : Save Sequences as JSON
[ 5] : Load JSON
[ 6] : Send Sequences to Arduino
[ 7] : Receive Test Log
[ 8] : Load Midi
[ 9] : Reset Memory
[10] : Print/Hide Debug Information
[11] : Clear Console
[12] : Exit
```

#### Submenu Add Teststeps

This submenu allows adding a datum to a sequence. In the first step you decide to which sequence you want to add. The next submenu determines how the sequence datum is defined:

```
Submenu teststeps:
   [ 0] : Frequency
   [ 1] : Period_Time
   [ 2] : Low_and_High_Period
```

For each of these 3 options you must set following parameters:

```
MotorOn off=0, on=1: 	-> Turns spindle motor on/off
Number of steps:	    -> Number of steps to be taken by the stepper motor
Direction:		        -> Direction of stepper motor (read head) movement: auto(0), in(1), out(2)
```

If you choose to define a sequence datum by its frequency [0] you are asked for the frequency in *Hertz* and the ratio of the high period to the low period. *Example*: A ratio of 2 means that the resulting high period time is twice as long as the low period time.

> **Note:** The low and high period time are rounded to integer before being transferred which can lead to small deviations of the frequency. It is therefore advisable to choose the values in such a way that the results are natural numbers.

In the menu *Period_Time* [1] you are also asked for the ratio but for the period time in *ms* instead of the frequency. 

> **Note:** It is advisable to choose the values in such a way that the results are natural numbers here as well.

The menu point *Low_and_High_Period* [2] asks for the low and high period time in *ms* directly.

#### Edit Teststeps
You select a sequence datum from a sequence and decide if you want to remove or edit the data. Editing data follows the same procedure as adding data.

#### Show Sequences
Displays all sequences and its data.

#### Save Sequences as JSON / Load JSON
The program asks for a path from which it can load/save the sequences. If you press enter with an empty input, it will load/save from this path `[Desktop]\floppysequence.json`.

#### Send Sequences to Arduino
This submenu lets you choose which sequence will be sent to the Arduino. In case of sequence *1* or *2* you are asked if these sequences should be saved persistently on Arduino´s EEPROM. These sequences can be played after the power supply was reconnected without transferring the sequences again. 

#### Receive Test Log
Receives and prints the last test log. For more details see [Test Sequences and Reports](#test-sequences-and-reports).

#### Load Midi
Parses a midi file to a sequence which can be sent to the Arduino. 
The program asks for the following parameters:

- Path to midi file
- Midi channel
- Number of Tracks (usually 40 or 80)
- Sequence number

#### Reset Memory
Resets Arduino. Clears all sequences and the test log.

#### Print/Hide Debug Information
If *print debug information* is enabled, every information received through the serial connection is printed out. 

#### Clear Console
Clears the console window.

#### Exit
Exits the console application.

## Remote Control

### Playing Sequences

Via the serial interface, a total of 4 different sequences can be sent to the Arduino and saved, for example with the *Console App*. A saved sequence can also be played via the *Remote Control*:

> Button `1`: Play sequence 1

> Button `2`: Play sequence 2

> Button `3`: Play sequence 3

> Button `4`: Play sequence 4

> **Note:** If the playback of the current sequence is a test sequence, a new sequence can not be started until the current test sequence has ended.

Sequences 1 and 2 can be saved persistently to the Arduino's EEPROM.

### Pause/Resume a Sequence

> Button `OK`: Pause or resume currently selected sequence (*except test sequences*)

### Cancel a Sequence

> Button `#`: Cancel current sequence or test sequence and reset stepper to initial state

## Serial Interface
Connection to the Arduino can be established directly with the serial interface. The baudrate is set to **9600Bd**. Thus *commands* can be invoked.

Commands always begin with **+** (plus) and end with **;** (semicolon). Parameters are separated by **,** (comma). Time values are given as **multiples of 100us** (1/10ms). The first parameter of a command is the *command number* and acts as an identifier. The following parameters are command dependent. The parameters may only be unsigned integers.

To stay consistent with the remote control, sequences are identified in user interfaces with ids from 1 to 4. To stay consistent with the Arduino's memory, **the serial interface uses ids from 0 to 3**. Id 1 in the user interface corresponds to id 0 in the serial interface.

It is possible to transfer multiple commands at once. They are processed in order.

### Commands:

> (a): Parameter index, [b...c] value range

___
#### Select Sequence
**Description**:

Select sequence as active. If the play button on the remote control is pressed, this will be played.

**Parameters**:

    (0): 0 (Command Number)
    (1): Id [0...3]

**Example:**
```python
# Select Sequence
+0,1;                   # Id 1
```
___
#### New Sequence
**Description:**

Create / allocate new temporary sequence and set its metadata. New sequence data will be added to this sequence ([see [ 2 ]](#2-new-sequence-datum)). It can be saved either persistently or transiently. 

>Note: Test Sequences cannot be looped! The *Test* flag will win over the *Loop* flag.

**Parameters:**

    (0): 1 (Command Number)
    (1): Id [0...3]
    (2): Number of Sequence data [1 ... 65535]
    (3): Loop Sequence [0,1]
    (4): Test Sequence [0,1]
    (5): Number of Tracks (usually 40 or 80)

**Example:**
```python
# New Sequence
+1,1,2,0,0,40;          # Id = 1, 2 Elements, Loop = false, Test = false, Tracks = 40
```

___
#### New Sequence Datum
**Description:**

Add new Sequence Datum to Sequence created with [[ 1 ]](#1-new-sequence). High/Low duration of step is set in multiples of 100us. *Example*: For 1ms duration a value of 10 must be set.

> **Note:** If the *Loop* flag of the sequence metadata is set to true, only *Auto* (0) is allowed for the direction. The value will be overwritten if set differently.

**Parameters:**

    (0): 2 (Command Number)
    (1): Steps [1...65535]
    (2): High Duration [0...65535]
    (3): Low Duration [0...65535]
    (4): Direction [0=Auto, 1=Inwards, 2=Outwards]
    (5): Motor On [0,1]

**Example:**
```python
# New Sequence Datum
+2,100,200,250,0,0;     # 100 Steps, High Duration 200 (20ms), Low Duration 250 (25ms),
                        # Direction Auto, Motor On false
```
___
#### Save Sequence
**Description:**

Save Sequence to memory. Sequences 1 and 2 (Id 0 and 1) can be saved persistently to the Arduino's EEPROM. See [Saving and Restoring Data](#saving-and-restoring-data).

**Parameters:**
    
    (0): 3 (Command Number)
    (1): Persistently (only Sequence 1 and 2 can be saved persistently) [0,1]

**Example:**
```python
# Save Sequence
+3,1;                   # Persistently true
```

___
#### Play Sequence
**Description:**

Plays the Sequence with the given Id.

**Parameters:**

    (0): 4 (Command Number)    
    (1): Id [0...3]

**Example:**
```python
# Play Sequence
+4,1;                   # Id 1
```

___
#### Restore
**Description:**

Moves the read head of the floppy drive outwards until *TRK00* is set.

**Parameters:**

    (0): 5 (Command Number)

**Example:**
```python
# Restore
+5;
```

___
#### Motor On/Off
**Description:**

Enables / Disables the spindle motor.

**Parameters:**

    (0): 6 (Command Number)
    (1): State [0,1]

**Example:**
```python
# Motor On/Off
+6,1;                   # Motor On true
```

___
#### Instant Playback
**Description:**

Instantaneously plays back the given sequence datum. 

**Parameters:**

    (0): 7 (Command Number)
    (1): Steps [1...65535]
    (2): High Duration [0...65535]
    (3): Low Duration [0...65535]
    (4): Number of Tracks (optional, switches number, required only on the first datum)

**Example:**
```python
# Instant Playback
+2,100,200,250,40;     # 100 Steps, High Duration 200 (20ms), Low Duration 250 (25ms), 40 Tracks
```
___
#### Receive Error Log
**Description:**

Displays the current error log. 

**Parameters:**

    (0): 8 (Command Number)

**Example:**
```python
# Receive Error Log
+8;
```

___
#### Delete all Data including EEPROM
**Description:**

Deletes all data from the Arduino including persistently saved sequences and the error log.

**Parameters:**

    (0): 9 (Command Number)

**Example:**
```python
# Delete all data
+9;
```

## Examples
> It is possible to transfer multiple commands at once. They are processed in order.

```python
# New Sequence
+1,1,2,0,0,40;         # Id = 1, 2 Elements, Loop = false, Test = false, Tracks = 40

# New Sequence Datum
+2,100,200,250,0,0;    # 100 Steps, High Duration 200, Low Duration 250, Direction Auto, Motor On false

# New Sequence Datum
+2,2000,100,100,0,0;   # 2000 Steps, High Duration 100, Low Duration 100, Direction Auto, Motor On false

# Save Sequence
+3,1;                  # Save latest Sequence persistently

# Play Sequence
+4,1;                  # Play Sequence with index 1
```

### Play Sequence
```python
# Create command with index 0, transfer some data, save it persistently and play:
+1,0,2,0,0,40;+2,100,100,100,0,1;+2,50,400,400,0,0;+3,1;+4,0;
```

### Test Sequence
```python
# Create test command with index 0, transfer some data, save it persistently and play:
+1,0,2,0,1,40;+2,10,100,100,1,1;+2,15,10,10,1,0;+3,1;+4,0;
```

### Receive Error Report / Delete Data
```python
# Receive last error report
+8;

# Restore, then receive last error report, then delete all data
+5;+8;+9;
```

# About

## Arduino Code

### Structs

#### Floppy

The struct `Floppy` contains all variables necessary to manage the state of the `MotorControl` instance. The pin assignments for controlling the floppy drive are defined. The resolution of the timer for the interrupt service routine, which controls the switching of the high and low phases of the floppy stepper, is set to *50 microseconds*, which is the reason for the time resolution to be **100 microseconds**.

#### Sequence

The sequence structure defines the metadata of a sequence and contains the actual sequence data in an array. To create a sequence, the required memory must be allocated via the `MemoryManager`. This requires the length of the sequence. All other metadata about the sequence (see [Serial Interface](#serial-interface)) can be saved in the corresponding variables after allocation. The actual (transmitted) length of the sequence is saved in the variable `actual_length`. If this value does not correspond to the length after transmission, a warning is printed.

In a sequence, the `test` and `loop` flags must never be true at the same time. This is contradictory and is usually prevented when creating in `Com` object.

#### SequenceDatum

A sequence datum describes a single sequence step of a sequence. These include the number of steps to be taken, the duration of the high period, the duration of the low period, the direction of movement of the read head and the status of the spindle motor. The sequence data can only be transferred after creating a sequence structure. In a loop sequence (loop flag in the sequence structure is true), the direction of movement of the read head must be set to automatic, as otherwise proper movement of the read head can not be ensured.

#### ErrorReport

The error report structure contains the results of a test sequence. If there is a difference between the setpoint and the actual value of the steps taken, this is saved in the step difference of the error report. If the read head moves against the start or end limit, the maximum or minimum position flag is set to true. The `MemoryManager` singleton instance has an error report object in which the results of the current test can be set and possibly be save persistently (see [Memory Manager](#memory-manager)).

### Com

The `Com` class is responsible for the serial communication to the Arduino. The parsing and triggering of the commands takes place here. The singleton instance must be updated regularly with the method `Update()`. Since this action is less important and time-critical than controlling the floppy drive, it is not triggered by a timer (and executed in an interrupt routine) but by calling the `Update()` method in the main loop. Each module of the application can send messages to the computer independently by calling `Serial.print(...)`.

### Memory Manager

Dynamic memory management on Arduino is problematic because there is very little memory available. Memory leaks must be prevented. The `MemoryManager` singleton instance takes on this task. To access the `MemoryManager` object, the static method `getInstance()` must be called. Access to a sequence is possible through the `GetSequence(int index)` method. For example, a full call looks like this: `MemoryManager::getInstance()->GetSequence(0);`.
Sequences can be deleted using the `DeleteSequence()` method and allocated using `AllocateSequence(unsigned int length)`. `SaveSequence(Sequence * sequence, bool persistent = false)` can be used to save sequences. After changing the `ErrorReport` object, it must be persisted via `SaveErrorReportToEEPROM()`.

### Motor Control

The `MotorControl` class is responsible for handling the frequency for the floppy stepper and driving the spindle motor. The frequency is realized with the help of a timer and the external library `TimerOne`. The class is implemented as a singleton. To access the `MotorControl` object, the static method `getInstance()` must be called.

When loading or turning on the Arduino, an initial restore is performed using the `InitialRestore()` method.

A sequence definition is addressed using the `GetSequence (int num)` method. In this method, the sequence is fetched from the `MemoryManager` instance.

The timer is initialized with the resolution defined in the `Floppy` struct and the `Tick()` method is attached as an interrupt service routine. Here is the frequency generation:

Every 50us, the `currentTick` variable of the `Floppy` struct is incremented and checked alternately against the *high* or *low duration* of the current sequence datum. Time values of high and low duration are given as multiples of *100us* (1 / 10ms). After exceeding the low period, a step (*high pulse*) is generated. The *currentState* changes to high. Now the high period is waited for and then a *low pulse* is generated and the current state is set low again, etc. The whole process takes place until the maximum step definition of a sequence datum is reached.

After completing the steps of a sequence datum, it will continue depending on the sequence definition:

1. To process another sequence datum in sequence?
    
    > The steps of the next sequence datum are processed.

2. Loop = *true*?
    
    > Once the complete sequence has been completed and the *Loop* flag set in the sequence definition, the sequence will be played again.

3. Test = *true*?
    
    > Once the complete sequence has been completed and the *Test* flag set in the sequence definition, the floppy stepper is restored. Any errors in the error log are passed to the *MemoryManager* instance and saved in EEPROM.

4. Loop = *false* and Test = *false*?
    
    > Once the complete sequence has been completed and the *Loop* flag and *Test* flag are not set in the sequence definition, the state variables for controlling the floppy stepper are reset. The floppy stepper will not be restored and will remain at its current position.

### Remote Control

The `RemoteControl` class is the responsible class for the operation of the Remote Control with which sequences on the floppy drive can be played. This is implemented as a singleton instance. To access the `RemoteControl` object, the static method `getInstance()` must be called. It also uses the external library `IRremote`. 

In the `Init(unsigned int recvPin)` method, the input pin for the infrared receiver is defined and the associated interrupt service routine is activated for reception. 

The method `Recv()` is called in the `loop()` method of `main.ino`, which decodes the hexadecimal values of the Remote Control. A decoded value can be used to call methods of the Motor Control. For this, the `RemoteControl` instance contains a reference to the `MotorControl` instance within the `setup()` method of the `main.ino`.

Floppy Tester uses this [Remote Control](https://www.amazon.de/Gaoxing-Tech-Infrarot-Empfänger-Wireless/dp/B072BPVTZH/ref=sr_1_1?ie=UTF8&qid=1521899060&sr=8-1&keywords=remote+control+arduino) with the following hexadecimal codes:

```
0               0xFF9867
1               0xFFA25D
2               0xFF629D
3               0xFFE21D
4               0xFF22DD
5               0xFF02FD
6               0xFFC23D
7               0xFFE01F
8               0xFFA857
9               0xFF906F
OK              0xFF38C7
#               0xFFB04F
*               0xFF6897
Left Arrow      0xFF10EF
Up Arrow        0xFF18E7
Right Arrow     0xFF5AA5
Down Arrow      0xFF4AB5
```

Another Remote Control can be used by changing the hexadecimal codes in `RemoteControl.cpp`.

### Main

The file `main.ino` defines the two main methods `setup()` and `loop()` for executing the Arduino program.

The `setup()` method of the floppy tester is called at startup or immediately after transferring the program to the Arduino. The *baud rate* for the serial transmission as well as the initialization for the *Motor Control*, *Remote Control* and *Memory Manager* is performed.

The `loop()` method of the floppy tester cyclically checks the reception of the Remote Control and parses serially received commands for the definition and playback of sequences. After executing a test sequence, the restore of the floppy stepper is handled.

### External Libraries

Library     | Intended purpose
---         | ---                 
[TimerOne](http://playground.arduino.cc/Code/Timer1)    |   Timer Resolution for floppy stepper frequency
[IRremote](http://z3t0.github.io/Arduino-IRremote/)    |   Remote Control for sequence handling

## Python Code (CLI)
The program is started by running `Floppy.py`. The function `ConsoleApp()` is the main function which prints the menus, reads the inputs and executes the code for each menu selection.

### Data Structures
The structure of a sequence is defined in `FloppySequences.py`.
The class `Teststep` can create a sequence datum by frequency, step time or high and low period time. The function `GetCommand` parses the sequence datum into a binary representation which the Arduino can understand (see [Commands](#commands)). 

The class `FloppySequences` is a representation of a sequence and offers the functions to play and save a sequence as well as static functions to create a `ReceiveLogCommand` and a `ResetAllCommand`.

The file `FloppyMenu.py` defines the `Enums` which represent the menu structure.

### JSON Files / Serializing
The file `FloppySerializer.py` is responsible for parsing sequences into a JSON file format. The JSON file has the following format:
```JSON
{
    "sequencearray": [
        {
            "NumberOfTracks": 40,
            "Test": false,
            "TestId": 1,
            "TestName": "Sequence1",
            "loop": false,
            "sequences": [
                {
                    "StepNumber": 200,
                    "dir": "AUTO",
                    "high": 100,
                    "low": 100,
                    "motorOn": true
                },
                {
                    "StepNumber": 10,
                    "dir": "OUT",
                    "high": 50,
                    "low": 50,
                    "motorOn": false
                }
            ]
        },
        {
            "NumberOfTracks": 40,
            "Test": false,
            "TestId": 2,
            "TestName": "Sequence2",
            "loop": false,
            "sequences": []
        },
        {
            "NumberOfTracks": 40,
            "Test": false,
            "TestId": 3,
            "TestName": "Sequence3",
            "loop": false,
            "sequences": []
        },
        {
            "NumberOfTracks": 40,
            "Test": false,
            "TestId": 4,
            "TestName": "Sequence4",
            "loop": false,
            "sequences": []
        }
    ]
}
```
The first element is the `"sequencearray"`, containing up to 4 sequences. Each sequence has the following elements:

- "NumberOfTracks":
  - Integer value
- "Test":
  - true or false
- "TestId": 
  - Integer (1-4)
- "TestName":
  - String
- "loop": 
  - true or false
- "sequences": []
  - Array of sequence data with following elements:
    - "StepNumber":
      - Integer (40 or 80)
    - "dir":
      - Direction "IN", "OUT" or "AUTO"
    - "high":
      - High period time in ms
    - "low": 50,
      - Low period time in ms
    - "motorOn":
      - true or false

### Serial Connection
The file `FloppySerialConnection.py` is handling the serial connection to the Arduino. It is running a thread to continuously read from the serial port. If it receives a `'+'` the application is waiting for a test report and continues to read until it receives a `';'` character.
The `SendCommands` method is used to send a list of commands to the Arduino. The `getAvailablePorts` method returns all available serial ports. The `Connect` method tries to connect to a port. The baudrate is set to 9600Bd.

### Midi Parser

The Midi parser reads and processes midi files so that they can be played by a floppy drive. For this the [mido](https://mido.readthedocs.io/en/latest/) library is used.
To be able to play all midi files, the frequencies are tuned to a value of less than 440Hz. The songs are thus shifted only in their octave, but not in their key. To avoid memory problems, songs are shortened to a maximum of 50 steps (sounds and pauses). If a midi file contains more than one track, the user is prompted to select one.

A `MidiParser` object is created with a path to a midi file and a `FloppySerialConnection` object.
In addition to the `parse(channel, numberOfTracks)` method, there is also a `play(channel, numberOfTracks)` method, which plays the contents of a midi file directly. However, due to time delays in transmitting and processing the messages, this is not appropriate for musical midis where correct timing is of relevance.

### External Libraries

Library     | Intended purpose
---         | ---
[pyserial](http://pythonhosted.org/pyserial/pyserial.html)    |   Serial Connection
[mido](https://mido.readthedocs.io/en/latest/)        |   Midi Parsing
[json](https://docs.python.org/2/library/json.html)		|   Parsing JSON files
