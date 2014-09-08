==================================
Overview
==================================

OVITO scripting interface provides full access to most of OVITO's program features. Using Python scripts, you can
do many things that are already familiar from the graphical user interface (and even a few more):

  * Import data from simulation files
  * Apply a chain of modifiers to the data
  * Let OVITO compute the results of the modifier pipeline
  * Set up a camera and render pictures or movies of the scene
  * Control the visual appearance of particles and other objects
  * Access per-particle data and other analysis results computed by OVITO
  * Export the output data to a file

This following sections will introduce the essential concepts and walk you through different parts of OVITO's 
scripting interface.

------------------------------------
OVITO's data flow architecture
------------------------------------

If you have worked with OVITO's graphical user interface before, you should already be familiar with 
its key workflow concept: You typically load a simulation file into OVITO and set up a sequence of modifiers 
that act on that input data. The results of this *modification pipeline* are computed by OVITO 
and displayed in the interactive viewports.

To make use of this capability from a script, we first need to understand the basics of OVITO's underlying 
data model. In general, there are two different groups of objects that participate in the described system: 
Objects that constitute the modification pipeline (e.g. modifiers and a data source) and *data objects*, which 
enter the modification pipeline, get modified by it, or are newly produced by modifiers (e.g. particle properties). 
We will start by discussing the first group in the next section.

------------------------------------
Data sources, modifiers, and more
------------------------------------

It all begins with the *data source*, which is an object
that provides the input data entering a modification pipeline. OVITO currently knows only one type of 
data source: a :py:class:`~ovito.io.FileSource`. It is responsible for loading data from an external file and
passing it to the modification pipeline. 

The data source and the modification pipeline both belong to an :py:class:`~ovito.ObjectNode` instance. This object
orchestrates the data flow from the source into the modification pipeline and stores the pipeline's output in an internal 
data cache. As we will see later, the :py:class:`~ovito.ObjectNode` is also responsible for displaying the output
data in the three-dimensional scene. The data source is stored in the node's :py:attr:`~ovito.ObjectNode.source`
attribute, while the modification pipeline is accessible through the node's :py:attr:`~ovito.ObjectNode.modifiers`
property. The pipeline is simply a Python list that can be populated with modifiers. They will be applied to the
input data one after the other.

An :py:class:`~ovito.ObjectNode` is usually placed in the *scene*, i.e. the three-dimensional world that is visible
through OVITO's viewports. The objects in the scene, and all other information that would get saved along in 
a :file:`.ovito` file (e.g. current render settings, viewport cameras, etc.), comprise the so-called :py:class:`~ovito.DataSet`. 
A Python script always runs in the context of exactly one global :py:class:`~ovito.DataSet` instance. This 
object can be accessed through the :py:data:`ovito.dataset` module-level attribute. It provides access to the
list of object nodes in the scene (:py:attr:`dataset.scene_nodes <ovito.DataSet.scene_nodes>`), 
the current animation settings (:py:attr:`dataset.anim <ovito.DataSet.anim>`), the four 
viewports in OVITO's main window (:py:attr:`dataset.viewports <ovito.DataSet.viewports>`), and more.

------------------------------------
Loading data and applying modifiers
------------------------------------

After the general object model has been described above, it is now time to give some code examples and demonstrate how
we deal with these things in a script. Usually, we first want to load a simulation file. This is done
using the :py:func:`ovito.io.import_file` function::

   >>> from ovito.io import *
   >>> node = import_file("simulation.dump")
   
This high-level function does several things: It creates a :py:class:`~ovito.io.FileSource` (which will load data 
from the given file), it creates an :py:class:`~ovito.ObjectNode` instance with an empty modification pipeline, and assigns the 
:py:class:`~ovito.io.FileSource` to the :py:attr:`~ovito.ObjectNode.source` property of the node. The function finally returns the 
newly created node to the caller after it has been inserted into the scene.

