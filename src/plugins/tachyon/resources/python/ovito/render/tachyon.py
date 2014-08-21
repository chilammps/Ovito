# Load dependencies first.
import ovito

# Load the native code module
import Tachyon

# Inject TachyonRenderer class into parent module.
ovito.render.TachyonRenderer = Tachyon.TachyonRenderer