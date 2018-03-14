# pip install tqdm

import time
from threading import Thread

import serial
import serial.tools.list_ports
import sys

class SerialConnection:
    def __init__(self):
        self.readThread = Thread(target=self.ReadSerialIput)
        self.connection = None
        self.running = True
        self.wait = False
        self.allowPrint =False
        self.enableDebugOutputGlobal=False
    def Connect(self,com):
        try:
            print("\nTrying to connect to "+ com)
            self.connection= serial.Serial(com,9600)   # open serial port that Arduino is using
            print("Connection established successfully\n")
            return True
        except :
            print("\nFailed to connect")
            return False
    #Read until a certain charakter is received
    #returns the read string
    def ReadSerialIputUntilChar(self,char):
        self.wait= True
        stringbuffer =""
        while True:
            readchar =self.connection.read().decode() 
            if readchar == char:
                break
            else:
                stringbuffer += readchar

        self.wait= False
        return stringbuffer       

    #read thread
    def ReadSerialIput(self):
        buffer=""
        while self.running:
            if self.connection is None or self.wait ==True:
                time.sleep(1)
                continue
            try:
                if not self.connection.isOpen():
                    continue
                else:
                    char = self.connection.read().decode()

                    #receive testlog
                    if char == "+":
                         sys.stdout.write(self.ReadSerialIputUntilChar(";"))
                         sys.stdout.write("\n")
                         sys.stdout.flush()
                    else:
                        buffer= self.connection.readline().decode()
                        buffer = char + buffer
                        #print everything received if enableDebugOutputGlobal == true
                        if self.enableDebugOutputGlobal:
                            sys.stdout.write(buffer)
                            sys.stdout.flush()
            except Exception as ex:
                time.sleep(1)
                continue

    def StartThread(self):
        self.running = True
        #close thread as soon as main thread finishes
        self.readThread.daemon = True
        self.readThread.start()

    def StopThread(self):
        self.running = False

    #send to arduino
    def SendCommands(self,cmds):
        if self.connection != None and self.connection.isOpen():
            for cmd in cmds:
                self.connection.write(cmd.encode())
                if(len(cmds) > 1):
                    self.connection.flush()
                    time.sleep(0.2)
            return True
        else:
            return False

    #get available ports
    def GetAvailablePorts(self):
        return list(serial.tools.list_ports.comports())