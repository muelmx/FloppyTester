import json
from FloppySequences import Direction,FloppySequences,TestStep
from FloppyMenu import MenuEnum,SubMenuAddTeststepsEnum

errornumber =-1

#parse input to number 
def toNum(text):
    try:
        number = int(text)
        if(number < 0):
            return errornumber
        return number 
    except:
        try:
            number = float(text)
            if(number < 0):
                return errornumber
            return number
        except:
            print("Please input a positive number")
            return errornumber
#parse input to int
def toInt(text):
    try:
        number = int(text)
        if(number < 0):
            return errornumber
        return number
    except:
        print("Please input a positive number")
        return errornumber

class Serializer:
    def __init__(self):
        #Json attributes
        self.__stepNumber="StepNumber"
        self.__high ="high"
        self.__low = "low"
        self.__dir ="dir"
        self.__motorOn ="motorOn"
        self.__sequences ="sequences"
        self.__TestId ="TestId"
        self.__TestName ="TestName"
        self.__loop ="loop"
        self.__test ="Test"
        self.__numberTrack="NumberOfTracks"
        self.__sequencearray ="sequencearray"

    #save json file
    def Save(self,sequencesList,filepath):
        #Genearte Json structure
        jsonSequences =[]
        for idx, seq in enumerate(sequencesList):
            steplist=[]
            for val in (seq.steps):
                steplist.append({self.__stepNumber: val.numberSteps,self.__high : int(round(val.durationHigh)),self.__low : int(round(val.durationlow)), self.__dir: val.direction.name, self.__motorOn:val.motorOn})
            jsonSequences.append({self.__sequences :steplist,self.__TestId : idx+1, self.__TestName :seq.TestName, self.__loop :seq.loop, self.__test :seq.test, self.__numberTrack : seq.NumberOfTracks })
        dictionary = {self.__sequencearray: jsonSequences}

        #write output
        with open(filepath, 'w') as outfile:
            json.dump(dictionary, outfile,indent=4, sort_keys=True)
        print("Saved file to "+filepath)
    
    #load json file
    def Load(self,filepath):
        with open(filepath, 'r') as infile:
            jsonsequence=json.load(infile)
        newsequencesList= [] 
        for idx, val in enumerate(jsonsequence[self.__sequencearray]):
            newsequencesList.append(FloppySequences(idx+1,val[self.__TestName],bool(val[self.__loop]),bool(val[self.__test]),toInt(val[self.__numberTrack]),False))
            for sequence in val[self.__sequences]:
                newsequencesList[idx].steps.append(TestStep(toInt(sequence[self.__stepNumber]),toNum(sequence[self.__high]),toNum(sequence[self.__low]),Direction[sequence[self.__dir]],bool(sequence[self.__motorOn])))
        return newsequencesList