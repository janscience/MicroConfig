"""# configactions

## Classes

`class ConfigActions`: buttons for managing the configuration of a microcontroller.


"""

from .interactors import Interactor, InteractorQWidget, ActionButton

try:
    from PyQt5.QtCore import Signal
except ImportError:
    from PyQt5.QtCore import pyqtSignal as Signal
from PyQt5.QtWidgets import QVBoxLayout, QSpacerItem, QSizePolicy
from PyQt5.QtWidgets import QWidget, QLabel, QPushButton
from PyQt5.QtWidgets import QFileDialog


class Put(ActionButton):

    def __init__(self, *args, **kwargs):
        super().__init__('configuration>put configuration to eeprom',
                         '&Put', 'confput', *args, **kwargs)
        self.setToolTip('Put configuration to EEPROM memory (Alt+P)')

    def read(self, ident, stream, success):
        for s in stream:
            if 'error' in s.lower():
                self.sigDisplayMessage.emit('\n'.join(stream))
                return
        self.sigDisplayTerminal.emit('EEPROM', stream)



class Get(ActionButton):
    
    sigSetParameter = Signal(str, str)

    def __init__(self, *args, **kwargs):
        super().__init__('configuration>get configuration from eeprom',
                         '&Get', 'confget', *args, **kwargs)
        self.setToolTip('Get configuration from EEPROM memory (Alt+G)')
        self.matches = False

    def read(self, ident, stream, success):
        if len(stream) == 0:
            return
        if 'error' in stream[0].lower():
            self.sigDisplayMessage.emit(stream[0].strip())
            return
        title = stream[0].strip()
        text = '<style type="text/css"> td { padding: 0 15px; }</style>'
        text += '<table>'
        for s in stream[1:]:
            cs = s.split(' to ')
            key = cs[0].strip()[4:]
            cs = cs[1].split(' from ')
            value = cs[0].strip()
            cs = cs[1].split()
            addr = cs[-1]
            self.sigSetParameter.emit(key, value) 
            text += f'<tr><td>set</td><td>{key}</td><td>to</td><td><b>{value}</b></td><td>from address <tt>{addr}</tt></td>'
            if self.matches:
                text += '<td>&#x2705;</td></tr>'
            else:
                text += '<td>&#x274C;</td></tr>'
        text += '</table>'
        self.sigDisplayTerminal.emit(title, text)
        

class Clear(ActionButton):

    def __init__(self, *args, **kwargs):
        super().__init__('clear eeprom memory', 'Clear', 'confclear',
                         *args, **kwargs)
        self.setToolTip('Clear the full EEPROM memory')

    def read(self, ident, stream, success):
        self.sigDisplayMessage.emit('\n'.join(stream))
        

class Save(ActionButton):
    
    sigConfigFile = Signal(bool)

    def __init__(self, *args, **kwargs):
        super().__init__('configuration>save', '&Save', 'confsave',
                         *args, **kwargs)
        self.setToolTip('Save the configuration to file on SD card (Alt+S)')

    def read(self, ident, stream, success):
        text = ''
        for s in stream:
            if s.strip().lower().startswith('saved'):
                self.sigConfigFile.emit(True)
            text += s.rstrip()
            text += '\n'
        if len(text) > 0:
            self.sigDisplayMessage.emit(text)
        self.sigUpdate.emit()


class Load(ActionButton):
    
    sigSetParameter = Signal(str, str)

    def __init__(self, *args, **kwargs):
        super().__init__('configuration>load', '&Load', 'confload',
                         *args, **kwargs)
        self.setToolTip('Load the configuration from file on SD card (Alt+L)')
        self.matches = False

    def read(self, ident, stream, success):
        # TODO: this should have been done by parse_read_request:
        while len(stream) > 0 and len(stream[0].strip()) == 0:
            del stream[0]
        if len(stream) == 0:
            return
        if 'not found' in stream[0]:
            self.sigDisplayMessage.emit(stream[0].strip())
            return
        title = stream[0].strip()
        text = '<style type="text/css"> td { padding: 0 15px; }</style>'
        text += '<table>'
        for s in stream[1:]:
            cs = s.split(' to ')
            key = cs[0].strip()[4:]
            value = cs[1].strip()
            self.sigSetParameter.emit(key, value) 
            text += f'<tr><td>set</td><td>{key}</td><td>to</td><td><b>{value}</b></td>'
            if self.matches:
                text += '<td>&#x2705;</td></tr>'
            else:
                text += '<td>&#x274C;</td></tr>'
        text += '</table>'
        self.sigDisplayTerminal.emit(title, text)


