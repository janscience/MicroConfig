"""
Interacting with a micro controller via a
[MicroConfig](https://github.com/janscience/MicroConfig) menu.
"""
    
from .version import __version__
    
__all__ = ['discover']
    
# make all important functions available in the microconfig namespace:
from .discover import discover_teensy_ports, print_teensys

