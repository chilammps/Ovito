from ovito import *
from ovito.io import *

node = import_file("../../files/shear.void.120.cfg")
ptype_property = node.source.data.particle_type

assert(len(ptype_property.type_list) == 3)
assert(ptype_property.type_list[0].id == 1)

print(ptype_property.type_list[0].id)
print(ptype_property.type_list[0].color)
print(ptype_property.type_list[0].name)
print(ptype_property.type_list[0].radius)

print(ptype_property.array)
