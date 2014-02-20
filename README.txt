****************************************************************************************
OVITO - Open Visualization Tool 
****************************************************************************************

Author: Alexander Stukowski (Darmstadt University of Technology, Germany)

OVITO is a scientific visualization and analysis software for atomistic simulation data. 
See website for more information: 

http://www.ovito.org/

****************************************************************************************
Change Log 
****************************************************************************************

Release 2.2.4 (29-Jan-14):

 - Modified particle file importers to ensure stable ordering of particle types (using 
   lexicographical ordering when atom types have names, and ID-based ordering otherwise). 
   The ordering of named particle types is now independent of their first occurrence in the input file.
 - Fixed particle picking issue on computers with Intel graphics.
 - Fixed OpenGL issues on systems with Intel graphics.
 - Fixed blurred display of viewport captions.
 - Fixed program crash when changing particle radius/color without having selected a particle type first. 
 - OVITO is now built using version 5.2.1 of the Qt library. 
   This fixes several issues related to the graphical user interface. 

Release 2.2.3 (15-Jan-14):

 - Fixed the CFG file importer, which is now able to read CFG files written by newer versions of LAMMPS correctly.
   Auxiliary file columns are now automatically mapped to OVITO's standard particle properties if possible.
 - Improved compatibility with some OpenGL implementations (Intel HD graphics on Windows and ATI Mobility Radeon HD 5470).
 - A 64-bit version of the program for Windows has been built.
 - A construction grid can now be shown in the viewports (like it was possible with OVITO 1.x). 

Release 2.2.2 (05-Jan-14):

 - Fixed regression: Rendering a movie with Ovito 2.2.1 resulted in an empty file.
 - Fixed display of the selection polygon when using the Fence selection mode of the Manual Selection modifier.
 - Added --glversion command line option, which allows to request a specific OpenGL standard.

Release 2.2.1 (26-Dec-13):

 - Added a file parser for binary LAMMPS dump files.
 - Added a dialog window that displays information about the system's OpenGL graphics driver. This dialog can be accessed via the Help menu.
 - Fixed bug in the Expression Select and Compute Property modifiers, which couldn't handle particle property names that start with a number.
 - The OpenGL compatibility profile is now used instead of the core profile on Windows and Linux platforms.
 - Fixed an issue in the Construct Surface Mesh modifier, which sometimes led to a program crash on Windows.

Release 2.2.0 (15-Dec-13):

 - Added the Cluster Analysis modifier.
 - Added the Construct Surface Mesh modifier.
 - Added possibility to open multiple application windows (useful on Mac OS X platform).
 - Added user option to the viewport settings dialog that allows to turn off the restriction of 
   the vertical camera rotation. 
 - Added first version of the CrystalAnalysis plugin, which allows working with data 
   produced by the Crystal Analysis Tool.
 - The XYZ file exporter now writes particle type names instead of numeric type IDs.
 - Extended user documentation.
 - The user manual is now distributed with the program. An Internet connection is no longer
   necessary to access it.
 - A few bug fixes and OpenGL compatibility improvements.

Release 2.1.0 (15-Nov-13):

 - Added the Manual Selection modifier, which allows selecting individual particles
   with the mouse in the viewports. Using its "Fence selection" mode, a group of particles
   can be selected by drawing a closed path around them.
 - In addition to spherical particles, OVITO can now display particles with cubic and square shape.
   This can be useful for visualization of large lattice systems, Ising models, etc.
 - OVITO can now import more than one dataset into the scene and display them side by side. 
   Use the "Import Local/Remote File" function multiple times to load 
   several simulation files (or multiple instances of the same file) into the program.
   Each dataset has its own modification pipeline, i.e., one can customize the 
   visualization of each dataset separately. Use the Affine Transformation modifier
   to move one of the datasets to a different location such that multiple
   objects do not overlap.
 - A new VTK file importer allows reading triangle meshes into OVITO to visualize
   geometric objects such as an indentor tip. The VTK format is the native format
   used by the ParaView software and can, for instance, be written by LIGGGHTS/LAMMPS.
 - Camera objects can be created through the viewport context menu. A viewport can be 
   linked to a camera object to show the the corresponding view. Camera objects can
   be animated (that's still an experimental and incomplete feature).
 - When importing a sequence of simulation snapshots into OVITO, one can now configure
   the mapping of input frames to OVITO's animation frames. This allows to generate output
   movies with less (or more) frames than the imported snapshot sequence. 
 - The Tachyon renderer now supports semi-transparent particles. The transparency is controlled
   through the "Transparency" particle property. Use, for instance, the Computer Property
   modifier to set this property for certain particles. The transparency values can range
   from 0 (=fully opaque) to 1 (=not visible).
 - Fixed saving/loading of the selected gradient type in the Color Coding modifier.
 - Fixed a program deadlock when when dragging the time slider after loading a file sequence.
   from a remote location.

Release 2.0.3 (22-Oct-13):

 - Ported Tachyon raytracing renderer from old OVITO 1.1.0 release. This software-based 
   rendering engine allows to produce images with high-quality shading and ambient
   occlusion lighting.
 - The Create Bonds modifier will automatically turn off the display of bonds when 
   creating a large number of bonds (>1 million), which would make the program freeze.
 - The Displacement Vectors modifier now supports relative reference frames, i.e.,
   displacements can be calculated from two snapshots separated by a fixed time interval.
   Before this addition, the modifier could only compute displacements with respect
   to a fixed simulation snapshot.
 - Fixed bug in the Affine Transformation modifier leading to recursive updates.
 - Added support for computers with high-resolution (Retina) displays.
 - The 'Inspect Particle' applet now lets one select multiple particles and can report 
   distances and angles.
 - Added 'Clear history' button to remote file import dialog.
 - POSCAR file exporter now writes the new file format, which includes atom type names.

Release 2.0.2 (30-Sep-13):

 - Fixed loading of multi-timestep files with names containing a digit.
 - Fixed import of CFG file with atom type information.

Release 2.0.1 (27-Sep-13):

 - Fixed loading of file sequences based on wildcard pattern on Windows platform.
 - Replaced const arrays in GLSL shaders with uniform variables to support older Intel graphics chips.