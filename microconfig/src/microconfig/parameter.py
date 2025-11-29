from .interactors import Interactor, InteractorQObject
from PyQt5.QtCore import Qt, QObject, QLocale
from PyQt5.QtWidgets import QLabel, QSpinBox
from PyQt5.QtWidgets import QCheckBox, QLineEdit, QComboBox
from .spinbox import SpinBox
from .units import parse_number, change_unit


class Parameter(Interactor, QObject, metaclass=InteractorQObject):
    """Widgets and algorithms for editing a configuration parameter.

    Parameters
    ----------
    ids: list of str
        Serial inputs required to interact with this parameters menu entry.
    name: str
        Name of the parameter.
    value: str
        Initial value of the parameter.
    num_value: int or float
        In case of a numerical parameter its value as a number.
    out_unit: str
        Optional unit of a numerical parameter used for user interactions.
    unit: str
        Internal unit of a numerical parameter.
    mode_str: str
        Contains 'U' if editable in user mode, 'A' for admin mode.
    type_str: str
        The type of parameter. One of "string", "boolean", "integer" or "float".
    max_chars: int
        In case of a string parameter, the maximum number of characters
        the string can hold.
    ndec: int
        In case of a float parameter, the number of decimals shown.
    min_val: int or float
        In case of a numerical parameter, the smallest valid value.
    max_val: int or float
        In case of a numerical parameter, the largest valid value.
    special_val: int or float
        In case of a numerical parameter, the valid that shold be shown
        as `special_str`.
    special_str: str
        In case of a numerical parameter with a `special_val` the corresponding
        special string to be shown in the widget.
    selection: list of str, list of int, or list of float
        Optional list of valid values.
    """
    
    def __init__(self, ids, name, value, num_value=None,
                 out_unit=None, unit=None, mode_str=None, type_str=None,
                 max_chars=None, ndec=None, min_val=None,
                 max_val=None, special_val=None, special_str=None,
                 selection=None, *args, **kwargs):
        super(QObject, self).__init__(*args, **kwargs)
        self.ids = list(ids)
        self.name = name
        self.value = value
        self.num_value = num_value
        self.out_unit = out_unit
        self.unit = unit
        self.mode_str = mode_str
        self.type_str = type_str
        self.max_chars = max_chars
        self.ndec = ndec
        self.min_val = min_val
        self.max_val = max_val
        self.special_val = special_val
        self.special_str = special_str
        self.selection = selection
        self.title_widget = None
        self.label_widget = None
        self.edit_widget = None
        self.state_widget = None
        self.matches = False

    def initialize(self, s):
        ss = s.split(',')
        self.mode_str = ss.pop(0).strip()
        self.type_str = ss.pop(0).strip()
        self.max_chars = 0
        self.min_val = None
        self.max_val = None
        self.special_val = None
        self.special_str = None
        if self.type_str.startswith('string'):
            self.max_chars = int(self.type_str.split()[-1].strip())
            self.type_str = 'string'
        self.num_value = None
        self.unit = None
        self.out_unit = None
        self.ndec = None
        if self.type_str in ['float', 'integer']:
            self.unit = ss.pop(0).strip()
            self.num_value, self.out_unit, self.ndec = parse_number(self.value)
        while len(ss) > 0:
            s = ss.pop(0).strip()
            if s.startswith('between'):
                mm = s.split()
                self.min_val = mm[1].strip()
                self.max_val = mm[3].strip()
            elif s.startswith('greater than'):
                self.min_val = s.split()[-1]
            elif s.startswith('less than'):
                self.max_val = s.split()[-1]
            elif s.startswith('or'):
                special = s.split('"')
                self.special_str = special[1]
                special = special[2]
                special = special[special.find('[') + 1:special.find(']')]
                if self.unit:
                    special = special[:-len(self.unit)]
                self.special_val = int(special)
        if self.type_str in ['float', 'integer'] and self.num_value is None:
            if self.value == self.special_str:
                self.num_value = self.special_val
                self.out_unit = self.unit

    def set_selection(self, stream):
        self.selection = []
        for k, l in enumerate(stream):
            sel = l[4:]
            i = sel.find(') ')
            if i >= 0 and sel[:i].isdigit():
                sel = (sel[:i], sel[i + 2:])
            else:
                sel = (None, sel)
            self.selection.append(sel)
        
    def setup(self, parent, label_widget, title_widget):
        self.label_widget = label_widget
        self.title_widget = title_widget
        if self.type_str == 'boolean':
            self.edit_widget = QCheckBox(parent)
            checked = self.value.lower() in ['yes', 'on', 'true', 'ok', '1']
            self.edit_widget.setChecked(checked)
            try:
                self.edit_widget.checkStateChanged.connect(self.transmit_bool)
            except AttributeError:
                self.edit_widget.stateChanged.connect(self.transmit_bool)
        elif len(self.selection) > 0:
            locale = QLocale()
            self.edit_widget = QComboBox(parent)
            idx = None
            for i, s in enumerate(self.selection):
                si = s[1]
                if si == self.value:
                    idx = i
                if self.type_str in ['integer', 'float']:
                    si = si.replace('.', locale.decimalPoint())
                self.edit_widget.addItem(si)
            self.edit_widget.setCurrentIndex(idx)
            self.edit_widget.setEditable(False)
            self.edit_widget.currentTextChanged.connect(self.transmit_str)
        elif self.type_str == 'integer' and not self.unit:
            self.edit_widget = QSpinBox(parent)
            if self.min_val is not None:
                self.edit_widget.setMinimum(int(self.min_val))
            if self.max_val is not None:
                self.edit_widget.setMaximum(int(self.max_val))
            else:
                self.edit_widget.setMaximum(100000)
            if self.special_val is not None and \
               self.special_str is not None and \
               self.special_val == self.edit_widget.minimum():
                self.edit_widget.setSpecialValueText(self.special_str)
            self.edit_widget.setValue(self.num_value)
            self.edit_widget.textChanged.connect(self.transmit_str)
        elif self.type_str in ['integer', 'float']:
            self.edit_widget = SpinBox(parent)
            self.edit_widget.setDecimals(self.ndec)
            if self.min_val is not None:
                if self.out_unit:
                    minv = float(self.min_val[:-len(self.out_unit)])
                else:
                    minv = float(self.min_val)
                self.edit_widget.setMinimum(minv)
            if self.edit_widget.minimum() is not None and \
               self.edit_widget.minimum() >= 0:
                self.edit_widget.setStepType(QSpinBox.AdaptiveDecimalStepType)
            if self.max_val is not None:
                if self.out_unit:
                    maxv = float(self.max_val[:-len(self.out_unit)])
                else:
                    maxv = float(self.max_val)
                self.edit_widget.setMaximum(maxv)
            else:
                self.edit_widget.setMaximum(1e9)
            if self.out_unit:
                self.edit_widget.setSuffix(self.out_unit)
            self.edit_widget.setValue(self.num_value)
            self.edit_widget.textChanged.connect(self.transmit_str)
        elif self.type_str == 'string':
            self.edit_widget = QLineEdit(self.value, parent)
            self.edit_widget.setMaxLength(self.max_chars)
            fm = self.edit_widget.fontMetrics()
            self.edit_widget.setMinimumWidth(32*fm.averageCharWidth())
            self.edit_widget.textChanged.connect(self.transmit_str)
        self.state_widget = QLabel(parent)
        self.state_widget.setTextFormat(Qt.RichText)
        self.state_widget.setToolTip('Indicate whether dialog value matches logger settings')
        self.state_widget.setText('&#x2705;')
        self.set_mode('U')

    def set_mode(self, mode):
        v = mode in self.mode_str
        self.label_widget.setVisible(v)
        self.edit_widget.setVisible(v)
        self.state_widget.setVisible(v)
        return v

    def transmit_bool(self, check_state):
        start = list(self.ids)
        start.append('2' if check_state > 0 else '1')
        self.sigTransmitRequest.emit(self, self.name, start)

    def transmit_str(self, text):
        start = list(self.ids)
        if len(self.selection) > 0:
            for s in self.selection:
                if s[1].lower() == text.lower() and s[0] is not None:
                    text = s[0]
                    break
        if self.type_str in ['integer', 'float']:
            locale = QLocale()
            text = text.replace(locale.groupSeparator(), '')
            text = text.replace(locale.decimalPoint(), '.')
        start.append(text)
        self.sigTransmitRequest.emit(self, self.name, start)

    def verify(self, text):
        self.matches = True
        if self.type_str == 'boolean':
            checked = text.lower() in ['yes', 'on', 'true', 'ok', '1']
            self.matches = checked == self.edit_widget.isChecked()
        elif len(self.selection) > 0:
            if self.type_str in ['integer', 'float']:
                value, unit, _ = parse_number(text)
                locale = QLocale()
                s = self.edit_widget.currentText()
                s = s.replace(locale.groupSeparator(), '')
                s = s.replace(locale.decimalPoint(), '.')
                svalue, sunit, _ = parse_number(s)
                self.matches = abs(value - svalue) < 1e-6 and unit == sunit
            else:
                self.matches = self.edit_widget.currentText() == text
        elif self.type_str in ['integer', 'float']:
            value, unit, _ = parse_number(text)
            if value is None and text == self.special_str:
                value = self.special_val
            locale = QLocale()
            s = self.edit_widget.text()
            s = s.replace(locale.groupSeparator(), '')
            s = s.replace(locale.decimalPoint(), '.')
            svalue, sunit, _ = parse_number(s)
            if svalue is None and s == self.special_str:
                svalue = self.special_val
            if self.out_unit:
                svalue = change_unit(svalue, sunit, unit)
            self.matches = abs(value - svalue) < 1e-6
        elif self.type_str == 'string':
            self.matches = (self.edit_widget.text() == text)
        if self.matches:
            self.state_widget.setText('&#x2705;')
        else:
            self.state_widget.setText('&#x274C;')

    def set_value(self, text):
        if self.type_str == 'boolean':
            checked = text.lower() in ['yes', 'on', 'true', 'ok', '1']
            self.edit_widget.setChecked(checked)
        elif len(self.selection) > 0:
            if self.type_str in ['integer', 'float']:
                value, unit, _ = parse_number(text)
                for i, s in enumerate(self.selection):
                    svalue, sunit, _ = parse_number(s[1])
                    if abs(value - svalue) < 1e-8 and unit == sunit:
                        self.edit_widget.setCurrentIndex(i)
                        break
            else:
                self.edit_widget.setCurrentText(text)
        elif self.type_str in ['integer', 'float']:
            value, unit, _ = parse_number(text)
            if value is None and text == self.special_str:
                value = self.special_val
            self.edit_widget.setValue(value)
        elif self.type_str == 'string':
            self.edit_widget.setText(text)
        self.verify(text)
    
    def read(self, ident, stream, success):
        for l in stream:
            if self.name in l:
                ss = l.split(':')
                if len(ss) > 1:
                    text = ':'.join(ss[1:]).strip()
                    self.verify(text)
            elif 'new value' in l:
                self.state_widget.setText('&#x274C;')
                
