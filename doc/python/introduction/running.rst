==================================
Running scripts
==================================

This section explains how to use OVITO's built-in script interpreter.

.. topic:: Note to 64-bit Windows users:

	Due to compiler limitations, Python scripting is currently NOT available in the 64-bit version of OVITO for Windows.
	Please use the 32-bit version of OVITO instead if you want to use this feature on Windows.
	The 32-bit Windows version of the program runs on 64-bit operating systems too.

OVITO's Python interpreter
----------------------------------

OVITO includes a built-in script interpreter, which can execute programs written in the Python language.
The current version of OVITO is compatible with the Python 2.7 language standard. 
You typically execute a Python script from the terminal using the :program:`ovitos` script launcher that comes with OVITO:

.. code-block:: shell-session

	ovitos [-o file] [-g] [script.py] [args...]
	
The :program:`ovitos` program is located in the :file:`bin/` subdirectory of OVITO for Linux, in the 
:file:`Ovito.app/Contents/MacOS/` directory of OVITO for MacOS, and in the main program directory 
on Windows systems. It should not be confused with :program:`ovito`, the main OVITO program
showing a graphical user interface.

Let's assume we used a text editor to write a simple Python script file named :file:`hello.py`::

	import ovito
	print "Hello, this is OVITO %i.%i.%i" % ovito.version

We can execute the script from a Linux terminal as follows:

.. code-block:: shell-session

	me@linux:~/ovito-2.4.0-x86_64/bin$ ./ovitos hello.py
	Hello, this is OVITO 2.4.0
	
By default, the :program:`ovitos` script launcher invokes OVITO in console mode, which is a non-graphical mode
where the main window isn't shown. This allows running OVITO scripts on remote machines or
computing clusters that don't possess a graphics terminal. In OVITO's console mode, scripts can read from and write
to the terminal as if they were executed by a standard Python interpreter. Any command line arguments following the 
script's name are passed to the script via the ``sys.argv`` variable. Furthermore, it is possible to start OVITO's 
interpreter in interactive scripting mode by running :program:`ovitos` without specifying a script file.

The :command:`-o` command line option loads an OVITO scene file before executing the
script. This allows you to preload and use an existing visualization setup that has 
been manually prepared using the graphical version of OVITO and saved to a :file:`.ovito` file. This can save you programming
work, because modifiers, parameters, and the camera setup get loaded from the OVITO file and 
don't have to be set up programatically in the script anymore.

The :command:`-g` command line option switches from console mode to graphical mode. This displays OVITO's main window
and you can follow your script's actions as they are being executed. This is useful, for instance, if you want to visually 
inspect the results of your script and check if everything is correctly set up during the development of a script.

.. note::

	With the current program version it is not possible to run OVITO scripts with 
	a standard Python interpreter (usually named :program:`python`). OVITO scripts must be executed with OVITO's built-in interpreter. 
	If you want to use a third-party Python package in your OVITO scripts, you can install it in OVITO's built-in interpreter as usual
	(Use :program:`ovitos` instead of :program:`python` to run the package install script).
