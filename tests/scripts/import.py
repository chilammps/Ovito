import ovito
import ovito.particles

# Check program version
print "This is OVITO version", ovito.version

# Load test data
obj = ovito.load("../files/animation.dump.gz")
print obj

# Apply a modifier
mod = ovito.particles.CoordinationNumberModifier({'cutoff': 2.36, 'test': 6})
obj.applyModifier(mod)
