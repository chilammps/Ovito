****************************************************************************************
OVITO - Open Visualization Tool 
****************************************************************************************

Author: Alexander Stukowski (mail@ovito.org)
        (Institute of Materials Science, Darmstadt University of Technology, Germany)

OVITO is a scientific visualization and analysis software for atomistic simulation data. 
See website for more information:

   http://www.ovito.org/

If you want to build OVITO from source, see http://www.ovito.org/manual/development.html

****************************************************************************************
Change Log 
****************************************************************************************

Release 2.4.4 (29-Mar-15):

 - Fixed error when rendering and encoding high-resolution videos.
 - Surface geometry computed by ConstructSurfaceModifier can now be exported to a VTK file from Python.
 - Added Python class ovito.data.CutoffNeighborFinder, which enables access to particle neighbor lists from Python.
 - Particles and bonds are now rendered in chunks in the OpenGL viewports to workaround a memory limit on
   some graphics hardware.
 - Bond cylinders will now be rendered using a geometry shader if supported by graphics card.
 - The IMD file exporter now lets the user select the particle properties to export (instead of exporting all).
 - The VTK triangle mesh importer now reads per-face color information.

Release 2.4.3 (02-Mar-15):

 - Added a dialog box to Affine Transformation modifier, which lets the user enter a rotation axis, angle, and center.
 - Removed cutoff option from Voronoi Analysis modifier in favor of a faster algorithm for orthogonal simulation cells, which is based on Voro++'s container classes.
 - OVITO can now load bonds from LAMMPS data files.
 - The Atomic Strain analysis can now be performed even when the number of particles changes with time (but only for the particles that are present in both the reference and the current configuration).
 - The Freeze Property modifier now works when particles are lost during the simulation.
 - The Wrap at Periodic Boundaries modifier now wraps bonds crossing a periodic boundary too.
 - The Show Periodic Images modifier now replicates bonds too.
 - Changed integrated script interpreter from Python 2.7 to Python 3.4.
 - Switched from MinGW to Visual C++ 2013 compiler for Windows builds. 
 - Python scripting is now supported by the 64-bit version of OVITO for Windows too.
 - Added a scriptable viewport overlay, which allows to paint arbitrary 2d contents over the rendered image.
 - Created C++ API documentation.
 - Removed old Javascript plugin.

Release 2.4.2 (14-Nov-14):

 - The Color Coding modifier now supports user-defined color maps.
 - Significantly improved performance of cutoff-based neighbor finding and k-nearest neighbor search routines.
   This code optimization speeds up many analysis algorithms in OVITO, in particular for large datasets.
 - Added the Identify Diamond Structure modifier, which finds atoms that form a cubic or hexagonal diamond lattice.
 - The Color Legend overlay now provides an option to overwrite the numeric labels with a custom text.
 - Dialog box asking to save changes is only shown when scene has already been saved before.
 - Bug fix: Periodic boundary flags were not correctly updated when loading a new file using the 'Pick new local input file' button.
 - Bug fix: Viewport.render() Python function throws exception when called without a RenderSettings object.
 
Release 2.4.1 (01-Nov-14):

 - Introduced viewport overlay concept, which allows to include the coordinate system tripod and a color legend in the rendered image. 
 - The OpenGL renderer now supports the display of semi-transparent particles and surfaces.
 - Particle properties are displayed in the status bar when hovering over a particle in the viewports.
 - Periodic boundary conditions can be overridden by the user without the changes being lost when a new simulation frame is loaded.
 - Bug fix: Implemented workaround for high-quality particle rendering on Windows computers with Intel HD 4000 graphics.
 - Bug fix: StrainTensor.XZ and StrainTensor.YZ components output by Atomic Strain modifier were swapped.
 - Bug fix: Fixed issue in Histogram modifier that occured when the fixed x-range is smaller than the value range.
 - Bug fix: Atom type ordering is now maintained when importing a sequence of LAMMPS dump files with named atom types.
 - Performance improvements in modification pipeline by avoiding unnecessary particle data copies and initialization operations.
 - The 'Freeze Selection' modifier has been superseded by the more general 'Freeze Property' modifier.
 
