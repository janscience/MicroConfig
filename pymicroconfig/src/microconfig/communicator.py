""" # communicator

- `class Communicator`: basic infrastructure for interacting with the MicroConfig menu of the microcontroller.
"""

from serial import Serial
from serial.serialutil import SerialException


class Communicator:
    """Basic infrastructure for interacting with the MicroConfig menu of the microcontroller.

    A GUI exposing the MicroConfig menu should inherit the Communicator
    class and implement a number of member functions.

    TODO...

    """
    
    def __init__(self, device):
        self.device = device.device
        self.serial = None
        self.read_count = 0
        self.read_state = 0
        self.startup_input = []
        self.input = []
        self.read_func = None
        self.request_stack = []
        self.request_block = False
        self.request_type = None
        self.request_target = None
        self.request_ident = None
        self.request_start = None
        self.request_end = None
        self.request_stop = None
        self.request_stop_index = None

        self.menu = {}
        self.menu_iter = []
        self.menu_ids = []
        self.menu_key = None
        self.menu_item = None
        
    def parse_halt(self, k):
        """ Reimplement and call this.
        """
        self.read_func = self.parse_idle
        
    def parse_startup(self):
        """ Reimplement.
        """
        pass
            
    def setup(self):
        """ Reimplement and call this.
        """
        if 'Help' in self.menu:
            self.menu.pop('Help')

    def ask(self, stream):
        """ Reimplement.
        """
        return True
                
    def start(self, device):
        """ Reimplement.

        Call this at end and then start a timer that calls
        read() periodically every few milliseconds.
        """
        try:
            self.serial = Serial(self.device)
            self.serial.reset_input_buffer()
            self.serial.reset_output_buffer()
        except (OSError, SerialException):
            self.serial = None
        self.input = []
        self.read_count = 0
        self.read_state = 0
        self.read_func = self.read_startup

    def stop(self):
        """ Reimplement and call this.
        """
        if self.serial is not None:
            self.serial.close()
        self.serial = None

    def write(self, text):
        if self.serial is not None:
            try:
                self.serial.write(text.encode('latin1'))
                self.serial.write(b'\n')
                self.serial.flush()
            except (OSError, SerialException):
                self.stop()

    def parse_idle(self):
        pass
        
    def read_startup(self):
        while len(self.input) > 0:
            s = self.input.pop(0)
            self.startup_input.append(s)
            if 'HALT' in s:
                self.parse_halt(len(self.input) - 1)
                return
            elif s[:20] == 20*':':
                self.input = []
                self.parse_startup()
                self.read_func = self.configure_menu
                return
        if len(self.startup_input) < 10:
            if self.read_count > 100:
                self.read_count = 0
                self.write('reboot')
            else:
                self.read_count += 1

    def configure_menu(self):
        if self.read_state == 0:
            self.write('detailed on')
            self.read_state += 1
        elif self.read_state == 1:
            self.write('echo off')
            self.read_state += 1
        elif self.read_state == 2:
            self.write('mode both')
            self.read_state = 0
            self.read_func = self.parse_mainmenu

    def parse_menu(self, title_str):
        menu_start = None
        menu_end = None
        for k in range(len(self.input)):
            if 'HALT' in self.input[k]:
                self.parse_halt(k)
                return
            elif title_str + ':' in self.input[k]:
                menu_start = k
            elif menu_start is not None and \
                 'Select' in self.input[k]:
                menu_end = k
        if menu_start is None or menu_end is None:
            return {}
        menu = {}
        for l in self.input[menu_start + 1:menu_end]:
            x = l.split()
            num = x[0][:-1] if x[0][-1] == ')' else ''
            if x[-1] == '...':
                # sub menu:
                name = ' '.join(x[1:-1])
                menu[name] = (num, 'menu', {})
            else:
                if num:
                    l = ' '.join(x[1:])
                if ':' in l:
                    # parameter:
                    x = l.split(':')
                    name = x[0].strip()
                    value = x[1].strip()
                    menu[name] = [num, 'param', value]
                else:
                    # action:
                    menu[l] = (num, 'action')
        self.input = []
        return menu

    def parse_mainmenu(self):
        if self.read_state == 0:
            self.clear_input()
            self.write('print')
            self.read_state += 1
        elif self.read_state == 1:
            self.menu = self.parse_menu('Menu')
            if len(self.menu) > 0:
                self.menu_iter = [iter(self.menu.items())]
                self.menu_ids = [None]
                self.read_state = 0
                self.read_func = self.parse_submenus
                
    def parse_submenus(self):
        if self.read_state == 0:
            # get next menu entry:
            try:
                if len(self.menu_iter) == 0:
                    exit()
                self.menu_key, self.menu_item = next(self.menu_iter[-1])
                self.menu_ids[-1] = self.menu_item[0]
                if self.menu_item[1] == 'menu':
                    self.read_state = 10
                elif self.menu_item[1] == 'param':
                    self.read_state = 20
            except StopIteration:
                self.menu_iter.pop()
                self.menu_ids.pop()
                if len(self.menu_iter) == 0:
                    self.write('gui on')
                    self.setup()
                    self.clear_input()
                    self.read_func = self.parse_request_stack
                else:
                    self.write('q')
        elif self.read_state == 10:
            # request submenu:
            self.clear_input()
            self.write(self.menu_item[0])
            self.read_state += 1
        elif self.read_state == 11:
            # parse submenu:
            submenu = {}
            if len(self.input) > 1 and 'Select' in self.input[-1]:
                submenu = self.parse_menu(self.menu_key)
            if len(submenu) > 0:
                self.menu_item[2].update(submenu)
                self.menu_iter.append(iter(self.menu_item[2].items()))
                self.menu_ids.append(None)
                self.read_state = 0
        elif self.read_state == 20:
            if len(self.menu_item[0]) == 0:
                # constant string parameter:
                pargs = (list(self.menu_ids), str(self.menu_key),
                         str(self.menu_item[2]), 'A, string 128', [])
                self.menu_item[2] = pargs
                self.read_state = 0
            else:
                # request parameter:
                self.clear_input()
                self.write(self.menu_item[0])
                self.read_state += 1
        elif self.read_state == 21:
            # parse parameter:
            list_start = None
            list_end = None
            for k in range(len(self.input)):
                if list_start is None and \
                   self.input[k].lower().startswith(self.menu_key.lower()):
                    list_start = k + 1
                elif list_end is None and \
                     'new value' in self.input[k].lower() and \
                     self.input[k].rstrip()[-1] == ':':
                    list_end = k
            if list_start is None or list_end is None:
                return
            s = self.input[list_end]
            i = s.find('new value')
            s = s[i + s[i:].find('(') + 1:s.find('):')]
            pargs = (list(self.menu_ids), str(self.menu_key),
                     str(self.menu_item[2]), s,
                     list(self.input[list_start:list_end]))
            self.menu_item[2] = pargs
            self.write('keepthevalue')
            self.read_state = 0
    
    def parse_request_stack(self):
        if len(self.request_stack) == 0:
            if self.request_type is None:
                self.request_block = False
            return
        self.clear_input()
        request = self.request_stack.pop(0)
        self.request_target = request[0]
        self.request_ident = request[1]
        self.request_start = request[2]
        self.request_end = request[3]
        self.request_stop = request[4]
        if not isinstance(self.request_stop, (list, tuple)):
            self.request_stop = (self.request_stop, )
        self.request_stop_index = None
        self.request_type = request[5]
        self.read_state = 0
        if self.request_type in ['read', 'transmit']:
            self.read_func = self.parse_read_request
        else:
            self.read_func = self.parse_write_request
        self.read_func()

    def read_request(self, target, ident, start, stop, act='read'):
        if len(start) == 0:
            return
        # put each request only once onto the stack:
        for req in self.request_stack:
            if req[0] == target and req[1] == ident and req[-1] == act:
                return
        block = self.request_block
        if start[-1] == 'STAY':
            start.pop()
            end = False
            self.request_block = True
            block = False
        else:
            end = True
        if not block:
            self.request_stack.append([target, ident, start, end, stop, act])
        if self.read_func == self.parse_request_stack:
            self.parse_request_stack()
            
    def transmit_request(self, target, ident, start):
        stop = ['select', 'new value']
        self.read_request(target, ident, start, stop, 'transmit')

    def parse_read_request(self):
        if self.read_state == 0:
            # write menu entry:
            self.clear_input()
            self.write(self.request_start[0])
            self.request_start.pop(0)
            if len(self.request_start) > 0:
                self.read_state = 1
            else:
                self.request_start = None
                self.read_state = 2
        elif self.read_state == 1:
            # wait for next menu:
            stop_str = 'select'
            if self.request_type == 'transmit' and len(self.request_start) == 1:
                stop_str = 'new value'
            if len(self.input) > 0 and \
               stop_str in self.input[-1].lower():
                self.read_state = 0
        elif self.read_state == 2:
            # erase empty line in input:
            while len(self.input) > 0 and len(self.input[0].strip()) == 0:
                del self.input[0]
            self.read_state = 3
        elif self.read_state == 3:
            # ask question:
            if self.request_type == 'read' and len(self.input) > 0 and \
               self.input[-1].lower().endswith(' [y/n] '):
                self.ask(self.input)
                self.read_state = 3
            # finish input:
            elif self.request_stop is None or \
               len(self.request_stop) == 0:
                self.read_state = 4
            # check for stop string:
            elif len(self.input) > 0:
                last_line = self.input[-1].lower()
                if len(last_line.strip()) == 0 and len(self.input) > 1:
                    last_line = self.input[-2].lower()
                for k in reversed(range(len(self.request_stop))):
                    if self.request_stop[k] in last_line:
                        self.request_stop = None
                        self.request_stop_index = k
                        self.read_state = 4
                        break
            # process input:
            if self.request_ident[:3] == 'run' and \
               self.request_target is not None and \
               self.request_stop_index != 0:
                    self.request_target.read(self.request_ident,
                                             self.input,
                                             self.request_stop_index == 0)
        elif self.read_state == 4:
            # final processing of input:
            if self.request_target is not None:
                if self.request_stop_index == 0:
                    if len(self.input) > 0:
                        del self.input[-1]
                    if len(self.input) > 0 and len(self.input[-1].strip()) == 0:
                        del self.input[-1]
                if self.request_ident[:3] != 'run':
                    while len(self.input) > 0 and len(self.input[0].strip()) == 0:
                        del self.input[0]
                self.request_target.read(self.request_ident,
                                         self.input,
                                         self.request_stop_index == 0)
                self.request_target = None
            if self.request_type == 'transmit' and self.request_stop_index == 1:
                self.write('keepthevalue')
            self.read_state = 5
        elif self.read_state == 5:
            # go back to root menu:
            self.clear_input()
            if self.request_end:
                self.write('h')
            self.request_type = None
            self.read_func = self.parse_request_stack

    def write_request(self, msg, start):
        if len(start) == 0:
            return
        if not self.request_block:
            self.request_stack.append([msg, None, start,
                                       True, None, 'write'])
        if self.read_func == self.parse_request_stack:
            self.parse_request_stack()

    def parse_write_request(self):
        if self.read_state == 0:
            self.clear_input()
            if len(self.request_start) > 0:
                self.write(self.request_start[0])
                self.request_start.pop(0)
            else:
                self.request_start = None
                self.read_state += 1
                if not self.request_target:
                    self.read_state += 1
        elif self.read_state == 1:
            self.clear_input()
            self.write(self.request_target)
            self.request_target = None
            self.read_state += 1
        elif self.read_state == 2:
            self.clear_input()
            if self.request_end:
                self.write('h')
            self.request_type = None
            self.read_func = self.parse_request_stack

    def read(self):
        """Read from serial stream.

        And call self.read_func() for processing the read in data.
        """
        if self.serial is None:
            try:
                self.serial = Serial(self.device)
                self.serial.reset_input_buffer()
                self.serial.reset_output_buffer()
            except (OSError, SerialException):
                self.serial = None
                self.stop()
                return
        try:
            if self.serial.in_waiting > 0:
                # read in incoming data:
                x = self.serial.read(self.serial.in_waiting)
                lines = x.decode('utf8').split('\n')
                if len(self.input) == 0:
                    self.input = ['']
                self.input[-1] += lines[0].rstrip('\r')
                for l in lines[1:]:
                    self.input.append(l.rstrip('\r'))
            else:
                # execute requests:
                self.read_func()
        except (OSError, SerialException):
            self.stop()
            
    def clear_input(self):
        if self.serial is not None:
            try:
                self.serial.reset_input_buffer()
            except (OSError, SerialException):
                self.stop()
        self.input = []
        
