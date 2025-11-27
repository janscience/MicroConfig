try:
    from PyQt5.QtCore import Signal
except ImportError:
    from PyQt5.QtCore import pyqtSignal as Signal

from PyQt5.QtCore import Qt, QLocale
from PyQt5.QtGui import QValidator
from PyQt5.QtWidgets import QAbstractSpinBox

from .units import parse_number


class SpinBox(QAbstractSpinBox):

    textChanged = Signal(str)
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setAccelerated(True)
        self.setAlignment(Qt.AlignLeft)
        self.setButtonSymbols(QAbstractSpinBox.UpDownArrows)
        self.setCorrectionMode(QAbstractSpinBox.CorrectToPreviousValue)
        self.setFrame(True)
        self.setKeyboardTracking(True)
        self.setReadOnly(False)
        self.setGroupSeparatorShown(False)
        self.setWrapping(False)
        self._minimum = None
        self._maximum = None
        self._decimals = 0
        self._value = 0
        self._unit = ''

    def setDecimals(self, n):
        self._decimals = n

    def minimum(self):
        return self._minimum

    def maximum(self):
        return self._maximum

    def setMinimum(self, minv):
        self._minimum = minv

    def setMaximum(self, maxv):
        self._maximum = maxv

    def setStepType(self, stype):
        pass

    def stepEnabled(self):
        steps = self.StepUpEnabled | self.StepDownEnabled
        try:
            if self._minimum is not None and self._value <= self._minimum:
                steps &= ~self.StepDownEnabled
            if self._maximum is not None and self._value >= self._maximum:
                steps &= ~self.StepUpEnabled
        except AttributeError:  # why does it not know self._minimum initially?
            pass
        return steps

    def value(self):
        return self._value

    def setValue(self, value):
        self._value = value
        locale = QLocale()
        text = f'{value:.{self._decimals}f}'
        text = text.replace('.', locale.decimalPoint())
        text += self._unit
        self.lineEdit().setText(text)

    def setSuffix(self, suffix):
        self._unit = suffix

    def validate(self, text, pos):
        locale = QLocale()
        s = text.replace(locale.decimalPoint(), '.')
        value, unit, ndec = parse_number(s)
        if value is None:
            return QValidator.State.Intermediate, text, pos
        if self._minimum is not None and value <= self._minimum:
            return QValidator.State.Intermediate, text, pos
        if self._maximum is not None and value >= self._maximum:
            return QValidator.State.Intermediate, text, pos
        if ndec > self._decimals:
            return QValidator.State.Intermediate, text, pos
        self._value = float(value)
        if not self._unit and unit:
            text = text[:-len(unit)]
        self.textChanged.emit(text)
        #return QValidator.State.Intermediate
        #return QValidator.State.Invalid
        return QValidator.State.Acceptable, text, pos

    def fixup(self, text):
        locale = QLocale()
        s = text.replace(locale.decimalPoint(), '.')
        value, unit, ndec = parse_number(s)
        if value is None:
            value = self._minimum  # TODO should be previous value
        if self._minimum is not None and value <= self._minimum:
            value = self._minimum
        if self._maximum is not None and value >= self._maximum:
            value = self._maximum
        if ndec > self._decimals:
            ndec = self._decimals
        text = f'{value:.{ndec}f}'.replace('.', locale.decimalPoint())
        text += unit
        self.setValue(float(value))
        self.textChanged.emit(text)

    def stepBy(self, steps):
        self.setValue(self._value + steps)

