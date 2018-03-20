from enum import Enum

class Direction(Enum):
    AUTO = 0
    IN = 1
    OUT = 2

class TestStep:
    arduinoTimerCompensation = 10 # the arduino software uses 1/10 ms instead of 1/1 ms, the factor compensates this
    def __init__(self, steps, high,low,direction,motoron):
        self.numberSteps =steps
        self.durationHigh =high
        self.durationlow =low
        self.direction = direction
        self.motorOn = motoron
        
    @staticmethod
    def CreateStep(numberOfsteps, high,low,direction,motorOn):
        return(TestStep(numberOfsteps, high,low,direction,motorOn))
    
    @staticmethod
    def CreateStepFrequency(numberOfsteps, frequencyInHz, highToLowRatio, direction, motorOn):
        if(frequencyInHz > 0):
            periodTime = (1 / frequencyInHz)*1000 # in ms =low + high
        else:
            periodTime =0
        return TestStep.CreateStepTime(numberOfsteps,periodTime,highToLowRatio,direction, motorOn)
    @staticmethod
    def CreateStepTime(numberOfsteps, timeinms, highToLowRatio,direction, motorOn):
        if(highToLowRatio >= 1):
            low = timeinms / (highToLowRatio +1)
            high = timeinms-low
        elif (highToLowRatio >= 0):
            high = timeinms * highToLowRatio
            low = timeinms - high
        else:
            print("Negative ratio not allowed")
        return (TestStep(numberOfsteps, high,low,direction,motorOn))


    def __str__(self):
        return 'Number of steps: {0}, {1} ms high , {2} ms low, MotorOn = {3}, {4}'.format(self.numberSteps,int(round(self.durationHigh)),int(round(self.durationlow)),self.motorOn,self.direction)

    def GetCommand(self, instant = False):
        if not instant:
            return "+2,{0},{1},{2},{3},{4};".format(self.numberSteps,int(round(self.durationHigh * TestStep.arduinoTimerCompensation)),int(round(self.durationlow * TestStep.arduinoTimerCompensation)),self.direction.value,int(self.motorOn == True))
        else:
            return "+7,{0},{1},{2},{3};".format(self.numberSteps,int(round(self.durationHigh * TestStep.arduinoTimerCompensation)),int(round(self.durationlow * TestStep.arduinoTimerCompensation)),self.numberSteps)
        
class FloppySequences:
    def __init__(self, id, name,loop,test,numberOfTracks, instant = False):
        self.TestId =id
        self.TestName =name
        self.loop =loop
        self.NumberOfTracks =numberOfTracks
        self.test = test
        self.steps =[]
        self.instant = instant
    def __str__(self):
        str= '{0} (id: {1}) loop = {2}, Test = {3} , Number of tracks = {4} :\n'.format(self.TestName,self.TestId,self.loop,self.test, self.NumberOfTracks)
        for counter, step in enumerate(self.steps):
            str+='{0}: ({1})\n'.format(counter,step)
        return str

    #Parse Sequence to command
    def GetCommand(self):
        commands = []
        str="+1,{0},{1},{2},{3},{4};".format(self.TestId -1,len(self.steps),int(self.loop == True),int(self.test == True),self.NumberOfTracks)
        
        if not self.instant:
            commands.append(str)
        for step in (self.steps):
            str=step.GetCommand(self.instant)
            commands.append(str)
        return commands

    #Save the last command 
    #sequence 1 and 2 can be saved on arduino persistently
    def SaveCommand(self,persistent=False):
         return"+3,{0};".format(int(persistent == True))  


    #Command to receive last testlog
    @staticmethod
    def ReceiveLogCommand():
        return"+8;" 
    #reset everything on arduino
    @staticmethod
    def ResetAllCommand():
        return"+9;"   

    #play this sequence on arduino
    def PlayCommand(self):
        return"+4,{0};".format(self.TestId-1)    

        