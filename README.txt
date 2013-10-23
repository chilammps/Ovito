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