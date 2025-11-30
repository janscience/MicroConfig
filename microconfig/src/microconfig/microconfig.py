import sys
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QFont
from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox
from PyQt5.QtWidgets import QHBoxLayout, QVBoxLayout, QStackedWidget
from PyQt5.QtWidgets import QWidget, QLabel

from .version import __version__
from .configactions import ConfigActions
from .configeditor import ConfigEditor
from .terminal import Terminal
from .communicator import Communicator
from .scanner import Scanner
from .discover import discover_teensy


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

        
class MicroConfig(Communicator, QMainWindow):
    
    def __init__(self, title, device, *args, **kwargs):
        Communicator.__init__(self, device)
        QMainWindow.__init__(self, *args, **kwargs)

        self.title = title if title is not None else 'MicroConfig'
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
                
        self.configacts = ConfigActions(self)
        self.configacts.sigReadRequest.connect(self.read_request)
        self.configacts.sigWriteRequest.connect(self.write_request)
        self.configacts.sigDisplayTerminal.connect(self.display_terminal)
        self.configacts.sigDisplayMessage.connect(self.display_message)
        self.configacts.sigVerifyParameter.connect(self.verify_parameter)
        self.configacts.sigSetParameter.connect(self.set_parameter)
        self.configacts.sigShowStartup.connect(self.show_startup)

        self.configeditor = ConfigEditor(self)
        self.configeditor.sigTransmitRequest.connect(self.transmit_request)
        self.configeditor.sigSetMode.connect(self.configacts.set_mode)        
        self.configacts.sigConfigFile.connect(self.configeditor.set_configfile_state)
                
        self.boxw = QWidget(self)
        self.box = QHBoxLayout(self.boxw)
        self.box.addWidget(self.configacts)
        self.box.addWidget(self.configeditor)
        
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
        
        self.read_timer = QTimer(self)
        self.read_timer.timeout.connect(self.read)

    def start(self, device):
        self.msg.setText('Reading configuration ...')
        self.msg.setAlignment(Qt.AlignCenter)
        self.stack.setCurrentWidget(self.msg)
        super().start(device)
        self.read_timer.start(2)

    def stop(self):
        self.read_timer.stop()
        super().stop()
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
        
    def parse_halt(self, k):
        super().parse_halt(k)
        s = 'Logger halted\n'
        k -= 1
        while k >= 0 and len(self.input[k]) == 0:
            k -= 1
        self.msg.setText(s + self.input[k])
        self.stack.setCurrentWidget(self.msg)

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
                self.configeditor.set_configfile(config_file,
                                                 not 'not found' in s.lower())
                self.configacts.config_file = config_file
                break
            elif '! error: no sd card present' in s.lower():
                self.configeditor.set_configfile_state(False)
                break
            
    def setup(self):
        super().setup()
        self.configacts.setup(self.menu)
        self.configeditor.setup(self.menu)
        self.stack.setCurrentWidget(self.boxw)

        
def main():
    app = QApplication(sys.argv)
    main = Scanner([discover_teensy], MicroConfig)
    main.show()
    app.exec_()

    
if __name__ == '__main__':
    main()
