from serial import Serial
from serial.serialutil import SerialException

from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QFont
from PyQt5.QtWidgets import QMainWindow, QMessageBox
from PyQt5.QtWidgets import QHBoxLayout, QVBoxLayout, QGridLayout
from PyQt5.QtWidgets import QSpacerItem, QSizePolicy
from PyQt5.QtWidgets import QStackedWidget, QLabel
from PyQt5.QtWidgets import QWidget, QFrame, QRadioButton

from .version import __version__
from .configactions import ConfigActions
from .parameter import Parameter
from .terminal import Terminal


class SoftwareInfo(QLabel):
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setAlignment(Qt.AlignRight | Qt.AlignVCenter)

    def set(self, stream):
        text = '<style type="text/css"> th, td { padding: 0 15px; }</style>'
        text += '<table>'
        libs = False
        n = 0
        for s in stream:
            s = s.strip()
            if len(s) > 0:
                if n == 0:
                    text += f'<tr><td colspan=2><b>{s}</b></td></tr>'
                else:
                    if not libs:
                        text += '<tr><td>based on</td>'
                        libs = True
                    else:
                        text += '<tr><td></td>'
                    s = s.replace('based on ', '')
                    s = s.replace('and ', '')
                    text += f'<td><b>{s}</b></td></tr>'
                n += 1
        text += '</table>'
        self.setText(text)

        
