==================================
Running scripts
==================================

This section explains how to use OVITO's built-in script interpreter.

OVITO's Python interpreter
----------------------------------

OVITO includes a built-in script interpreter, which can execute programs written in the Python language.
The current version of OVITO is compatible with the `Python 3.4 <https://docs.python.org/3.4/>`_ language standard. 
You typically execute a Python script from the terminal using the :program:`ovitos` script launcher that comes with OVITO:

.. code-block:: shell-session

	ovitos [-o file] [-g] [script.py] [args...]
	
The :program:`ovitos` program is located in the :file:`bin/` subdirectory of OVITO for Linux, in the 
:file:`Ovito.app/Contents/MacOS/` directory of OVITO for MacOS, and in the main program directory 
on Windows systems. It should not be confused with :program:`ovito`, the main program, which
provides a graphical user interface.

Let's assume we used a text editor to write a simple Python script file named :file:`hello.py`::

	import ovito
	print("Hello, this is OVITO %i.%i.%i" % ovito.version)

We can execute the script from a Linux terminal as follows:

.. code-block:: shell-session

	me@linux:~/ovito-2.4.0-x86_64/bin$ ./ovitos hello.py
	Hello, this is OVITO 2.4.0
	
By default, the :program:`ovitos` script launcher invokes OVITO in console mode, which is a non-graphical mode
where the main window isn't shown. This allows running OVITO scripts on remote machines or
computing clusters that don't possess a graphics terminal. In OVITO's console mode, scripts can read from and write
to the terminal as if they were executed by a standard Python interpreter. Any command line arguments following the 
script's name are passed to the script via the ``sys.argv`` variable. Furthermore, it is possible to start OVITO's 
interpreter in interactive scripting mode by running :program:`ovitos` without any arguments.

The :command:`-o` command line option loads an OVITO scene file before executing the
script. This allows you to preload and use an existing visualization setup that has 
been manually prepared using the graphical version of OVITO and saved to a :file:`.ovito` file. This can save you programming
work, because modifiers, parameters, and the camera setup get loaded from the OVITO file and 
don't have to be set up programatically in the script anymore.

The :command:`-g` command line option switches from console mode to graphical mode. This displays OVITO's main window
and you can follow your script's actions as they are being executed. This is useful, for instance, if you want to visually 
inspect the results of your script and check if everything is correctly set up during the development of a script.

.. note::

	It is not possible to run scripts written for OVITO with a standard Python interpreter (usually named :program:`python`). 
	They must always be executed with the launcher :program:`ovitos`. The Python interpreter shipping with OVITO
	includes only the standard Python modules and `NumPy <http://www.numpy.org/>`_, a popular package for working with numeric data.
	
	If you want to use other third-party Python packages in your OVITO scripts, it might be possible to install them in the 
	built-in interpreter using the normal *setuptools* mechanism. 
	(Use :program:`ovitos` instead of :program:`python` to run the *setup.py* installation script).

	Installing Python extension that include native code (e.g. `Scipy <http:://www.scipy.org>`_) in the interpreter that ships with OVITO is currently not possible.
	In this case it is recommended to build OVITO from source. OVITO will then make use of the system's standard Python interpreter.	
	All modules that are available in the standard Python interpreter will also be accessible within OVITO. (Note that you still need
	to execute OVITO scripts with the :program:`ovitos` launcher.) How to build OVITO from source is explained `on this page <http://www.ovito.org/manual/development.html>`_.
	
