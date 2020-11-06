'''
Convert ArduinoBLE debug files into Btsnoop files ready to be analyzed using wireshark or hcidump
Btsnoop file format reference
 https://www.fte.com/WebHelpII/Sodera/Content/Technical_Information/BT_Snoop_File_Format.htm
'''

import  os
import argparse

DEBUG = False

parser = argparse.ArgumentParser()
parser.add_argument('-i', dest='inputPath', type=str, required=True, help='input file containing debug log')
parser.add_argument('-o', dest='outputPath', type=str, required=True, help='result file that will contain the btsnoop encoded debug file')
args = parser.parse_args()

# Extract only hci debug messages
def extractHCIDebugPrint(inputPath, outputPath):
  inputFile = open(inputPath, 'r')
  outputFile = open(outputPath, 'w')
  for inputLine in inputFile:
    lineItems = inputLine.split()
    if (len(lineItems) < 7) or (lineItems[1] != "->") or (lineItems[2] != "HCI"):
      if (len(lineItems) < 4) or (lineItems[0] != "HCI") or ((lineItems[3] != "<-") and (lineItems[3] != "->")):
        continue
    outputFile.write(inputLine)  
  outputFile.close()

# Return packet in btsnoop format
def buildBinaryPacket(hciMessage, hciDirection, hciType):
  commandFlag = 1 if (hciType == "COMMAND" or hciType == "EVENT") else 0
  directionFlag =  0 if (hciDirection == "TX") else 1
  flagHex = ("0" * 7) + str((commandFlag * 2) + directionFlag)
  timestampHex = "0" * 16
  packetDropHex = "0" * 8
  dataLengthHex = format( (int(len(hciMessage) / 2)), 'x')
  packetLengthHex = ("0" * (8 - len(dataLengthHex))) + dataLengthHex
  binaryPacket = bytearray.fromhex(packetLengthHex + packetLengthHex + flagHex + packetDropHex + timestampHex + hciMessage)
  if DEBUG:
    print(len(hciMessage))
    print(dataLengthHex)
    print(packetLengthHex)
    print(flagHex)
    print('\n')
  return binaryPacket

def buildBinaryHeader():
  defaultHeader = "6274736e6f6f700000000001000003ea"
  binaryHeader = bytearray.fromhex(defaultHeader)
  return binaryHeader

def convertToBtsnoop(inputPath, outputPath):
  # Open output file and write the Btsnoop header
  outputFile = open(outputPath,'wb')
  header = buildBinaryHeader()
  outputFile.write(header)

  # Open input file containing HCI debug packets
  inputFile = open(inputPath, 'r')
  for inputLine in inputFile:
    lineItems = inputLine.split()
    # For a safer script, do not use indexes but look for symbols in the line
    baseIndex = lineItems.index("HCI")
    hciMessage = lineItems[baseIndex + 4]
    hciDirection = lineItems[baseIndex + 2]
    hciType = lineItems[baseIndex + 1]
    # Build and write the encoded line
    btsnoopPacket = buildBinaryPacket(hciMessage, hciDirection, hciType)
    outputFile.write(btsnoopPacket)
    if DEBUG:
      print(hciDirection)
      print(hciMessage)
      print(hciType)
      print('\n')
  outputFile.close()

inputPath = args.inputPath
outputPath = args.outputPath
tempFile = "temp-debug-print.txt"
# Run
extractHCIDebugPrint(inputPath,tempFile)
convertToBtsnoop(tempFile, outputPath)
# Delete temp file
os.remove(tempFile) 

