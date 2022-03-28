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
        print(f"Connection will be establised to port: {self.settings['port']}, baudrate: {self.settings['baudrate']}", end="", flush=True)        
        self.connect()
        
    def read_config(self, config_path, profile):
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
        self.settings = data[profile]

    def connect(self):
        port = self.settings["port"]
        baudrate = self.settings["baudrate"]
        timeout = self.settings["timeout"]
        self.serial = serial.Serial(port, baudrate, timeout=timeout)
        self.serial.flushInput()
        print(" -> connected")

    def print_lines(self, lines):

        for i in range(lines):
            serial_bytes = self.serial.readline()
            message = serial_bytes.decode()
            print(message, end="")





if __name__ == "__main__":
    print("Starting")
    obj = SerialHandler("BT_communication\config.json")
    obj.print_lines(10)

    obj.serial.readlines()
