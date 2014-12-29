PROJECT(RandomColorPlugin)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12 FATAL_ERROR)

# Find OVITO build tree.
FIND_PACKAGE(OVITO REQUIRED)

# Generate plugin module.
OVITO_PLUGIN(RandomColorPlugin
	SOURCES RandomColorModifier.cpp
	PLUGIN_DEPENDENCIES Particles
)
