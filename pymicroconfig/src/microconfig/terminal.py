from PyQt5.QtCore import Qt
from PyQt5.QtGui import QKeySequence, QFont
from PyQt5.QtWidgets import QShortcut, QVBoxLayout
from PyQt5.QtWidgets import QWidget, QPushButton, QLabel, QScrollArea


class Terminal(QWidget):
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.title = QLabel(self)
        self.out = QLabel(self)
        self.out.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self.scroll = QScrollArea(self)
        self.scroll.setWidget(self.out)
        self.done = QPushButton(self)
        self.done.setText('&Done')
        self.done.setToolTip('Close the terminal (Return, Escape, Space)')
        self.done.clicked.connect(self.clear)
        key = QShortcut(QKeySequence.Cancel, self)
        key.activated.connect(self.done.animateClick)
        key = QShortcut(Qt.Key_Space, self)
        key.activated.connect(self.done.animateClick)
        key = QShortcut(Qt.Key_Return, self)
        key.activated.connect(self.done.animateClick)
        vbox = QVBoxLayout(self)
        vbox.addWidget(self.title)
        vbox.addWidget(self.scroll)
        vbox.addWidget(self.done)

    def clear(self):
        self.title.setText('')
        self.out.setText('')
        self.out.setMinimumSize(1, 1)
        self.out.setMaximumSize(self.out.sizeHint())

    def update(self, stream):
        if len(stream) > 0:
            self.title.setText(stream[0])
        s = ''
        for l in stream[1:]:
            s += l + '\n'
        self.done.setEnabled(False)
        self.out.setText(s)
        self.out.setMinimumSize(self.out.sizeHint())
        vsb = self.scroll.verticalScrollBar()
        vsb.setValue(vsb.maximum())

    def display(self, title, stream):
        if title:
            self.title.setText(title)
        self.done.setEnabled(True)
        if isinstance(stream, (tuple, list)):
            text = ''
            for s in stream:
                text += s
                text += '\n'
            self.out.setText(text)
            self.out.setFont(QFont('monospace'))
        else:
            self.out.setText(stream)
            self.out.setFont(QFont('sans'))
        self.out.setMinimumSize(self.out.sizeHint())
        vsb = self.scroll.verticalScrollBar()
        vsb.setValue(vsb.maximum())

