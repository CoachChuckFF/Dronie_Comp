#!/usr/bin/env python3

import sys
import os
import re

# ------------ Parameters ----------------

FileName = "output.bpf"
ShowName = "Test Show"

if(len(sys.argv) > 2):
    ShowName = sys.argv[2]

if(len(sys.argv) > 1):
    FileName = sys.argv[1]


# ------------ Defines ------------------------

#Endian
Endian = "little"

#Magic
HeaderMagic = "Blizzard"
FrameMagic = "Fram"
FooterMagic = "This is the end."

#Sizes
TotalFrameByteLength = 4
TotalTimeByteLength = 4
StartingDelayByteLength = 4
ShowNameByteLength = 64
DMXArrayByteLength = 512

FrameDelayByteLength = 4
FrameDiffCountByteLength = 2

FullFrameDiffs = 0xFFFF
FrameDMXValueByteLength = 1

FrameDiffAddressByteLength = 2
FrameDiffVauleByteLength = 2


#Offsets
TotalFrameOffset = 8
TotalTimeOffset = 12
StartingDelayOffset = 16
ShowNameOffset = 20
StartingDMXOffset = 84
Frame1Offset = 596



# ------------ Writing the Show ----------------

FileHandle = open(FileName, "wb")
TotalTime = 0 #ms
TotalFrames = 0
FirstFrameWritten = False
FirstFrame = []

def writeShow():
    print("Writing Show")
    writeHeader()

def writeHeader():
    print("Writing Header")

    FileHandle.write(str.encode(HeaderMagic)) #Magic

    zero = 0
    nullByte = zero.to_bytes(1, Endian)
    

    for i in range(TotalFrameByteLength): #Frames
        FileHandle.write(nullByte)
    for i in range(TotalTimeByteLength): #Total Time
        FileHandle.write(nullByte)
    
    startingDelay = 100 #ms
    FileHandle.write(startingDelay.to_bytes(4, Endian))

    FileHandle.write(str.encode(ShowName)) #Show Name
    for i in range(ShowNameByteLength - len(ShowName)): 
        FileHandle.write(nullByte)

    for i in range(DMXArrayByteLength): #Starting DMX Frame
        FileHandle.write(nullByte)
    
    writeBody()


def writeBody():
    print("Writing Body")
    writeCustomShow()
    writeFooter()

def shouldWriteFirstFrame():
    global FirstFrameWritten
    if not FirstFrameWritten:
        FirstFrameWritten = True
        return True
        
    return False


def writeFullFrame(delay, dmx):
    global TotalFrames
    global TotalTime

    writeFirstFrame = shouldWriteFirstFrame()

    TotalFrames += 1
    TotalTime += delay

    diffCount = FullFrameDiffs

    FileHandle.write(str.encode(FrameMagic)) #Magic
    FileHandle.write(delay.to_bytes(FrameDelayByteLength, Endian)) #Delay
    FileHandle.write(diffCount.to_bytes(FrameDiffCountByteLength, Endian)) #Diff Count

    for i in range(DMXArrayByteLength):
        value = 0
        if(i < len(dmx)):
            value = dmx[i]

        if writeFirstFrame:
            FirstFrame.append(value)

        FileHandle.write(value.to_bytes(FrameDMXValueByteLength, Endian))


def writeFrame(delay, changeAddresses, changeValues):
    global TotalFrames
    global TotalTime

    writeFirstFrame = shouldWriteFirstFrame()

    TotalFrames += 1
    TotalTime += delay

    FileHandle.write(str.encode(FrameMagic)) #Magic
    FileHandle.write(delay.to_bytes(FrameDelayByteLength, Endian)) #Delay
    FileHandle.write(len(changeAddresses).to_bytes(FrameDiffCountByteLength, Endian)) #Diff Count

    if writeFirstFrame:
        for i in range(DMXArrayByteLength):
            FirstFrame.append(0)

    for i in range(len(changeAddresses)):
        if changeAddresses[i] != 0:
            FileHandle.write(changeAddresses[i].to_bytes(FrameDiffAddressByteLength, Endian)) #Address
            FileHandle.write(changeValues[i].to_bytes(FrameDiffVauleByteLength, Endian)) #Value 

            if writeFirstFrame:
                FirstFrame[changeAddresses[i] - 1] = changeValues[i]


def writeFooter():
    print("Writing Footer")
    FileHandle.write(str.encode(FooterMagic))
    updateHeader()

def updateHeader():
    print("Updating Header")
    FileHandle.seek(TotalFrameOffset, 0)
    FileHandle.write(TotalFrames.to_bytes(TotalFrameByteLength, Endian))

    FileHandle.seek(TotalTimeOffset, 0)
    FileHandle.write(TotalTime.to_bytes(TotalTimeByteLength, Endian))

    FileHandle.seek(StartingDMXOffset, 0)
    for i in range(DMXArrayByteLength):
        value = 0
        if(i < len(FirstFrame)):
            value = FirstFrame[i]

        FileHandle.write(value.to_bytes(FrameDMXValueByteLength, Endian))

    finish()

def finish():
    print("Finishing Up")
    FileHandle.close()

#------------- Custom Show Functions ----------

def writeCustomShow():

    atFull = []
    redrum = []

    for i in range(DMXArrayByteLength): 
        atFull.append(0xFF)
        redrum.append(0x00)


    #Blink All
    # writeFullFrame(50, atFull)
    # writeFullFrame(50, redrum)
    # writeFullFrame(50, atFull)
    # writeFullFrame(50, redrum)
    # writeFullFrame(50, atFull)
    # writeFullFrame(50, redrum)

    #Rainbow
    rainbowShow()

    #For Large Show
    # for i in range(300000):
    #     rainbowShow()




def rainbowShow():
    for i in range(256):
        j = i

        #Addresses are 1 based
        addresses = [1, 2, 3]
        changes = [0, 0, 0]

        if i < 85:
            changes[0] = j*3
            changes[1] = 255 - j*3
            changes[2] = 0
        elif i < 170:
            j-= 85
            changes[0] = 255 - j*3
            changes[1] = 0
            changes[2] = j*3
        else:
            j-= 170
            changes[0] = 0
            changes[1] = j*3
            changes[2] = 255 - j*3

        writeFrame(10, addresses, changes)


#------------- Final Call ----------

writeShow()




