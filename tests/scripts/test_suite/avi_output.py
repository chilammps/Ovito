import ovito
from ovito.io import *
from ovito.vis import *
import os
import os.path

import_file("../../files/LAMMPS/animation.dump.gz", multiple_frames = True)

vp = ovito.dataset.viewports.active_vp

if os.path.isfile("movie.avi"):
    os.remove("movie.avi")
assert(not os.path.isfile("movie.avi"))

settings = RenderSettings(
    filename = "movie.avi",
    size = (64, 64),
    range = RenderSettings.Range.ANIMATION        
)
vp.render(settings)

assert(os.path.isfile("movie.avi"))
os.remove("movie.avi")