class Erase(ActionButton):
    
    sigConfigFile = Signal(bool)

    def __init__(self, *args, **kwargs):
        super().__init__('configuration>erase', '&Erase', 'conferase',
                         *args, **kwargs)
        self.setToolTip('Erase configuration file on SD card (Alt+E)')

    def read(self, ident, stream, success):
        text = ''
        for s in stream:
            if s.strip().lower().startswith('removed'):
                self.sigConfigFile.emit(False)
            text += s.rstrip()
            text += '\n'
        if len(text) > 0:
            self.sigDisplayMessage.emit(text)
        self.sigUpdate.emit()


class Check(ActionButton):
    
    sigVerifyParameter = Signal(str, str)

    def __init__(self, name, *args, **kwargs):
        super().__init__('configuration>print', '&Check', 'confcheck',
                         *args, **kwargs)
        self.name = name
        self.setToolTip(f'Check configuration on the {self.name} and whether it matches the values show in the GUI (Alt+C)')
        self.matches = False

    def read(self, ident, stream, success):
        top_key = None
        text = '<style type="text/css"> td { padding: 0 15px; }</style>'
        text += '<table>'
        for s in stream:
            text += '<tr>'
            cs = s.split(':')
            if len(cs) > 1 and len(cs[1].strip()) > 0:
                key = cs[0].strip()
                value = (":".join(cs[1:])).strip()
                keys = f'{top_key}>{key}' if top_key else key
                self.sigVerifyParameter.emit(keys, value)
                text += f'<td></td><td>{key}</td><td><b>{value}</b></td>'
                if self.matches:
                    text += '<td>&#x2705;</td>'
                else:
                    text += '<td>&#x274C;</td>'
            else:
                top_key = cs[0].strip()
                text += f'<td colspan=4><b>{top_key}</b></td>'
            text += '</tr>'
        text += '</table>'
        self.sigDisplayTerminal.emit(f'Current configuration on the {self.name}',
                                     text)

            
class Import(ActionButton):
    
    sigSetParameter = Signal(str, str)

    def __init__(self, name, *args, **kwargs):
        super().__init__('configuration>read configuration from stream',
                         '&Import', 'confimport', *args, **kwargs)
        self.setToolTip('Import configuration from host (Alt+I)')
        self.config_file = f'{name}.cfg'
        self.start_print = None

    def run(self):
        file_path, _ = QFileDialog.getOpenFileName(self,
                                                   'Load configuration file',
                                                   self.config_file,
                                                   'configuration files (*.cfg)')
        if not file_path:
            return
        conf_lines = ''
        with open(file_path, 'r') as sf:
            conf_lines = [line.rstrip() for line in sf.readlines()]
        self.sigWriteRequest.emit('DONE', self.start + conf_lines)
        self.sigReadRequest.emit(self, 'confimport', self.start_print,
                                 ['select'])

    def read(self, ident, stream, success):
        top_key = None
        for s in stream:
            cs = s.split(':')
            if len(cs) > 1 and len(cs[1].strip()) > 0:
                key = cs[0].strip()
                value = (":".join(cs[1:])).strip()
                keys = f'{top_key}>{key}' if top_key else key
                self.sigSetParameter.emit(keys, value)
            else:
                top_key = cs[0].strip()

                
class Export(ActionButton):

    def __init__(self, name, *args, **kwargs):
        super().__init__(None, 'E&xport', 'confexport', *args, **kwargs)
        self.setToolTip('Export configuration file to host (Alt+X)')
        self.config_file = f'{name}.cfg'

    def read(self, ident, stream, success):
        file_path, _ = QFileDialog.getSaveFileName(self,
                                                   'Save configuration file',
                                                   self.config_file,
                                                   'configuration files (*.cfg)')
        if not file_path:
            return
        with open(file_path, 'w') as df:
            for s in stream:
                df.write(s)
                df.write('\n')

                
class Reboot(ActionButton):

    def __init__(self, name, *args, **kwargs):
        super().__init__(None, 'Re&boot', None, *args, **kwargs)
        self.name = name
        self.setToolTip(f'Reboot {self.name} (Alt+B)')

    def run(self):
        self.sigReadRequest.emit(self, 'reboot', ['reboot'], [''])

    def read(self, ident, stream, success):
        pass

                
class Run(ActionButton):

    def __init__(self, name, *args, **kwargs):
        super().__init__(None, '&Run', None, *args, **kwargs)
        self.name = name
        self.setToolTip(f'Run {self.name} (Alt+R)')
        self.stream_len = 0

    def run(self):
        self.sigReadRequest.emit(self, 'run', ['q'], ['halt'])
        self.stream_len = 0

    def read(self, ident, stream, success):
        if len(stream) != self.stream_len:
            self.sigDisplayTerminal.emit(f'Run {self.name}', stream)
            self.stream_len = len(stream)

        
