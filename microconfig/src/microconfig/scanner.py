from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QKeySequence
from PyQt5.QtWidgets import QApplication, QMainWindow, QAction
from PyQt5.QtWidgets import QWidget, QLabel, QVBoxLayout
from .discover import Discover


class Scanner(QMainWindow):
    """Scan serial ports for known devices and initialize GUIs.

    Parameters
    ----------
    discover_funcs: list of functions
        Functions to be used for discovering devices via Discover.
        
    device_gui: class name
        A class with an activate(), show(), and stop() member function
        that implements a GUI for configuring the microcontroller.
    """
    
    def __init__(self, discover_funcs, device_gui, *args, **kwargs):
        super().__init__(*args, **kwargs)
        quit = QAction('&Quit', self)
        quit.setShortcuts(QKeySequence.Quit)
        quit.triggered.connect(QApplication.quit)
        self.addAction(quit)
        self.devices = dict()
        self.scan = Discover(*discover_funcs)
        self.device_gui = device_gui
        boxw = QWidget(self)
        self.setCentralWidget(boxw)
        box = QVBoxLayout(boxw)
        self.label = QLabel(self)
        self.label.setAlignment(Qt.AlignCenter)
        self.display()
        box.addWidget(self.label)
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.scan_ports)
        self.timer.start(500)

    def scan_ports(self):
        changed = False
        # mark devices as not found:
        for d in self.devices:
            self.devices[d][0] = False
        if self.scan.discover():
            # loop over detected devices:
            for dev in self.scan:
                d = (dev.device, dev.model, dev.serial)
                if d in self.devices:
                    self.devices[d][0] = True
                else:
                    logger = self.device_gui(None, dev, self)
                    logger.start(dev)
                    logger.show()
                    self.devices[d] = [True, logger]
                    changed = True
        # remove devices that are not active anymore:
        old_devs = []
        for d in self.devices:
            if not self.devices[d][0]:
                old_devs.append(d)
        for d in old_devs:
            changed = True
            self.devices[d][1].stop()
            del self.devices[d]
        if changed:
            self.display()

    def display(self):
        text = '<style type="text/css"> td { padding: 0 15px; }</style>'
        text += 'Scanning for loggers ...<br/>'
        if len(self.devices) > 0:
            text += '<table><tr><th align="left">Device</th><th align="left">Model</th><th align="left">Serial number</th></tr>'
            for dev in self.devices:
                text += f'<tr><td align="left">{dev[0]}</td><td align="left">{dev[1]}</td><td align="left">{dev[2]}</td></tr>'
            text += '</table>'
        else:
            text += 'Please connect a logger to an USB port.'
        self.label.setText(text)
        
