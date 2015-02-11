# This Python helper script simply runs pip.
# This can be is used to uninstall Python modules from the Python interpreter that is shipped with OVITO.
#
# For example:
#
#    ovitos run_pip.py uninstall sphinx
#
import sys
import os.path
import pip
sys.executable = os.path.join(os.path.dirname(sys.executable), "ovitos")
pip.main()
