import os
import sys
import random
#pip install pyserial
import serial
import serial.tools.list_ports
from FloppySequences import Direction,FloppySequences,TestStep
from FloppySerializer import Serializer,toInt,toNum
from FloppyMenu import MenuEnum,SubMenuAddTeststepsEnum,SubMenuChooseSequenceEnum,MainMenu,SubMenu,printMenu
#pip install mido
from MidiParser import MidiParser
import time
from threading import Thread
import atexit
from FloppySerialConnection import SerialConnection

serConnection = SerialConnection()

def ReadInput(inputText):
    try:
        text = input(inputText)
    except KeyboardInterrupt:
        print("")
        sys.exit()
    
    return text

#Stop serial read thread
@atexit.register
def onClosing():
    serConnection.StopThread()

def getDesktopPath():
    if os.name == 'nt':
        return os.path.join(os.path.join(os.environ['USERPROFILE']), 'Desktop') 
    else:
        return os.path.join(os.path.join(os.path.expanduser('~')), 'Desktop') 

def ConnectArduino():
    print("Not connected to Arduino. Trying to connect...")
    #list all ports
    ports = serConnection.GetAvailablePorts()
    found = False
    for idx,p in enumerate(ports):
        print (printMenu(idx,p))
        if "Arduino" in p[1]:
            found = True
            print ("Found Arduino at "+p[0])
            serConnection.Connect(p[0])
    if (not found):
        while True: #Read until connected
            portNr =toInt(ReadInput('Select serial port: '))
            #wait in this loop if no arduino is found
             
            if(portNr < len(ports) and portNr >= 0 and serConnection.Connect(ports[portNr][0])):
                break
            else:
                print("Invalid Input")

def printSequenceMenu(selecttext):
    SubMenu("Select sequence:",SubMenuChooseSequenceEnum)
    sequenceNumber= toInt(ReadInput(selecttext))
    if(sequenceNumber < 1 or sequenceNumber > 4):
        print("Invalid sequence number")
        return False
    return sequenceNumber

#Create new Sequence-Datum and return the newly generated Datum
def GenerateSequenceDatum(sequenceNumber):
    #Choose how to add to sequence
    SubMenu("SubMenu teststeps:",SubMenuAddTeststepsEnum)
    subMenuIndex = toInt(ReadInput('Choose how to add to sequence '+str(sequenceNumber)+": "))
    if(subMenuIndex < SubMenuAddTeststepsEnum.Frequency.value or subMenuIndex > SubMenuAddTeststepsEnum.Low_and_High_Period.value ):
        print("Invalid sequence number")
        return False

    MotorOn=toInt(ReadInput('MotorOn off=0, on=1: '))
    if(MotorOn < 0 or MotorOn > 1):
        print("Invalid number")
        return False

    steps =toInt(ReadInput('Number of steps: '))
    if(steps <0):
        print("Invalid steps number")
        return False

    dirstring =str(ReadInput('Direction auto(0), in(1), out(2): '))
    if dirstring == 'in' or toInt(dirstring) ==1:
        direct =Direction.IN
    elif (dirstring == "out" or toInt(dirstring) ==2):
        direct =Direction.OUT
    elif(dirstring == "auto" or toInt(dirstring) ==0):
            direct =Direction.AUTO
    else:
        print("Invalid Input.")
        return False

    #enter a frequency
    if (subMenuIndex == SubMenuAddTeststepsEnum.Frequency.value):
        ratio =toNum(ReadInput('High low ratio: '))
        if(ratio <0):
            print("Invalid ratio")
            return False
        freq =toNum(ReadInput('Frequency: '))
        if(freq <0):
            print("Invalid frequency")
            return False
        step= TestStep.CreateStepFrequency(steps,freq,ratio,direct,bool(MotorOn))
    #enter a time period
    elif(subMenuIndex == SubMenuAddTeststepsEnum.Period_Time.value):
        ratio =toNum(ReadInput('High low ratio: '))
        if(ratio <0):
            print("Invalid ratio")
            return False
        time =toNum(ReadInput('Period time in ms: '))
        if(time <0):
            print("Invalid time")
            return False
        step=TestStep.CreateStepTime(steps,time,ratio,direct,bool(MotorOn))
    #enter low and high period
    elif(subMenuIndex == SubMenuAddTeststepsEnum.Low_and_High_Period.value):
        high =toNum(ReadInput('High period in ms: '))
        if(high <0):
            print("Invalid high period")
            return False
        low =toNum(ReadInput('Low period in ms: '))
        if(low <0):
            print("Invalid low period")
            return False
        step=TestStep.CreateStep(steps,high,low,direct,bool(MotorOn))
    return step

