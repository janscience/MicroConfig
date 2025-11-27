"""
# discover

Discover Teensys connected to USB ports.

## Functions

- `discover_teensy_ports()`: check all serial ports for Teensys.
- `print_teensys()`: print detected Teensys on console.

"""

from serial.tools.list_ports import comports
from usb.core import find


def get_teensy_model(vid, pid, serial_number):
    """ Figure out the teensy model that is connected to USB.

    Parameters
    ----------
    vid: int
        Vendor id of the device.
    pid: int
        Product id of the device.
    serial_number: int
        Serial number of the device.

    Returns
    -------
    model: str
        Name of the Teensy model.
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

    dev = find(idVendor=vid, idProduct=pid,
               serial_number=serial_number)
    if dev is None:
        print('ERROR! Failed to retrieve USB device information.')
        # this happens when we do not have permissions for the device!
        # we better throw an exception ...
        return ''
    else:
        return teensy_model[dev.bcdDevice]


def discover_teensy_ports():
    """ Check all serial ports for Teensys.

    Returns
    -------
    devices: list of str
        Names of serial ports to which a Teensy is connected.
    models: list of str
        Corresponding names of Teensy models.
    serial_numbers: list of int
        Corresponding serial numbers of the Teensys.
    """
    devices = []
    serial_numbers = []
    models = []
    for port in sorted(comports(False)):
        if port.vid is None and port.pid is None:
            continue
        #if port.vid == 0x16C0 and port.pid in [0x0483, 0x048B, 0x048C, 0x04D5]:
        if port.manufacturer == 'Teensyduino':
            teensy_model = get_teensy_model(port.vid, port.pid,
                                            port.serial_number)
            # TODO: we should also check for permissions!
            devices.append(port.device)
            serial_numbers.append(port.serial_number)
            models.append(teensy_model)
    return devices, models, serial_numbers

    
def print_teensys(devices, models, serial_numbers):
    """ Print detected Teensys on console.

    Parameters
    ----------
    devices: list of str
        Names of serial ports to which a Teensy is connected.
    models: list of str
        Corresponding names of Teensy models.
    serial_numbers: list of int
        Corresponding serial numbers of the Teensys.
    """
    for dev, model, num in zip(devices, models, serial_numbers):
        print(f'Teensy{model} with serial number {num} on {dev}')

