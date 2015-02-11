# This Python helper script runs PIP within OVITO.
# This can be is used to uninstall Python modules from the Python interpreter that is shipped with OVITO.
#
# For example:
#
#    ovitos run_pip.py uninstall sphinx
#
import runpy
runpy.run_module('pip', run_name='__main__')
