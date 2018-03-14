from enum import Enum

def printMenu(Index,Text):
	return '[{0:>2}] : {1}'.format(Index,Text)

def MainMenu(title,Enum):
    print(title)
    for menuitem in Enum:
        print(menuitem)
def SubMenu(title,Enum):
    print(title)
    for menuitem in Enum:
        print("   "+str(menuitem)) 

class MenuEnum(Enum):
    addTeststeps =0
    editTeststeps=1
    editSequence=2
    showSequences =3
    Save =4
    Load =5
    Connect_to_arduino = 6
    ReceiveLog =7
    Load_Midi = 8
    ResetAll =9
    ToggleDebug =10
    Clear =11
    EXIT =12
    def __str__(self):
        return printMenu(self.value,self.GetString())

    def GetString(self):
        if(self.value ==self.addTeststeps.value):
            return "Add Teststeps"
        if(self.value ==self.editTeststeps.value):
            return "Edit Teststeps"
        elif (self.value ==self.editSequence.value):
            return "Edit Sequence"
        elif (self.value ==self.showSequences.value):
            return "Show Sequences"
        elif (self.value ==self.Connect_to_arduino.value):
             return "Send Sequences to Arduino"
        elif (self.value ==self.Load_Midi.value):
             return "Load Midi"
        elif (self.value ==self.ReceiveLog.value):
             return "Receive Test Log"        
        elif (self.value ==self.ResetAll.value):
             return "Reset Memory"
        elif (self.value ==self.ToggleDebug.value):
             return "Print/Hide Debug Information"
        elif (self.value ==self.Clear.value):
             return "Clear Console"
        elif (self.value ==self.Save.value):
            return "Save Sequences as JSON"
        elif (self.value ==self.Load.value):
             return "Load JSON"
        elif (self.value ==self.EXIT.value):
             return "Exit"
        else :
            return self.name

class SubMenuAddTeststepsEnum(Enum):
    Frequency =0
    Period_Time =1
    Low_and_High_Period =2
    def __str__(self):
        return printMenu(self.value,self.name)

class SubMenuChooseSequenceEnum(Enum):
    Sequence1 =1
    Sequence2 =2
    Sequence3 =3
    Sequence4 =4
    def __str__(self):
        return printMenu(self.value,self.name)
