# Load dependencies
import ovito.vis

# Load the native code module
import Tachyon

# Inject TachyonRenderer class into parent module.
ovito.vis.TachyonRenderer = Tachyon.TachyonRenderer