We can now populate the node's modification pipeline with some modifiers::

   >>> from ovito.modifiers import *
   >>> node.modifiers.append(SelectExpressionModifier(expression = "PotentialEnergy < -3.9"))
   >>> node.modifiers.append(DeleteSelectedParticlesModifier())

Here we have created two modifiers and appended them to the modification pipeline. Note how modifier parameters 
can be initialized:

.. note::

   When constructing new objects such as modifiers it is possible to initialize object
   parameters using an arbitrary number of keyword arguments at construction time. Thus ::
   
       node.modifiers.append(CommonNeighborAnalysisModifier(cutoff = 3.2, adaptive_mode = False))
       
   is equivalent to::

       modifier = CommonNeighborAnalysisModifier()
       modifier.cutoff = 3.2
       modifier.adaptive_mode = False
       node.modifiers.append(modifier)
       
After the modification pipeline has been populated with the desired modifiers, we can basically do three things:
(i) write the results to a file, (ii) render an image of the data, (iii) or directly work with the pipeline 
data and read out particle properties, for instance.

------------------------------------
Exporting data to a file
------------------------------------

Exporting the processed data to a file is simple; we use the :py:func:`ovito.io.export_file` function
for this::

    >>> export_file(node, "outputdata.dump", "lammps_dump",
    ...    columns = ["Position.X", "Position.Y", "Position.Z", "Structure Type"])
    
This high-level function accepts the node whose pipeline results should be exported as its first parameter.
Furthermore, the name of the output file and the format need to be specified. Depending on the selected file format,
additional parameters such as the list of particle properties to be exported must be provided.

------------------------------------
Rendering images
------------------------------------

To generate an image of the data, we first need a viewport that defines the view on the three-dimensional scene.
We can either use one of the four predefined viewports of OVITO for this, or simply create an *ad hoc* 
viewport in Python::

    >>> from ovito.vis import *
    >>> vp = Viewport()
    >>> vp.type = Viewport.Type.PERSPECTIVE
    >>> vp.camera_pos = (-100, -150, 150)
    >>> vp.camera_dir = (2, 3, -3)
    >>> vp.fov = math.radians(60.0)
    
As you can see, the :py:class:`~ovito.vis.Viewport` class has several parameters that control the 
position and orientation of the camera, the projection type, and the field of view (FOV) angle. Note that this
viewport will not be visible in OVITO's main window; it's only a temporary object used in the script.