Release 2.4.0 (16-Sep-14):

 - New Python script engine (see user manual for more information).
 - Added the 'Voronoi analysis' modifier, which can compute atomic volumes, coordination numbers and Voronoi indices.
 - Added import/export support for extended XYZ format (see http://jrkermode.co.uk/quippy/io.html#extendedxyz),
   which includes metadata describing the file's columns and the simulation cell.
 - Improved input and output performance for text-based file formats.
 - Added calculation of non-affine displacements to 'Atomic strain' modifier. 
   (This is Falk & Langer's D^2_min measure, see the 1998 PRB.)
 - Added 'Bin and reduce' analysis modifier.
 - The 'Create bonds' modifier can now handle particles that are located outside the (periodic) simulation box.
 - Added a file parser for PDB files (still experimental).
 - The 'Show periodic images' modifier can now assign unique IDs to particle copies.
 - LAMMPS data file parser now supports additional LAMMPS atom styles such as 'charge' and 'bond'.
 - Command line options to run old Javascript programs have been renamed to --jsscript and --jsexec.
   Javascript plugin has been deprecated and will be removed in a future program version.
 - Bug fix: Export of compressed LAMMPS data files could result in truncated files.
 - Bug fix: Solid volume computed by 'Construct surface mesh' modifier could be inaccurate due to low numerical precision
 - Bug fix: 'Construct surface mesh' modifier crashed with certain input data.
 - Bug fix: VTK mesh file parser couldn't handle multiple points per line (as written by ParaView).
 - Bug fix: LAMMPS data file parser did not parse atom IDs.  

Release 2.3.3 (22-May-14):

 - Added user options to application settings dialog that give control over sharing of 
   OpenGL contexts and the use of OpenGL point sprites. This allows to work around compatibility
   problems on some systems.
 - User can now choose between dark and light viewport color schemes.
 - Added scripting interface for Tachyon renderer.
 - Added support for variable particle numbers in NetCDF reader (i.e. support for unlimited atom dimension)
   and for NC_CHAR variables as particle types. (I.e. particle types given by names instead of numbers.)
 - Added user options that control the automatic fetching of the news page from the web server and 
   the transmission of the installation ID.
 - Fixed bug in camera orbit mode, not correctly restricting camera's orientation when coordinate system 
   orientation has been changed.

Release 2.3.2 (07-Apr-14):

 - Fixed bug in Wigner-Seitz analysis modifier, which could cause a program crash when numbers of 
   atoms in reference and current configuration differ.
 
Release 2.3.1 (01-Apr-14):

 - Added saving/loading of presets to the file column mapping dialog.
 - Added the --exec command line option, which allows to directly execute a script command or to pass parameters to a script file.
 - When opening a XYZ file, the column mapping dialog displays an excerpt of the file content to help the user in figuring out the mapping.
 - The Construct Surface Modifier no longer creates cap polygons if the simulation cell doesn't contain any particles.

Release 2.3.0 (29-Mar-14):

 - Added the new scripting interface, which allows to automate tasks.
 - Added the 'Freeze property' modifier, which can prevent a particle property from changing over time.
 - Added the 'Scatter plot' modifier, which plots one particle property against another. 
   This modifier has been contributed by Lars Pastewka.
 - Added the 'Wigner-Seitz analysis' modifier, which can identify vacancies and interstitials in a lattice.
 - Added a file importer for NetCDF files. Code was contributed by Lars Pastewka.
 - Added more input variables to the 'Compute property' and 'Expression select' modifiers (e.g. reduced particle 
   coordinates and simulation cell size).
 - It's now possible to load a sequence of files with each file containing multiple frames. To do this, import the 
   first file from the sequence, activate the option "File contains multiple timesteps", finally open the 
   "Frame sequence" panel and change the wildcard pattern to include the '*' placeholder character again.
 - Fixed bug in CFG file importer, which did not read triclinic simulation cells correctly.
 - Fixed shader compilation error on OpenGL 2.0 systems and some other OpenGL related issues.
   
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