class ConfigActions(Interactor, QWidget, metaclass=InteractorQWidget):
    """Buttons for managing the configuration of a microcontroller.
    """

    sigVerifyParameter = Signal(str, str)
    sigSetParameter = Signal(str, str)
    sigConfigFile = Signal(bool)
    sigShowStartup = Signal()
    
    def __init__(self, name, *args, **kwargs):
        super(QWidget, self).__init__(*args, **kwargs)

        self.put_button = Put(self)
        self.put_button.sigReadRequest.connect(self.sigReadRequest)
        self.put_button.sigDisplayMessage.connect(self.sigDisplayMessage)
        self.put_button.sigDisplayTerminal.connect(self.sigDisplayTerminal)
        
        self.get_button = Get(self)
        self.get_button.sigReadRequest.connect(self.sigReadRequest)
        self.get_button.sigDisplayMessage.connect(self.sigDisplayMessage)
        self.get_button.sigDisplayTerminal.connect(self.sigDisplayTerminal)
        self.get_button.sigSetParameter.connect(self.sigSetParameter)
        
        self.clear_button = Clear(self)
        self.clear_button.sigReadRequest.connect(self.sigReadRequest)
        self.clear_button.sigDisplayMessage.connect(self.sigDisplayMessage)
        
        self.save_button = Save(self)
        self.save_button.sigReadRequest.connect(self.sigReadRequest)
        self.save_button.sigDisplayMessage.connect(self.sigDisplayMessage)
        self.save_button.sigConfigFile.connect(self.sigConfigFile)
        self.save_button.sigUpdate.connect(self.sigUpdate)
        
        self.load_button = Load(self)
        self.load_button.sigReadRequest.connect(self.sigReadRequest)
        self.load_button.sigDisplayMessage.connect(self.sigDisplayMessage)
        self.load_button.sigDisplayTerminal.connect(self.sigDisplayTerminal)
        self.load_button.sigSetParameter.connect(self.sigSetParameter)
        
        self.erase_button = Erase(self)
        self.erase_button.sigReadRequest.connect(self.sigReadRequest)
        self.erase_button.sigDisplayMessage.connect(self.sigDisplayMessage)
        self.erase_button.sigConfigFile.connect(self.sigConfigFile)
        self.erase_button.sigUpdate.connect(self.sigUpdate)
        
        self.check_button = Check(name, self)
        self.check_button.sigReadRequest.connect(self.sigReadRequest)
        self.check_button.sigDisplayTerminal.connect(self.sigDisplayTerminal)
        self.check_button.sigVerifyParameter.connect(self.sigVerifyParameter)
        
        self.import_button = Import(name, self)
        self.import_button.sigReadRequest.connect(self.sigReadRequest)
        self.import_button.sigWriteRequest.connect(self.sigWriteRequest)
        self.import_button.sigSetParameter.connect(self.sigSetParameter)
        
        self.export_button = Export(name, self)
        self.export_button.sigReadRequest.connect(self.sigReadRequest)
        
        self.startup_button = QPushButton('Star&tup', self)
        self.startup_button.setToolTip('Show startup messages (Alt+T)')
        self.startup_button.clicked.connect(self.sigShowStartup)
        
        self.reboot_button = Reboot(name, self)
        self.reboot_button.sigReadRequest.connect(self.sigReadRequest)
        
        self.run_button = Run(name, self)
        self.run_button.sigReadRequest.connect(self.sigReadRequest)
        self.run_button.sigDisplayTerminal.connect(self.sigDisplayTerminal)
        
        self.firmware_button = QPushButton('&Firmware', self)
        self.firmware_button.setToolTip('Upload new firmware (Alt+F)')
        self.show_firmware = True
        self.firmware_button.clicked.connect(self.firmware)
        
        box = QVBoxLayout(self)
        box.setContentsMargins(0, 0, 0, 0)
        box.addWidget(QLabel('<b>EEPROM:</b>'))
        box.addWidget(self.put_button)
        box.addWidget(self.get_button)
        box.addWidget(self.clear_button)
        box.addItem(QSpacerItem(0, 1000, QSizePolicy.Expanding,
                                QSizePolicy.Expanding))
        box.addWidget(QLabel('<b>File:</b>'))
        box.addWidget(self.save_button)
        box.addWidget(self.load_button)
        box.addWidget(self.erase_button)
        box.addItem(QSpacerItem(0, 1000, QSizePolicy.Expanding,
                                QSizePolicy.Expanding))
        box.addWidget(QLabel('<b>Host:</b>'))
        box.addWidget(self.import_button)
        box.addWidget(self.export_button)
        box.addItem(QSpacerItem(0, 1000, QSizePolicy.Expanding,
                                QSizePolicy.Expanding))
        box.addWidget(QLabel('<b>Logger:</b>'))
        box.addWidget(self.check_button)
        box.addItem(QSpacerItem(0, 1000, QSizePolicy.Expanding,
                                QSizePolicy.Expanding))
        box.addWidget(self.startup_button)
        box.addWidget(self.reboot_button)
        box.addWidget(self.firmware_button)
        box.addWidget(self.run_button)
        self.start_list_firmware = []
        self.start_update_firmware = []
        self.update_stream = []
    
    def setup(self, menu):
        self.get_button.setup(menu)
        self.put_button.setup(menu)
        self.clear_button.setup(menu)
        self.save_button.setup(menu)
        self.load_button.setup(menu)
        self.erase_button.setup(menu)
        self.check_button.setup(menu)
        self.import_button.setup(menu)
        self.export_button.setup(menu)
        self.import_button.start_print = self.check_button.start
        self.export_button.start = self.check_button.start
        
        self.start_list_firmware = self.retrieve('firmware>list', menu)
        self.start_update_firmware = self.retrieve('firmware>update', menu)
        if len(self.start_list_firmware) == 0:
            self.show_firmware = False
        else:
            self.sigReadRequest.emit(self, 'firmwarecheck',
                                     self.start_list_firmware, ['select'])
        if len(self.start_update_firmware) > 0:
            self.start_update_firmware.append('STAY')
        else:
            self.show_firmware = False
        self.firmware_button.setVisible(self.show_firmware)

    def set_sdcard(self, present):
        if self.show_firmware and not present:
            self.show_firmware = False

    def set_config_file(self, config_file):
        self.import_button.config_file = config_file
        self.export_button.config_file = config_file

    def set_match(self, matches):
        self.get_button.matches = matches
        self.load_button.matches = matches
        self.check_button.matches = matches

    def set_mode(self, mode):
        self.clear_button.setVisible('A' in mode)
        self.check_button.setVisible('A' in mode)
        self.firmware_button.setVisible('A' in mode and self.show_firmware)
        self.startup_button.setVisible('A' in mode)

    def firmware(self):
        self.sigReadRequest.emit(self, 'updatefirmware',
                                 self.start_update_firmware, ['select'])

    def read(self, ident, stream, success):
        if 'firmware' in ident:
            if ident == 'firmwarecheck':
                if len(stream) > 1 and 'no firmware files' in stream[1].lower():
                    self.show_firmware = False
            elif ident == 'updatefirmware':
                self.update_stream = []
                if len(stream) > 0 and 'available' in stream[0].lower():
                    del stream[0]
                for k in range(len(stream)):
                    if len(stream[k].strip()) == 0:
                        while k < len(stream):
                            del stream[k]
                        break
                if len(stream) == 1:
                    self.sigReadRequest.emit(self, 'runfirmware1',
                                             ['1', 'STAY'],
                                             ['select', 'enter', 'error'])
                else:
                    text = '<style type="text/css"> td { padding: 0 15px; } th { padding: 0 15px; }</style>'
                    text += '<table>'
                    text += f'<tr><th align="right">No</th><th align="left">Name</th></tr>'
                    for l in stream:
                        p = l.split()
                        number = p[1].rstrip(')')
                        name = p[2]
                        text += f'<tr><td align="right">{number}</td><td align="left">{name}</td></tr>'
                    text += '</table>'
                    self.sigDisplayTerminal.emit('Firmware', text)
                    self.sigReadRequest.emit(self, 'runfirmware1',
                                             ['n', 'STAY'],
                                             ['select', 'enter', 'error'])
            elif ident == 'runfirmware1':
                if len(stream) > 0 and 'aborted' in stream[0].lower():
                    for k in range(len(self.start_update_firmware) - 2):
                        self.sigWriteRequest.emit('q', [])
                elif len(stream) > 0:
                    self.sigDisplayTerminal.emit('Update firmware', stream)
                    if len(stream) > 1 and \
                       'enter' in stream[-2] and 'to flash' in stream[-2]:
                        self.update_stream = list(stream)
                        unlock_code = stream[-2].split()[1]
                        self.sigReadRequest.emit(self, 'runfirmware2',
                                                 [unlock_code, 'STAY'],
                                                 ['reboot'])
            elif ident == 'runfirmware2':
                self.sigDisplayTerminal.emit('Update firmware',
                                             self.update_stream + stream)
