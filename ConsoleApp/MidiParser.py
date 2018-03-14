#!/usr/bin/env python
"""
Open a MIDI file and print every message in every track.
Support for MIDI files is still experimental.
"""
import sys
import os
from mido import MidiFile
import math
import copy
from time import time

from FloppySequences import TestStep, Direction, FloppySequences

# Parse Midi File to Floppy Sequence
class MidiParser:
    tune_factor = 1
    arduino_freq = 10000 # Arduino Timer1 Frequency
    midi_file = None
    channels = 0
    serial = None

    MAXLEN = 50 # Maximum number of tones

    def __init__(self,filename, serial):
        if not os.path.isfile(filename):
            print("File not found " + filename)
            exit

        self.serial = serial
        self.midi_file = MidiFile(filename)
        self.channels = len(self.midi_file.tracks) - 1 # First Channel is for Meta-Information only I guess

    def convert_to_tone(self,n):
        # Convert time to 1/10 milliseconds
        time = round(n['time'] * self.arduino_freq)

        # Convert note from midi note number to frequency
        # https://newt.phys.unsw.edu.au/jw/notes.html
        frequency = math.pow(2,(n['note']/12)) * 440 if n['note'] >= 0 else 0 
        
        # Scale down frequency, so it can be played on a floppy drive, always double the factor to tune down in octaves
        while (frequency / self.tune_factor > 440):
            self.tune_factor*=2

        frequency /= self.tune_factor

        # Get time per period of frequency, set to zero if frequency is zero
        p = self.arduino_freq/frequency if frequency > 0 else 0

        # Get high and low times from period time, duty cycle 50%
        if(frequency > 0):
            hp = round(p/2) if round(p/2) > 0 else 1
            lp = hp
        else: # This is for pause
            hp = 0
            lp = 1

        # Calculate how much steps we need
        steps = round(time / (hp + lp))

        # Get TestStep Object
        return TestStep(steps,hp,lp,Direction.AUTO, 0)

    def parse(self, channel = 0,numberOfTracks=40):
        tones = {}

        # Tune down frequencies by factor to ensure it is playable by floppy drives
        # Reset when start parsing
        self.tune_factor = 1

        # Add Note to tone Array, do neccessary conversions
        def add_note(n):
            if(n['channel'] not in tones):
                tones[n['channel']] = []
            
            # Ignore notes where time equals zero
            if(n['time'] == 0): return
            
            step = self.convert_to_tone(n)

            # Done! Append it
            tones[n['channel']].append(step)
            
        m = {}
        current_time = 0
        for msg in self.midi_file:
            # Only process not events
            if not hasattr(msg,'note'):
                continue
            current_time += msg.time

            # Set initial values if new channel appears
            if(msg.channel not in m):
                m[msg.channel] = {}
                m[msg.channel]['time'] = 0
                m[msg.channel]['note'] = -1
                m[msg.channel]['channel'] = msg.channel

            # Process Note on and Note off events in order to transform them to frequency/time objects
            if msg.type == 'note_off':
                # Tone END
                m[msg.channel]['time'] = current_time - m[msg.channel]['time']
                add_note(m[msg.channel])

                # Pause START
                m[msg.channel]['time'] = current_time
                m[msg.channel]['note'] = -1

            if msg.type == 'note_on':
                # Pause END
                m[msg.channel]['time'] = current_time - m[msg.channel]['time']
                add_note(m[msg.channel])

                # Tone START
                m[msg.channel]['time'] = current_time
                m[msg.channel]['note'] = msg.note

        # Create Sequence for index 0 TODO: Let user select sequence id
        floppy = FloppySequences(0,"",False,False,numberOfTracks,False)
        floppy.steps = tones[channel] # TODO: Warn user if song is longer as maxlen
        return floppy

    def play(self, channel,numberOfTracks=40):
        m = {}
        self.tune_factor = 1
        for msg in self.midi_file.play():
            if(msg.channel != channel or not hasattr(msg, 'note')):
                continue
            if msg.type == 'note_on':
                m['time'] = time()
            elif  msg.type == 'note_off':
                diff = time() - m['time']

                # Extend time by 10%, it tends to sound better that way TODO: test and delete 
                m['time'] = diff
                m['note'] = msg.note

                tone = self.convert_to_tone(m)
                floppy = FloppySequences(0,"",False,False,numberOfTracks, True)
                floppy.steps = [tone]
                cmds = floppy.GetCommand()
                self.serial.SendCommands(cmds)
            else:
                 continue