In addition we need to create a :py:class:`~ovito.vis.RenderSettings` object, which controls the rendering
process (These are the parameters you normally set on the :guilabel:`Render` tab in OVITO's main window)::

    >>> settings = RenderSettings()
    >>> settings.filename = "myimage.png"
    >>> settings.size = (800, 600)
    
Here we have specified the output filename and the size of the image in pixels. Finally, we can let OVITO render 
the image::

    >>> vp.render(settings)
    
Note again how we could have used the more compact notation instead to initialize object parameters:: 

    vp = Viewport(
        type = Viewport.Type.PERSPECTIVE,
        camera_pos = (-100, -150, 150),
        camera_dir = (2, 3, -3),
        fov = math.radians(60.0)
    )
    vp.render(RenderSettings(filename = "myimage.png", size = (800, 600)))

------------------------------------
Accessing computation results
------------------------------------

OVITO's scripting interface allows us to directly access the output data that leaves the
modification pipeline. But first we have to let OVITO compute the results of the modification pipeline for us::

    >>> node.compute()
    
The node's :py:meth:`~ovito.ObjectNode.compute` method ensures that all modifiers in the pipeline
have been successfully evaluated. Note that the :py:meth:`~ovito.vis.Viewport.render` and 
:py:func:`~ovito.io.export_file` functions discussed above implicitly call :py:meth:`~ovito.ObjectNode.compute`
for us. But now, since we are not using any of these high-level functions, we have to explicitly request 
an evaluation of the modification pipeline.

The node stores the results of the last pipeline evaluation in its :py:attr:`~ovito.ObjectNode.output` field::

    >>> node.output
    DataCollection(['Simulation cell', 'Particle identifiers', 'Particle positions', 
                    'Potential Energy', 'Particle colors', 'Structure types'])
    
The :py:class:`~ovito.data.DataCollection` contains the *data objects* that were output
by the modification pipeline. For example, to access the simulation cell we would write::

    >>> node.output.cell.matrix
    [[ 148.147995      0.            0.          -74.0739975 ]
     [   0.          148.07200623    0.          -74.03600311]
     [   0.            0.          148.0756073   -74.03780365]]
     
    >>> node.output.cell.pbc
    (True, True, True)
     
Similarly, the data of individual particle properties may be accessed as NumPy arrays:

    >>> import numpy
    >>> node.output.position.array
    [[ 73.24230194  -5.77583981  -0.87618297]
     [-49.00170135 -35.47610092 -27.92519951]
     [-50.36349869 -39.02569962 -25.61310005]
     ..., 
     [ 42.71210098  59.44919968  38.6432991 ]
     [ 42.9917984   63.53770065  36.33330154]
     [ 44.17670059  61.49860001  37.5401001 ]]
     
Sometimes we might be interested in the data that *enters* the modification pipeline. 
This input data, which was read from the external file, 
is cached by the :py:class:`~ovito.io.FileSource` and can be accessed through the 
:py:attr:`~ovito.io.FileSource.data` attribute::

    >>> node.source.data
    DataCollection(['Simulation cell', 'Particle identifiers', 'Particle positions'])

-------------------------------------------------
Controlling the visual appearance of objects
-------------------------------------------------

So far we have only considered data objects such as particle properties or the simulation cell
that are processed in OVITO's modification pipeline system. How are these data objects displayed, and how
can we set the parameters that control their visual appearance?

Every data object that has a visual representation in OVITO is associated with a special :py:class:`~ovito.vis.Display`
object. It is stored in the data object's :py:attr:`~.ovito.data.DataObject.display` attribute. For example::

    >>> cell = node.source.data.cell           
    >>> cell                               # This is the data object
    <SimulationCell at 0x7f9a414c8060>
    
    >>> cell.display                       # This is its associated display object
    <SimulationCellDisplay at 0x7fc3650a1c20>

In this example we have accessed the :py:class:`~ovito.data.SimulationCell` data object from the 
file source's data collection. Its :py:attr:`~.ovito.data.DataObject.display` attribute contains
a :py:class:`~ovito.vis.SimulationCellDisplay` instance, which is responsible for producing
the visual representation of the simulation cell. It provides parameters that allow us to control
the appearance of the cell. We can even turn off the display of the simulation cell completely::

    >>> cell.display.enabled = False 

Particles are being rendered by a :py:class:`~ovito.vis.ParticleDisplay` object. Since there is no dedicated 
data object for particles in OVITO, only separate data objects that store the individual particle properties, OVITO associates
the :py:class:`~ovito.vis.ParticleDisplay` with the :py:class:`~ovito.data.ParticleProperty` data object
containing the particle positions. Thus, to modify the particle display properties, we have to access the particle 
positions::

    >>> p = node.source.data.position           
    >>> p                        # This is the data object holding the input particle positions
    <ParticleProperty at 0x7ff5fc868b30>
      
    >>> p.display                # This is the associated display object
    <ParticleDisplay at 0x7ff5fc868c40>
       
    >>> p.display.shading = ParticleDisplay.Shading.Flat
    >>> p.display.radius = 1.4

.. note::

    Note that display objects flow through the modification pipeline together with the data objects they are
    associated with. Normally they don't get modified by modifiers in the pipeline, only the data objects are.
    That means it doesn't matter whether we change display parameters in the input of the modification pipeline
    or in the output.
    
    However, some modifiers such as the :py:class:`~ovito.modifiers.CalculateDisplacementsModifier` 
    create new data objects (in this case a :py:class:`~ovito.data.ParticleProperty` holding the computed
    displacement vectors). Such newly generated data objects may be associated with a display object too
    (a :py:class:`~ovito.vis.VectorDisplay` in this case), which will only be accessible in the pipeline output
    or via the modifier itself.
    
