from abc import ABC, abstractmethod

try:
    from PyQt5.QtCore import Signal
except ImportError:
    from PyQt5.QtCore import pyqtSignal as Signal
from PyQt5.QtCore import QObject
from PyQt5.QtGui import QPalette, QColor
from PyQt5.QtWidgets import QWidget, QFrame, QPushButton

            
class Interactor(ABC):
    """Abstract base class for interacting with a microconfig menu entry.

    A derived class needs to implement the setup() and read() functions.
    In setup() the menu entry this class acts on should be selected
    using `retrieve()`.

    A number of signals are provided that allow you to interact with
    the microcontroller and with the GUI.

    Signals
    -------
    sigReadRequest:
        Activate a menu entry and expect some output from the microcontroller
        that can then be parsed by the read() function. Call it with three
        arguments: (1) a identifier string of your choice, (2) the list
        returned by retrieve, (3) a list of strings that indicate a
        stop condition.
    sigWriteRequest:
        Write something to the microcontroller. The first argument is
        a string that is written to the microcontroller after a menu
        entry has been activated by sending the list of strings given
        as the second argument.
    sigTransmitRequest:
        Write something to the microcontroller and read its response.
    sigDisplayTerminal:
        Display some information in a terminal. The first argument is
        a title string displayed on top. The second argument is either
        a list of strings. Then they are displayed line by line in
        typewriter fonts. Or a single string is provided. This is then
        expected to contain html formatting instructions.        
    sigDisplayMessage:
        Display a brief text in a message dialog box. The single argument
        is a string or a list of strings with the text to be displayed.
    sigUpdate:
        Request to update infos about SD card usage.
    """

    sigReadRequest = Signal(object, str, list, list)
    sigWriteRequest = Signal(str, list)
    sigTransmitRequest = Signal(object, str, list)
    sigDisplayTerminal = Signal(str, object)
    sigDisplayMessage = Signal(object)
    sigUpdate = Signal()

    @abstractmethod
    def setup(self, menu):
        """ Setup handling of a single entry of a microconfig menu.

        A derived class needs to implement this function.
        In this function, retrieve() should be called and its return value
        should be stored so that later this menu entry can be executed
        by emitting a sigReadRequest.
        """
        pass

    def retrieve(self, key, menu, verbose=True):
        """ Retrieve an entry from the microconfig menu.

        Parameters
        ----------
        key: str
            Key to be searched in the menu.
            Hierarchical levels are separated by '>'.
        menu: dict
            The microconfig menu.
        verbose: bool
            If True print message when key can not be found in menu.

        Returns
        -------
        ids: list of str
            Keyboard commands needed to select the requested menu entry.
        """
        
        def find(keys, menu, ids):
            found = False
            for mk in menu:
                if keys[0] in mk.lower():
                    found = True
                    menu_item = menu[mk]
                    ids.append(menu_item[0])
                    if len(keys) > 1:
                        if menu_item[1] == 'menu' and \
                           find(keys[1:], menu_item[2], ids):
                            if len(menu_item[2]) == 0:
                                menu.pop(mk)
                            return True
                    else:
                        menu.pop(mk)
                        return True
                    break
            if not found:
                for mk in menu:
                    menu_item = menu[mk]
                    ids.append(menu_item[0])
                    if menu_item[1] == 'menu' and \
                       find(keys, menu_item[2], ids):
                        if len(menu_item[2]) == 0:
                            menu.pop(mk)
                        return True
                    ids.pop()
            return False

        keys = [k.strip() for k in key.lower().split('>') if len(k.strip()) > 0]
        ids = []
        if find(keys, menu, ids):
            return ids
        elif verbose:
            print(key, 'not found')
        return []

    @abstractmethod
    def read(self, ident, stream, success):
        """Read stream from microcontroller.
        
        A derived class needs to implement this function.
        In this function any information requested from the microcontroller
        via sigReadRequest can be parsed.

        Parameters
        ----------
        ident: str
            The identifier string that was passed on to the
            corresponding sigReadRequest.
        stream: list of str
            The serial stream obtained from the microcontroller in response
            to the corresponding sigReadRequest.
        success: bool
            True if everything was obtained from the microcontroller
            and the microcontroller is ready again for selecting another action.
        """
        pass

        
class InteractorQObject(type(Interactor), type(QObject)):
    # this class is needed for multiple inheritance of ABC ...
    pass

        
class InteractorQWidget(type(Interactor), type(QWidget)):
    # this class is needed for multiple inheritance of ABC ...
    pass

        
class InteractorQFrame(type(Interactor), type(QFrame)):
    # this class is needed for multiple inheritance of ABC ...
    pass

        
class InteractorQPushButton(type(Interactor), type(QPushButton)):
    # this class is needed for multiple inheritance of ABC ...
    pass


class InfoFrame(Interactor, QFrame, metaclass=InteractorQFrame):
    """ An interactor producing a frame with lots of infos.
    """
    
    def __init__(self, *args, **kwargs):
        super(QFrame, self).__init__(*args, **kwargs)
        self.setFrameStyle(QFrame.Panel | QFrame.Sunken)

                
class ReportButton(Interactor, QPushButton, metaclass=InteractorQPushButton):
    """ A small button that activates an entry of a microconf menu.
    """
    
    def __init__(self, key, text, *args, **kwargs):
        """ Initialize the ReportButton.

        Parameters
        ----------
        key: str
            The key of the menu entry this button should activate.
        text: str
            A text displayed on the button.
        """
        super(QPushButton, self).__init__(*args, **kwargs)
        self.setText(text)
        self.clicked.connect(self.run)
        self.key = key
        self.start = []

    def setText(self, text):
        """ Se the text of the button.

        Parameters
        ----------
        text: str
            Text to be displayed on the button.
        """
        super().setText(text)
        bbox = self.fontMetrics().boundingRect(text)
        self.setMaximumWidth(bbox.width() + 10)
        self.setMaximumHeight(bbox.height() + 2)

    def set_button_color(self, color):
        """ Se the background color of the button.

        Parameters
        ----------
        color: anything QColor takes as argument
            Background color of the button.
        """
        pal = self.palette()
        pal.setColor(QPalette.Button, QColor(color))
        self.setAutoFillBackground(True)
        self.setPalette(pal)
        self.update()
     
    def setup(self, menu):
        """ Retrieve the buttons key from the menu.
        """
        self.start = self.retrieve(self.key, menu)
        if len(self.start) == 0:
            self.setVisible(False)

    def run(self):
        """ Activate the menu entry.
        """
        self.sigReadRequest.emit(self, 'run', self.start, ['select'])
