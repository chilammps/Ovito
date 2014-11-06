from ovito import *
from ovito.io import *
from ovito.modifiers import *

import numpy

node = import_file("../../files/animation.dump.gz")
modifier = ColorCodingModifier()

print("Parameter defaults:")

print("  start_value:", modifier.start_value)
print("  end_value:", modifier.end_value)
print("  gradient:", modifier.gradient)
print("  only_selected:", modifier.only_selected)
print("  property:", modifier.property)

modifier.gradient = ColorCodingModifier.Rainbow()
modifier.gradient = ColorCodingModifier.Jet()
modifier.gradient = ColorCodingModifier.Hot()
modifier.gradient = ColorCodingModifier.Grayscale()
modifier.gradient = ColorCodingModifier.Custom("../../../doc/manual/images/modifiers/color_coding_custom_map.png")

node.modifiers.append(modifier)

print(node.compute().color.array)
assert((node.compute().color.array[05] == numpy.array([1,0,0])).all())
