import ovito
from ovito.io import *
import os
import os.path

import_file("../../files/animation.dump.gz")

if os.path.isfile("scene.ovito"): os.remove("scene.ovito")

ovito.dataset.save("scene.ovito")
assert(os.path.isfile("scene.ovito"))

os.remove("scene.ovito")
