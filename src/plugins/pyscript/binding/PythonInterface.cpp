///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#include <plugins/pyscript/PyScript.h>
#include <core/gui/app/Application.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

BOOST_PYTHON_MODULE(PyScript)
{
	docstring_options docoptions(true, false);

	// Make Ovito program version number available to script.
	scope().attr("version") = make_tuple(OVITO_VERSION_MAJOR, OVITO_VERSION_MINOR, OVITO_VERSION_REVISION);

	// Make environment information available
	scope().attr("gui_mode") = Application::instance().guiMode();
	scope().attr("headless_mode") = Application::instance().headlessMode();
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(PyScript);

};
