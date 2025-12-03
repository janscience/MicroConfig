import sys
import threading

from time import sleep
from serial import Serial
from serial.serialutil import SerialException

from .version import __version__, __year__
from .discover import Discover, discover_teensy

        
def communicate(device, key_input):
    """ Read and write keyboard input to serial device.
    
    Parameters
    ----------
    device: str
        Name of serial device port.
    key_input: KeyboardInput
        Handles keyboard input and sends it to the serial device.
    """
    serial = Serial(device.device)
    serial.reset_input_buffer()
    serial.reset_output_buffer()
    key_input.set_serial(serial)
    while True:
        try:
            if serial.in_waiting > 0:
                x = serial.read(serial.in_waiting)
                if x == '\n':
                    sys.stdout.write(b'\n')
                else:
                    sys.stdout.write(x.decode('latin1'))
                sys.stdout.flush()
            sleep(0.01)
        except (OSError, SerialException):
            print()
            print()
            print('disconnected')
            break
    serial.close()


class KeyboardInput(threading.Thread):
    """ Background thread reading keyboard input and writing it so serial.
    """

    def __init__(self):
        super().__init__(daemon=True)
        self.serial = None
        self.start()

    def set_serial(self, serial):
        self.serial = serial

    def run(self):
        while True:
            x = input()
            if self.serial is not None:
                try:
                    self.serial.write(x.encode('latin1'))
                    self.serial.write(b'\n')
                    self.serial.flush()
                except (OSError, SerialException):
                    self.serial = None


def main():
    print(f'This is serialmonitor from microconfig version {__version__}, {__year__}.')
    print()
    devices = Discover(discover_teensy)
    key_input = KeyboardInput()
    while True:
        print()
        print('Waiting for Teensy device ...')
        while True:
            if devices.discover():
                sleep(0.5)
                break
        print('- found ', end='')
        devices.print()
        communicate(devices[0], key_input)
        sleep(1)
    

if __name__ == '__main__':
    main()

