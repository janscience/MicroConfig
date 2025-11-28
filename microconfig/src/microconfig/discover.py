"""# discover

Discover known devices connected to serial ports.

## Classes

`class Discover`: discover devices on serial ports.
`class SerialDevice`: a device connected to a serial port.

## Discover functions

These functions check whether a specific known device is connected to
a serial port. Pass these functions to the Discover class, for example
via its constructor.

- `discover_teensy()`: check whether a Teensy is connected to serial port.

"""

from serial.tools.list_ports import comports
from usb.core import find


class SerialDevice:
    """ A device connected to a serial port.

    Members
    -------
    device: str
        Serial port the device is connected to.
    model: str
        Name of the device.
    serial: int
        Serial number of the device.
    """
    
    def __init__(self, device, model, serial):
        self.device = device
        self.model = model
        self.serial = serial

        
class Discover:
    """Discover devices on serial ports.

    The Discover class is a list of known devices that you can iterate over.
    Its `len()` is the number of discovered devices, and a single device can
    be accessed via the `[]` operator.

    You need to pass a discover function to Discover via the
    constructor or the `add()` function. The discover functions decide
    whether a detected serial device is known. See `discover_teensy()`
    for an example of such a function.

    Parameters
    ----------
    args: list of functions
        Functions to be used for discovering devices.
        Each function gets an serial.tools.list_ports.ListPortInfo as
        the single argument. This object holds information about a
        serial port with the following fields: device, name,
        description, hwid, vid, oid, serial_number, location,
        manufacturer, product, and interface.
        The functions are supposed to return a model name for the
        discovered device as a string. If the device is not known,
        None should be returned.

    Members
    -------
    - `discover()`: check all serial ports for known devices.
    - `print()`: print discovered devices on console.

    """
    
    def __init__(self, *args):
        self.devices = []
        self.iter_counter = -1
        self.funcs = args

    def add(self, *args):
        """ Add discover functions.

        Parameters
        ----------
        args: list of functions
            Functions to be used for discovering devices.
            Each function gets an serial.tools.list_ports.ListPortInfo as
            the single argument. This object holds information about a
            serial port with the following fields: device, name,
            description, hwid, vid, oid, serial_number, location,
            manufacturer, product, and interface.
            The functions are supposed to return a model name for the
            discovered device as a string. If the device is not known,
            None should be returned.
        """
        self.funcs.append(func)

    def __len__(self):
        """ Number of discovered devices.
        """
        return len(self.devices)

    def __getitem__(self, index):
        """ A single device.

        Returns
        -------
        device: SerialDevice
            Device at index.
        """
        return self.devices[index]
    
    def __iter__(self):
        """Initialize iteration over devices.
        """
        self.iter_counter = -1
        return self

    def __next__(self):
        """Next device.

        Returns
        -------
        device: SerialDevice
            A discovered device.
        """
        self.iter_counter += 1
        if self.iter_counter >= len(self.devices):
            raise StopIteration
        else:
            return self.devices[self.iter_counter]

    def discover(self):
        """ Check all serial ports for known devices.

        Returns
        -------
        success: bool
            True if at least one device was discovered.
        """
        self.devices = []
        for port in sorted(comports(False)):
            for func in self.funcs:
                model = func(port)
                if model is not None:
                    dev = SerialDevice(port.device, model,
                                       port.serial_number)
                    self.devices.append(dev)
                    break
        return (len(self.devices) > 0)

    def print(self):
        """ Print discovered devices on console.
        """
        for dev in self:
            print(f'{dev.model} with serial number {dev.serial} on {dev.device}')


def discover_teensy(port):
    """ Check whether a Teensy is connected to serial port.

    Pass this function to an instance of the Discover class.

    Parameters
    ----------
    port: serial.tools.list_ports.ListPortInfo
        Information about a serial port with the following fields:
        device, name, description, hwid, vid, oid, serial_number,
        location, manufacturer, product, and interface.

    Returns
    -------
    model: str or None
        If port is known, name of the Teensy model, otherwise None.
    """
    
    # map bcdDevice of USB device to Teensy model version:
    teensy_model = {   
        0x274: '30',
        0x275: '31',
        0x273: 'LC',
        0x276: '35',
        0x277: '36',
        0x278: '40 beta',
        0x279: '40',
        0x280: '41',
        0x281: 'MM'}

    if port.vid is None and port.pid is None:
        return None
    #if port.vid == 0x16C0 and port.pid in [0x0483, 0x048B, 0x048C, 0x04D5]:
    if port.manufacturer == 'Teensyduino':
        dev = find(idVendor=port.vid, idProduct=port.pid,
                   serial_number=port.serial_number)
        if dev is None:
            print('ERROR! Failed to retrieve USB device information.')
            # this happens when we do not have permissions for the device!
            return None
        else:
            return 'Teensy' + teensy_model[dev.bcdDevice]
    else:
        return None

