"""# interactors

Base classes for interacting with the microcontroller's MicroConfig menu.

- `class Interactor`: abstract base class for interacting with a microconfig menu entry.
- `class InfoFrame`: an interactor producing a frame with lots of infos.
- `class ActionButton`: a normal button that activates an entry of a microconf menu. 
- `class ReportButton`: a small button that activates an entry of a microconf menu.
"""

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

    A derived class needs to implement the setup() function and in most
    cases also the read() function.
    In setup() the menu entry this class acts on should be selected
    using `retrieve()`.
    read() then parses the output of the microcontroller produced upon
    activation of the menu entry via the sigReadRequest().

    A number of signals are provided that allow you to interact with
    the microcontroller and with the GUI.

    Signals
    -------
    sigReadRequest(object, str, list, list):
        Activate a menu entry and expect some output from the
        microcontroller that can then be parsed by the read()
        function. Call it with four arguments: (1) the interactor
        instance whose read() function should be called to parse the
        microcontroller's output, (2) an identifier string of your
        choice, (3) the list returned by retrieve() to be used to
        activate the menu entry, (4) a list of strings that indicate a
        stop condition in the microcontroler's output.
    sigWriteRequest(str, list):
        Write something to the microcontroller. The first argument is
        a string that is written to the microcontroller after a menu
        entry has been activated by sending the list of strings as
        returned by retrieve() given as the second argument.
    sigTransmitRequest(object, str, list):
        Write something to the microcontroller (second argument) after
        selecting a menu entry (thrid argument) and read its response
        (first argument).
    sigDisplayTerminal(str, object):
        Display some information in a terminal. The first argument is
        a title string displayed on top. The second argument is either
        a list of strings. Then they are displayed line by line in
        typewriter fonts. Or a single string is provided. This is then
        expected to contain html formatting instructions.        
    sigDisplayMessage(object):
        Display a brief text in a message dialog box. The single argument
        is a string or a list of strings with the text to be displayed.
    sigSDCardUpdate():
        Request to update infos about SD card usage.

    """

    sigReadRequest = Signal(object, str, list, list)
    sigWriteRequest = Signal(str, list)
    sigTransmitRequest = Signal(object, str, list)
    sigDisplayTerminal = Signal(str, object)
    sigDisplayMessage = Signal(object)
    sigSDCardUpdate = Signal()

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
    """ Class needed for multiple inheritance of ABC with QObject.
    """
    pass

        
class InteractorQWidget(type(Interactor), type(QWidget)):
    """ Class needed for multiple inheritance of ABC with QWidget.
    """
    pass

        
class _InteractorQFrame(type(Interactor), type(QFrame)):
    """ Class needed for multiple inheritance of ABC with QFrame.
    """
    pass

        
class _InteractorQPushButton(type(Interactor), type(QPushButton)):
    """ Class needed for multiple inheritance of ABC with QPushButton.
    """
    pass


class InfoFrame(Interactor, QFrame, metaclass=_InteractorQFrame):
    """ An interactor producing a frame with lots of infos.
    """
    
    def __init__(self, *args, **kwargs):
        QFrame.__init__(self, *args, **kwargs)
        self.setFrameStyle(QFrame.Panel | QFrame.Sunken)

                
class ActionButton(Interactor, QPushButton, metaclass=_InteractorQPushButton):
    """ A normal button that activates an entry of a microconf menu.
    """
    
    def __init__(self, key, text, ident, *args, **kwargs):
        """Initialize the ActionButton.

        Parameters
        ----------
        key: str or None
            The key of the menu entry this button should activate.
            If None do not retrieve a menu entry.
            You then need to reimplement the run() function
            to execute whatever you want when the button is clicked.
        text: str
            A text displayed on the button.
        ident: str or None
            The identifier str that is passed on to read.
            If None, it is set to 'run', i.e. read() is called even if
            the output of the microcontrolled has not completed yet.
        """
        QPushButton.__init__(self, *args, **kwargs)
        self.setText(text)
        self.clicked.connect(self.run)
        self.key = key
        self.start = []
        self.ident = ident if ident is not None else 'run'

    def setText(self, text):
        """ Se the text of the button.

        Parameters
        ----------
        text: str
            Text to be displayed on the button.
        """
        QPushButton.setText(self, text)

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
        """ Retrieve the button's key from the menu.
        """
        if self.key:
            self.start = self.retrieve(self.key, menu)
            if len(self.start) == 0:
                self.setVisible(False)

    def run(self):
        """ Activate the menu entry.
        """
        self.sigReadRequest.emit(self, self.ident, self.start, ['select'])


                
class ReportButton(ActionButton):
    """ A small button that activates an entry of a microconf menu.
    """
    
    def __init__(self, key, text, ident, *args, **kwargs):
        """ Initialize the ReportButton.

        Parameters
        ----------
        key: str or None
            The key of the menu entry this button should activate.
            If None do not retrieve a menu entry.
            You then need to reimplement the run() function
            to execute whatever you want when the button is clicked.
        text: str
            A text displayed on the button.
        ident: str or None
            The identifier str that is passed on to read.
            If None, it is set to 'run', i.e. read() is called even if
            the output of the microcontrolled has not completed yet.
        """
        super().__init__(key, text, ident, *args, **kwargs)

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
