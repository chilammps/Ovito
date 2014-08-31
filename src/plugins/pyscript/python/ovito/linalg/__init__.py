import numpy

# Load the native module.
from PyScriptLinearAlgebra import *

# Provide pretty string formatting for the matrix classes.
# For this, we make use of NumPy's capabilities.
AffineTransformation.__str__ = lambda self: str(numpy.asarray(self))
Matrix3.__str__ = lambda self: str(numpy.asarray(self))
