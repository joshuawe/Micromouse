import serial
import json

class SerialHandler():
    """This class handles a serial port, its incoming and outgoing messages.
    If the information is passed via a Bluetooth module, please make sure, that the BT device and your computer are paired already.
    """
    def __init__(self, config_path="config.json", profile="Josh"):
        print("Started SerialHandler.")
        # Read in the config settings
        self.read_config(config_path, profile)
        print(f"Connection settings -> port: {self.profile['port']}, baudrate: {self.profile['baudrate']}, timeout: {self.profile['timeout']}", end="", flush=True)        
        #self.connect()
        
    def read_config(self, config_path:str, profile:str):
        """The configuration is read in from a config file seciefied in the config_path. This file is a JSON file. 
        The root nodes are the profiles for different users/computers.

        Args:
            config_path (string): Path to config file.
            profile (string): Name of profile to be used. 
        """
        # Opening & reading JSON file
        f = open(config_path)
        data = json.load(f)
        f.close()
        self.profile = data["profiles"][profile]
        self.settings = data


    def connect(self):
        """Connect to the Serial port.
        """
        print(" ... connecting", end="")
        port = self.profile["port"]
        baudrate = self.profile["baudrate"]
        timeout = self.profile["timeout"]
        self.serial = serial.Serial(port, baudrate, timeout=timeout)
        self.serial.flushInput()
        print(" -> connected")


    def parseLines(self, lines:'list[str]', verbose=False):
        """Takes a list of strings and checks if it understands each line.
            A message could look like this (depending on format defined in config file):
                      signature:varName:varValue
                        |          |        |
                 message[0]  message[1]  message[2]

        Args:
            lines (list[str]): List of lines to check.
            verbose (boolean): Wether conosle prints for debugging should be executed.
        """
        delimiter = self.settings["variableParse"]["delimiter"]
        splittings = self.settings["variableParse"]["format"].count(delimiter) + 1 # The number of individual parts of the message
        signature = self.settings["variableParse"]["signature"]
        varKeys = self.settings["variables"].keys()
        newVars = dict()
        # Go through all the lines
        for i,line in enumerate(lines):
            consoleMsg = f"Line {i}\t"
            message = line.strip()
            message = message.split(delimiter)
            if (len(message) != splittings):
                # Invalid, could be that a delimiter was used inside the variable content
                consoleMsg +=  f"Invalid input, too many delimiters: '{line}'"
            elif (message[0] != signature):
                # Invalid signature
                consoleMsg += f"Invalid, Wrong signature, Expected {signature}, got {message[0]}: '{line}'"
            elif message[1] not in varKeys:
                # Invalid variable name
                consoleMsg += f"Invalid variable name: '{line}'"
            else:
                # parsing succesful
                if message[1] not in newVars.keys():
                    newVars[message[1]] = list() # if it is the first entry, create a list
                newVars[message[1]].append(message[2])

                consoleMsg += f"Correct parsing: {message[1]} = {message[2]}"

        if verbose: 
            print(consoleMsg)
            print("All variables:")
            print(newVars)
        return newVars





    def collectLines(self, lines, consolePrint=False):
        """Print X many lines from the serial. Connection has to be established already.

        Args:
            lines (int): Number of lines to be read.

        Returns:
            list(str): All the lines that were read.
        """
        messages = list()
        for i in range(lines):
            serial_bytes = self.serial.readline()
            message = serial_bytes.decode()
            if consolePrint: print(message, end="")
            messages.append(message)

        return messages





if __name__ == "__main__":
    print("Starting")
    communicator = SerialHandler("BT_communication\config.json")
    communicator.connect()
    lines = communicator.collectLines(10)

    varDict = dict()
    maxlen = communicator.settings["dash"]["dequeMaxlen"]
    for key in communicator.settings["variables"]:
        from collections import deque
        varDict[key] = deque(maxlen=maxlen)
    
    newVarDict = communicator.parseLines(lines)

    for key in newVarDict.keys():
        if key in varDict.keys():
            varDict[key].extend(newVarDict[key])

    import numpy as np
    X = np.arange(-len(varDict['dFront']), 0)
    Y = varDict['dFront']
    print(f"X: {X}")
    print(f"Y: {Y}")
    print(f"min(x), max(x): {min(X)}, {max(X)}")
    print(f"min(Y), max(Y): {min(Y)}, {max(Y)}")
