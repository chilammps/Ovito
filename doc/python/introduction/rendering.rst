===================================
Rendering pictures
===================================

-----------------------------------
Display objects
-----------------------------------

In OVITO, *data objects* are separated from *display objects*, which are responsible for
producing a visual representation of the data. For example, a :py:class:`~ovito.data.SimulationCell` 
is a pure data object, which stores the simulation cell vectors and the periodic boundary flags. 
The corresponding display object (a :py:class:`~ovito.vis.SimulationCellDisplay`)
takes this information to generate the actual box geometry to visualize the dimensions of the simulation
cell in the viewports. The display object also stores parameters such as the simulation cell display color
and line width, which control the visual appearance.

The display object is attached to the data object and can be accessed through the :py:attr:`~ovito.data.DataObject.display`
attribute of the :py:class:`~ovito.data.DataObject` base class::

    >>> cell = node.source.data.cell           
    >>> cell                                     # This is the data object
    <SimulationCell at 0x7f9a414c8060>
    
    >>> cell.display                             # This is the attached display object
    <SimulationCellDisplay at 0x7fc3650a1c20>

    >>> cell.display.rendering_color = (1,0,0)   # Giving the simulation box a red color
    
All display objects are derived from the :py:class:`~ovito.vis.Display` base class, which provides
the :py:attr:`~ovito.vis.Display.enabled` attribute to turn the display on or off::

    >>> cell.display.enabled = False         # This hides the simulation cell
    
The visual display of particles is controlled by a :py:class:`~ovito.vis.ParticleDisplay` object, which
is attached to the position :py:class:`~ovito.data.ParticleProperty`. For example, to display 
cubic particles, we would write::

    >>> pos = node.source.data.position      # ParticleProperty storing the positions
    >>> pos.display.shape = ParticleDisplay.Shape.Square

.. note::

    Note that display objects flow down the modification pipeline together with the data objects they are
    attached to. Normally they are not modified by modifiers in the pipeline, only the data objects are.
    That means it doesn't matter whether you change display parameters in the input of the modification pipeline
    or in the output. In the examples above we have accessed the input data collection (``node.source.data``),
    but changing the display parameters in the output data collection (``node.output``) would have worked
    equally well.
    
Some modifiers produce new data objects when the modification pipeline is evaluated.
For example, the :py:class:`~ovito.modifiers.CalculateDisplacementsModifier` generates a new :py:class:`~ovito.data.ParticleProperty` 
that stores the computed displacement vectors. To enable the display of displacement vectors
as arrows, the :py:class:`~ovito.modifiers.CalculateDisplacementsModifier` attaches a
:py:class:`~ovito.vis.VectorDisplay` to the new particle property. We can access this display object
in two equivalent ways: either directly though the :py:attr:`~ovito.modifiers.CalculateDisplacementsModifier.vector_display` attribute of the modifier::

    >>> modifier = CalculateDisplacementsModifier()
    >>> node.modifiers.append(modifier)
    >>> modifier.vector_display.enabled = True       # Enable the display of arrows
    >>> modifier.vector_display.color = (0,0,1)      # Give arrows a blue color

or via the :py:attr:`~ovito.data.DataObject.display` attribute of the resulting particle property::

    >>> node.compute()                                      # Ensure pipeline output exists
    >>> node.output.displacement.display.enabled = True     # Enable the display of arrows
    >>> node.output.displacement.display.color = (0,0,1)    # Give arrows a blue color
    
Similarly, the :py:class:`~ovito.modifiers.CreateBondsModifier` attached a :py:class:`~ovito.vis.BondsDisplay`
to the :py:class:`~ovito.data.Bonds` data object it computes.
    
-----------------------------------
Viewports
-----------------------------------

A :py:class:`~ovito.vis.Viewport` defines the view of the three-dimensional scene, in which the display
objects generate a visual representation of the data. To render a picture of the scene from a script, you
typically create a new *ad hoc* :py:class:`~ovito.vis.Viewport` instance and configure it by setting 
the camera position and orientation::

    >>> from ovito.vis import *
    >>> vp = Viewport()
    >>> vp.type = Viewport.Type.PERSPECTIVE
    >>> vp.camera_pos = (-100, -150, 150)
    >>> vp.camera_dir = (2, 3, -3)
    >>> vp.fov = math.radians(60.0)

As known from the graphical OVITO program, there exist various viewport types such as ``TOP``, ``FRONT``, ``PERSPECTIVE``, etc. 
The ``PERSPECTIVE`` and ``ORTHO`` viewport types allow you to freely orient the camera in space and
are usually what you want. Don't forget to set the viewport type first before setting up the camera as demonstrated
in the example above. That's because changing the viewport type will reset the camera to a default orientation.

The ``PERSPECTIVE`` viewport type uses a perspective projection, and you specify the field of view 
(:py:attr:`~ovito.vis.Viewport.fov`) as an angle (measured vertically). The ``ORTHO`` viewport type
uses a parallel projection; then the :py:attr:`~ovito.vis.Viewport.fov` parameter specifies the size of the visible
area in the vertical direction in length units. You can call the :py:meth:`Viewport.zoom_all() <ovito.vis.Viewport.zoom_all>`
method to let OVITO choose a reasonable camera zoom and position such that all objects are completely visible.

OVITO's graphical user interface defines four standard viewports. You can access and manipulate them from a script via 
the :py:attr:`dataset.viewports <ovito.DataSet.viewports>` list.

-----------------------------------
Rendering
-----------------------------------

Rendering parameters such as image resolution, output filename, background color, etc. are managed by a 
:py:class:`~ovito.vis.RenderSettings` objects. You can create a new instance of this class and specify 
the necessary parameters::

    from ovito.vis import *
    settings = RenderSettings(
        filename = "myimage.png",
        size = (800, 600)
    )

OVITO provides two different rendering engines, which are responsible for producing the final image
of the scene. The default renderer is the :py:class:`~ovito.vis.OpenGLRenderer`, which uses a fast, hardware-accelerated
OpenGL rendering method. The second option is the :py:class:`~ovito.vis.TachyonRenderer`, which is
based on a software-only raytracing algorithm and is able to produce better looking results in some cases.
Each of these rendering engines has specific parameters, and you can access the current renderer object
through the :py:attr:`RenderSettings.renderer <ovito.vis.RenderSettings.renderer>` attribute::

    settings.renderer = TachyonRenderer() # Replace default OpenGLRenderer with TachyonRenderer
    settings.renderer.shadows = False     # Turn off cast shadows
    
After all render settings have been specified, we can let OVITO render the image by calling 
:py:meth:`Viewport.render() <ovito.vis.Viewport.render>`::

    vp.render(settings)
