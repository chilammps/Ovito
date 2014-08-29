==================================
Overview
==================================

OVITO scripting interface provides full access to most of OVITO's program features. Using Python scripts, you can
do many things already familiar from the graphical user interface (and even a few more):

  * Import data from simulation files
  * Apply a chain of modifiers to the data
  * Let OVITO compute the results of the modifier pipeline
  * Set up a camera and render pictures or movies of the scene
  * Control the visual appearance of particles and other objects
  * Access per-particle data and analysis results computed by OVITO
  * Export data to a file

This following sections will introduce the essential concepts and walk you through the different parts of OVITO's scripting interface.

------------------------------------
OVITO's data model
------------------------------------

If you have worked with OVITO's graphical user interface before, you should already be familiar with 
its main workflow concept: You typically load a simulation file into OVITO and set up a sequence of modifiers 
that act on that input data. The results of this *modification pipeline* are computed by OVITO 
and displayed on screen in the interactive viewports.

To effectively control this process from a script, we first need to understand the basics of OVITO's underlying 
data model. There are two different groups of objects that participate in this data flow architecture: 
Those that form the modification pipeline (e.g. modifiers) and *data objects*, which are processed or generated
by the modification pipeline (e.g. particle properties). 

We will discuss the first group of objects first. 