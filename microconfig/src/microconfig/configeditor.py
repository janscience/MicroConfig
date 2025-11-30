try:
    from PyQt5.QtCore import Signal
except ImportError:
    from PyQt5.QtCore import pyqtSignal as Signal
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QHBoxLayout, QGridLayout
from PyQt5.QtWidgets import QSpacerItem, QSizePolicy
from PyQt5.QtWidgets import QWidget, QLabel, QRadioButton

from .interactors import InfoFrame
from .parameter import Parameter


class ConfigEditor(InfoFrame):
    
    sigSetMode = Signal(str)
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.configuration = QGridLayout(self)
        self.config_params = []
        self.config_file = QLabel()
        self.config_status = QLabel()
        self.config_status.setTextFormat(Qt.RichText)
        self.config_status.setToolTip('Indicates presence of configuration file')
        self.user_button = None
        self.admin_button = None

    def set_configfile_state(self, present):
        s = '&#x2705;' if present else '&#x274C;'
        self.config_status.setText(s)

    def set_configfile(self, file_name, present):
        self.config_file.setText(f'<b>{file_name}</b>')
        self.set_configfile_state(present)

    def set_mode(self, checked):
        mode = 'A' if self.admin_button.isChecked() else 'U'
        title_widget = None
        set_focus = True
        n = 0
        for p in self.config_params:
            if title_widget != p.title_widget:
                if title_widget is not None:
                    title_widget.setVisible(n > 0)
                title_widget = p.title_widget
                n = 0
            v = p.set_mode(mode)
            if set_focus and v:
                p.edit_widget.setFocus(Qt.MouseFocusReason)
                set_focus = False                
            if v:
                n += 1
        if title_widget is not None:
            title_widget.setVisible(n > 0)
        self.sigSetMode.emit(mode)

    def setup(self, menu):
        self.config_params = []
        missing_tools = False
        first_param = True
        row = 0
        title = None
        for mk in menu:
            action = menu[mk]
            add_title = True
            if action[1] == 'menu':
                for sk in action[2]:
                    if action[2][sk][1] == 'param':
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
                        pargs = action[2][sk][2]
                        param = Parameter(*pargs[:3])
                        param.initialize(pargs[3])
                        param.set_selection(pargs[4])
                        param.sigTransmitRequest.connect(self.sigTransmitRequest)
                        param.setup(self, param_label, title)
                        action[2][sk][2] = param
                        self.configuration.addWidget(param.edit_widget, row, 2)
                        self.configuration.addWidget(param.state_widget,
                                                 row, 3)
                        if first_param:
                            param.edit_widget.setFocus(Qt.MouseFocusReason)
                            first_param = False
                        row += 1
                        self.config_params.append(param)
                    elif action[2][sk][1] == 'action':
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