class MicroConfig(QMainWindow):
    
    def __init__(self, title, device, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.title = title
        self.setWindowTitle(f'{self.title} {__version__}: {device.device}')

        self.logo = QLabel(self)
        self.logo.setFont(QFont('monospace'))
        self.softwareinfo = SoftwareInfo(self)
        logoboxw = QWidget(self)
        logobox = QHBoxLayout(logoboxw)
        logobox.addWidget(self.logo)
        logobox.addWidget(QLabel())
        logobox.addWidget(self.softwareinfo)
        
        self.msg = QLabel(self)
        
        self.conf = QFrame(self)
        self.conf.setFrameStyle(QFrame.Panel | QFrame.Sunken)
        self.configuration = QGridLayout(self.conf)
        self.config_parames = []
        self.config_file = QLabel()
        self.config_status = QLabel()
        self.config_status.setTextFormat(Qt.RichText)
        self.config_status.setToolTip('Indicates presence of configuration file')
        self.user_button = None
        self.admin_button = None
        self.configacts = ConfigActions(self)
        self.configacts.sigReadRequest.connect(self.read_request)
        self.configacts.sigWriteRequest.connect(self.write_request)
        self.configacts.sigDisplayTerminal.connect(self.display_terminal)
        self.configacts.sigDisplayMessage.connect(self.display_message)
        self.configacts.sigVerifyParameter.connect(self.verify_parameter)
        self.configacts.sigSetParameter.connect(self.set_parameter)
        self.configacts.sigConfigFile.connect(self.set_configfile_state)
        self.configacts.sigShowStartup.connect(self.show_startup)
                
        self.boxw = QWidget(self)
        self.box = QHBoxLayout(self.boxw)
        self.box.addWidget(self.configacts)
        self.box.addWidget(self.conf)
        
        self.term = Terminal(self)
        self.term.done.clicked.connect(lambda x: self.stack.setCurrentWidget(self.boxw))
        
        self.stack = QStackedWidget(self)
        self.stack.addWidget(self.msg)
        self.stack.addWidget(self.boxw)
        self.stack.addWidget(self.term)
        self.stack.setCurrentWidget(self.msg)
        
        box = QWidget(self)
        vbox = QVBoxLayout(box)
        vbox.addWidget(logoboxw)
        vbox.addWidget(self.stack)
        self.setCentralWidget(box)

        self.device = device.device
        self.ser = None
        self.read_timer = QTimer(self)
        self.read_timer.timeout.connect(self.read)
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

    def activate(self, device):
        self.msg.setText('Reading configuration ...')
        self.msg.setAlignment(Qt.AlignCenter)
        self.stack.setCurrentWidget(self.msg)
        try:
            self.ser = Serial(self.device)
            self.ser.reset_input_buffer()
            self.ser.reset_output_buffer()
        except (OSError, SerialException):
            self.ser = None
        self.input = []
        self.read_count = 0
        self.read_state = 0
        self.read_func = self.read_startup
        self.read_timer.start(2)

    def write(self, text):
        if self.ser is not None:
            try:
                self.ser.write(text.encode('latin1'))
                self.ser.write(b'\n')
                self.ser.flush()
            except (OSError, SerialException):
                self.stop()

    def stop(self):
        self.read_timer.stop()
        if self.ser is not None:
            self.ser.close()
        self.ser = None
        self.close()

    def display_terminal(self, title, text):
        self.term.display(title, text)
        self.stack.setCurrentWidget(self.term)

    def display_message(self, text):
        if isinstance(text, (tuple, list)):
            text = '\n'.join(text)
        QMessageBox.information(self, self.title, text)

    def ask(self, stream):
        default = '[Y/' in stream[-1]
        stream[-1] = stream[-1][:stream[-1].lower().find(' [y/n] ')]
        r = QMessageBox.question(self, self.title, '\n'.join(stream),
                                 QMessageBox.Yes | QMessageBox.No,
                                 QMessageBox.Yes if default
                                 else QMessageBox.No )
        self.clear_input()
        if r == QMessageBox.Yes:
            self.write('y')
        else:
            self.write('n')

    def show_startup(self):
        self.display_terminal('Startup messages', self.startup_input)

    def set_mode(self, checked):
        mode = 'A' if self.admin_button.isChecked() else 'U'
        self.configacts.set_mode(mode)        
        title_widget = None
        set_focus = True
        n = 0
        for p in self.config_params:
            if title_widget != p.title_widget:
                if title_widget is not None:
                    title_widget.setVisible(n > 0);
                title_widget = p.title_widget
                n = 0
            v = p.set_mode(mode)
            if set_focus and v:
                p.edit_widget.setFocus(Qt.MouseFocusReason)
                set_focus = False                
            if v:
                n += 1
        if title_widget is not None:
            title_widget.setVisible(n > 0);
        
    def find_parameter(self, keys, menu):
        found = False
        for mk in menu:
            if keys[0] == mk:
                found = True
                menu_item = menu[mk]
                if len(keys) > 1:
                    if menu_item[1] == 'menu':
                        p = self.find_parameter(keys[1:], menu_item[2])
                        if p is not None:
                            return p
                        else:
                            found = False
                elif menu_item[1] == 'param':
                    return menu_item[2]
                else:
                    return None
                break
        if not found:
            for mk in menu:
                menu_item = menu[mk]
                if menu_item[1] == 'menu':
                    p = self.find_parameter(keys, menu_item[2])
                    if p is not None:
                        return p
        return None

    def verify_parameter(self, key, value):
        keys = [k.strip() for k in key.split('>') if len(k.strip()) > 0]
        p = self.find_parameter(keys, self.menu)
        if p is None:
            print('WARNING in verify():', key, 'not found')
        else:
            p.verify(value)
            self.configacts.matches = p.matches

    def set_parameter(self, key, value):
        keys = [k.strip() for k in key.split('>') if len(k.strip()) > 0]
        p = self.find_parameter(keys, self.menu)
        if p is None:
            print('WARNING in verify():', key, 'not found')
        else:
            p.set_value(value)
            self.configacts.matches = p.matches

    def set_configfile_state(self, present):
        if present:
            self.config_status.setText('&#x2705;')
        else:
            self.config_status.setText('&#x274C;')

    def parse_idle(self):
        pass
        
    def parse_halt(self, k):
        s = 'Logger halted\n'
        k -= 1
        while k >= 0 and len(self.input[k]) == 0:
            k -= 1
        self.msg.setText(s + self.input[k])
        self.stack.setCurrentWidget(self.msg)
        self.read_func = self.parse_idle

    def parse_startup(self):
        title_start = None
        title_mid = None
        title_end = -1
        for k in range(len(self.startup_input)):
            if self.startup_input[k][:20] == 20*'=':
                title_start = k
            elif title_start is not None and \
                 ' by ' in self.startup_input[k]:
                title_mid = k
            elif title_start is not None and \
                 self.startup_input[k][:20] == 20*'-':
                title_end = k
        if title_start is not None and \
           title_end > 0:
            if title_mid is not None:
                s = ''
                for l in self.startup_input[title_start + 1:title_mid]:
                    if len(l.strip()) == 0:
                        continue
                    if len(s) > 0:
                        s += '\n'
                    s += l
                self.logo.setText(s)
                title_start = title_mid - 1
            self.softwareinfo.set(self.startup_input[title_start + 1:title_end])

        for s in self.startup_input[title_end + 1:]:
            if 'configuration file "' in s.lower():
                config_file = s.split('"')[1].strip()
                self.config_file.setText(f'<b>{config_file}</b>')
                self.set_configfile_state(not 'not found' in s.lower())
                self.configacts.config_file = config_file
                break
            elif '! error: no sd card present' in s.lower():
                self.set_configfile_state(False)
                break
        
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
            num = x[0][:-1]
            if x[-1] == '...':
                # sub menu:
                name = ' '.join(x[1:-1])
                menu[name] = (num, 'menu', {})
            else:
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
                    self.init_menu()
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
            param = Parameter(self.menu_ids, self.menu_key, self.menu_item[2])
            param.initialize(s)
            param.set_selection(self.input[list_start:list_end])
            param.sigTransmitRequest.connect(self.transmit_request)
            self.menu_item[2] = param
            self.write('keepthevalue')
            self.read_state = 0
            

    def init_menu(self):
        if 'Help' in self.menu:
            self.menu.pop('Help')
        self.configacts.setup(self.menu)
        self.config_params = []
        missing_tools = False
        first_param = True
        row = 0
        title = None
        for mk in self.menu:
            menu = self.menu[mk]
            add_title = True
            if menu[1] == 'menu':
                for sk in menu[2]:
                    if menu[2][sk][1] == 'param':
                        if add_title:
                            title = QLabel('<b>' + mk + '</b>', self)
                            title.setSizePolicy(QSizePolicy.Policy.Preferred,
                                                QSizePolicy.Policy.Fixed)
                            self.configuration.addWidget(title, row, 0, 1, 4)
                            row += 1
                            add_title = False
                        self.configuration.addItem(QSpacerItem(10, 0), row, 0)
                        param_label = QLabel(sk + ': ', self)
                        self.configuration.addWidget(param_label, row, 1)
                        param = menu[2][sk][2]
                        param.setup(self, param_label, title)
                        self.configuration.addWidget(param.edit_widget, row, 2)
                        self.configuration.addWidget(param.state_widget,
                                                 row, 3)
                        if first_param:
                            param.edit_widget.setFocus(Qt.MouseFocusReason)
                            first_param = False
                        row += 1
                        self.config_params.append(param)
                    elif menu[2][sk][1] == 'action':
                        if not missing_tools:
                            print('WARNING! the following tool actions are not supported:')
                            missing_tools = True
                        if add_title:
                            print(f'{mk}:')
                            add_title = False
                        print(f'  {sk}')
        self.configuration.addItem(QSpacerItem(0, 0,
                                               QSizePolicy.Policy.Minimum,
                                               QSizePolicy.Policy.Expanding),
                                   row, 0)
        row += 1
        self.configuration.addWidget(QLabel('Configuration file'), row, 0, 1, 2)
        self.configuration.addWidget(self.config_file, row, 2)
        self.configuration.addWidget(self.config_status, row, 3)
        row += 1
        fm = self.fontMetrics()
        self.configuration.addItem(QSpacerItem(0, 2*fm.averageCharWidth(),
                                               QSizePolicy.Policy.Minimum,
                                               QSizePolicy.Policy.Minimum),
                                   row, 0)
        row += 1
        self.configuration.addWidget(QLabel('Mode'), row, 0, 1, 2)
        boxw = QWidget(self)
        box = QHBoxLayout(boxw)
        self.user_button = QRadioButton('&User', self)
        self.admin_button = QRadioButton('&Admin', self)
        self.user_button.toggled.connect(self.set_mode)
        self.user_button.setChecked(True)
        self.admin_button.toggled.connect(self.set_mode)
        box.addWidget(self.user_button)
        box.addWidget(self.admin_button)
        self.configuration.addWidget(boxw, row, 2)
        row += 1
            
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
        if self.ser is None:
            try:
                self.ser = Serial(self.device)
                self.ser.reset_input_buffer()
                self.ser.reset_output_buffer()
            except (OSError, SerialException):
                self.ser = None
                self.stop()
                return
        try:
            if self.ser.in_waiting > 0:
                # read in incoming data:
                x = self.ser.read(self.ser.in_waiting)
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
        if self.ser is not None:
            try:
                self.ser.reset_input_buffer()
            except (OSError, SerialException):
                self.stop()
        self.input = []
        
