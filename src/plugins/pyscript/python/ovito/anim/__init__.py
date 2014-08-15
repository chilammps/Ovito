# Load the native module.
from PyScriptAnimation import *

# Add __str__ method to TimeInterval class:
TimeInterval.__str__ = lambda self: str((self.start,self.end))