==================================
Running scripts
==================================

This section explains how to use OVITO's built-in script interpreter.

.. topic:: Note to 64-bit Windows users:

	Due to compiler limitations, Python scripting support is currently NOT available in the 64-bit version of OVITO for Windows.
	Please use the 32-bit version of OVITO instead if you want to use this feature under Windows.
	The 32-bit version of the program runs on 64-bit systems too.

OVITO's Python interpreter
----------------------------------

OVITO includes a built-in Python interpreter, which allows it to execute scripts written in the Python language.
The current version of OVITO is compatible with the Python 2.7 language standard. 
You normally execute a Python script from the terminal using the :program:`ovitos` script launcher that comes with OVITO:

.. code-block:: shell-session

	ovitos [-o file] [script.py] [args...]
	
The :program:`ovitos` launcher is located in the :file:`bin/` subdirectory of OVITO for Linux, in the 
:file:`Ovito.app/Contents/MacOS/` directory of OVITO for MacOS, and in the main directory 
on Windows systems.

Let's assume we have a simple Python script file named :file:`hello.py`::

	import ovito
	print "Hello, this is OVITO %i.%i.%i" % ovito.version
	
We can run the script from the system terminal as follows:

.. code-block:: shell-session

	me@linux:~/ovito-2.4.0-x86_64/bin$ ./ovitos hello.py
	Hello, this is OVITO 2.4.0
	
By default, the :program:`ovitos` script launcher invokes OVITO in console mode, which is a non-graphical mode,
where no main window is shown. This allows running OVITO scripts on remote machines or
compute clusters where no graphics terminal is available. OVITO scripts executed in console mode can read from and write
to the terminal as if they were executed by a standard Python interpreter. Any command line arguments that follow the 
script name are passed on to the script by :program:`ovitos` via the ``sys.argv`` variable. Furthermore, it is possible to start OVITO's 
interpreter in interactive mode by not specifying any script file.

The :command:`-o` option of the :program:`ovitos` launcher can be used to load an OVITO scene file before executing the
script. This makes it possible to let the script manipulate or use an existing visualization setup, which has 
been prepared using the graphical version of OVITO and saved to a :file:`.ovito` file. This can save you programming
work in some situations, because all settings, the modifiers, and the camera setup get already loaded from the OVITO file and don't have to
be explicitly set up programatically from the script.

.. note::

	In the current program version it is not possible to access OVITO's functions from scripts that are executed by
	the system Python interpreter :program:`python`. OVITO scripts must be run using the built-in interpreter of OVITO.
	If you want to use special Python packages from your OVITO script, you can install them as usual in 
	the search path of the build-in interpreter or by running their :command:`setup.py` installation script with 
	OVITO's interpreter.