#Main function
def ConsoleApp():
    ConnectArduino()
    serConnection.StartThread()
    serializer = Serializer()
    numberOftracks =0
    
    #Initialize the sequence list
    sequencesList= []
    sequencesList.append(FloppySequences(1,"Sequence1",False,False,40,False))
    sequencesList.append(FloppySequences(2,"Sequence2",False,False,40,False))
    sequencesList.append(FloppySequences(3,"Sequence3",False,False,40,False))
    sequencesList.append(FloppySequences(4,"Sequence4",False,False,40,False))

    print("FLOPPY CONSOLE APPLICATION")

    while True:
        MainMenu("Main Menu",MenuEnum)
        menuIndex = toInt(ReadInput('Choose a number: '))
        ###########################################
        #### Add Sequence
        ###########################################
        if (menuIndex == MenuEnum.addTeststeps.value):
            #Choose Sequence
            sequenceNumber= printSequenceMenu("Choose a sequence to add to: ")
            if not sequenceNumber:
                continue
            step = GenerateSequenceDatum(sequenceNumber)
            if step ==False:
                continue
            sequencesList[sequenceNumber-1].steps.append(step)
            print("Sequence added \n{0}".format(sequencesList[sequenceNumber-1].steps[-1]))
        
        ###########################################
        #### edit Teststep
        ###########################################
        elif menuIndex == MenuEnum.editTeststeps.value :
            #Choose Sequence
            sequenceNumber= printSequenceMenu("Choose a sequence: ")
            if not sequenceNumber:
                continue

            if len(sequencesList[sequenceNumber-1].steps) < 1:
                print("Empty sequence")
                continue

            print("Which sequence datum to you want to edit?")
            for idx, step in enumerate(sequencesList[sequenceNumber-1].steps):
                print("   "+printMenu(idx,step))  

            stepNumber= toInt(ReadInput('Choose a sequence datum to edit: '))
            if(stepNumber < 0 or stepNumber >len(sequencesList[sequenceNumber-1].steps)-1):
                print("Invalid number for sequence datum")
                continue

            edit= toInt(ReadInput('Remove(0) or edit(1) sequence datum '+str(stepNumber)+': '))
            if(edit < 0 or edit >1):
                print("Invalid input")
                continue

            if(edit ==1):
                step = GenerateSequenceDatum(sequenceNumber)
                if step ==False:
                    continue
                sequencesList[sequenceNumber-1].steps[stepNumber] =step
                print("Sequence datum {0} edited to:\n{1}".format(stepNumber,sequencesList[sequenceNumber-1].steps[stepNumber]))
            else:
                sequencesList[sequenceNumber-1].steps.remove(sequencesList[sequenceNumber-1].steps[stepNumber])
                print("Sequence datum removed")
        ###########################################
        #### showSequence
        ###########################################
        elif menuIndex == MenuEnum.showSequences.value :
            print("\nShowing all sequences:\n")
            for sequence in sequencesList:
                print(sequence)

        ###########################################
        #### editSequence
        ###########################################
        elif menuIndex == MenuEnum.editSequence.value :
            #Choose Sequence
            sequenceNumber= printSequenceMenu("Choose a sequence to edit: ")
            if not sequenceNumber:
                continue

            loop= toInt(ReadInput('Set sequence LOOP property true(1) or false(0): '))
            if(loop < 0 or loop > 1):
                print("Invalid number")
                continue

            test= toInt(ReadInput('Set sequence TEST property true(1) or false(0): '))
            if(test < 0 or test > 1):
                print("Invalid number")
                continue

            tracks= toInt(ReadInput('Number of Tracks 40 or 80: '))
            if(tracks != 40 and tracks != 80):
                print("Invalid number")
                continue

            #update sequence properties
            sequencesList[sequenceNumber-1].loop =bool(loop)
            sequencesList[sequenceNumber-1].test =bool(test)
            sequencesList[sequenceNumber-1].NumberOfTracks =tracks

            numberOftracks = tracks
        ###########################################
        #### Save
        ###########################################
        elif menuIndex == MenuEnum.Save.value :                 
            examplefilepath=os.path.join(getDesktopPath(),"floppysequence.json")
            filepath =str(ReadInput('Where do you want to save? e.g. '+examplefilepath+" : Enter to accept\n"))
            if not filepath:
                filepath=examplefilepath

            serializer.Save(sequencesList, filepath)

        ###########################################
        #### Load
        ###########################################
        elif menuIndex == MenuEnum.Load.value :
            examplefilepath=os.path.join(getDesktopPath(),"floppysequence.json")
            filepath =str(ReadInput('Load file from? e.g. '+examplefilepath+" : Enter to accept\n"))
            if filepath and os.path.exists(filepath) == False:
                print("File at path " + filepath + " does not exist!")
                continue
            if not filepath:
                filepath=examplefilepath
            
            sequencesList.clear()
            sequencesList = serializer.Load(filepath)
        ###########################################
        #### Clear
        ###########################################
        elif menuIndex == MenuEnum.Clear.value :
            if os.name == 'nt':
                os.system('cls')  # on windows
            else:
                os.system('clear')  # on linux / os x

        ###########################################
        #### Sent To arduino
        ###########################################
        elif menuIndex == MenuEnum.Connect_to_arduino.value :
            sequenceNumber= printSequenceMenu("Choose a sequence to send to the Arduino: ")
            if not sequenceNumber:
                continue

            persistent = False
            if sequenceNumber <3: #persistent is allowed for sequence 1 and 2
                persistent= toInt(ReadInput('Save persistent true(1), false(0) ? '))
                if(persistent < 0 or persistent > 1):
                    print("Invalid number")
                    continue

            cmds = sequencesList[sequenceNumber-1].GetCommand()
            cmds.append(sequencesList[sequenceNumber-1].SaveCommand(persistent))
            cmds.append(sequencesList[sequenceNumber-1].PlayCommand())
            
            if not serConnection.SendCommands(cmds):
                ConnectArduino()

        ###########################################
        #### Receive Log
        ###########################################
        elif menuIndex == MenuEnum.ReceiveLog.value :
            cmds =[]
            cmds.append(FloppySequences.ReceiveLogCommand())
            if serConnection.SendCommands(cmds):
                print("")
                time.sleep(0.5)
                print("")
                #CONNECT
            else:
                ConnectArduino()
        ###########################################
        #### Load Midi
        ###########################################
        elif menuIndex == MenuEnum.Load_Midi.value :

            path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "Tetris.mid")
            filepath = str(ReadInput('Load file from? e.g. '+path+" : Enter to accept:"))

            if filepath and os.path.exists(filepath) == False:
                print("File at path " + filepath + " does not exist!")
                continue

            if not filepath:
                filepath = path
            
            parser = MidiParser(filepath, serConnection)
            channel = toInt(ReadInput("Midi Track has " + str(parser.channels) + " channels. Select number (0..." + str(parser.channels - 1) + "):")) # TODO: Error handling
            if channel < 0 or channel > parser.channels - 1:
                print("Invalid number")
                continue
            
            if not numberOftracks>0:
                numberOftracks= toInt(ReadInput('Number of Tracks 40 or 80: '))
                if(numberOftracks != 40 and numberOftracks != 80):
                    print("Invalid number")
                    continue

            # parser.play(channel)
            floppysequence=parser.parse(channel,numberOftracks)
            numofSteps = len(floppysequence.steps)
            if  numofSteps>parser.MAXLEN:
                #crop= toInt(ReadInput('Sequence to long to be saved on Arduino.\nDo you want to play the sequence directly (0) or crop and transfer it (1)? '))
                crop=1
                if(crop < 0 or crop > 1):
                    print("Invalid number")
                    continue
                
                if(crop ==1):
                    numofSteps=parser.MAXLEN
                else:
                    floppysequence.instant =True
                    persistent = False

            persistent = False    
            if  numofSteps<parser.MAXLEN or crop ==1:
                floppysequence.instant =False
                sequenceNumber= printSequenceMenu("Choose a sequence to send to the Arduino: ")
                if not sequenceNumber:
                    continue
                if sequenceNumber <3:
                    persistent= toInt(ReadInput('Save persistent true(1), false(0) ? '))
                    if(persistent < 0 or persistent > 1):
                        print("Invalid number")
                        continue
                floppysequence.TestId =sequenceNumber
                floppysequence.TestName ="Sequence"+str(sequenceNumber)
                floppysequence.steps = floppysequence.steps[:numofSteps]
                sequencesList[sequenceNumber-1]=floppysequence
            
            cmds = floppysequence.GetCommand()
            cmds.append(floppysequence.SaveCommand(persistent))
            cmds.append(floppysequence.PlayCommand())

            if not serConnection.SendCommands(cmds):
                ConnectArduino()

        ###########################################
        #### Reset All
        ###########################################
        elif menuIndex == MenuEnum.ResetAll.value :
            cmds =[]
            cmds.append(FloppySequences.ResetAllCommand())
            if not serConnection.SendCommands(cmds):
                ConnectArduino()


        ###########################################
        #### Toggle Debug
        ###########################################
        elif menuIndex == MenuEnum.ToggleDebug.value :
            serConnection.enableDebugOutputGlobal = not serConnection.enableDebugOutputGlobal

            if serConnection.enableDebugOutputGlobal:
                print ("Debug Information enabled")
            else:
                print ("Debug Information disabled")   
        ###########################################
        #### Exit
        ###########################################
        elif menuIndex == MenuEnum.EXIT.value :
            serConnection.StopThread()
            break
        elif menuIndex == exit :
            break

if __name__ == "__main__":
    ConsoleApp();