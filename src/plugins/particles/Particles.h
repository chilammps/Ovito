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

#ifndef __OVITO_PARTICLES_H
#define __OVITO_PARTICLES_H

#include <core/Core.h>

#ifdef MAKING_MODULE_PARTICLES
#  define OVITO_PARTICLES_EXPORT Q_DECL_EXPORT
#else
#  define OVITO_PARTICLES_EXPORT Q_DECL_IMPORT
#endif

/*! \namespace Ovito::Plugins::Particles
    \brief This root namespace of the particles plugin.
*/
/*! \namespace Ovito::Plugins::Particles::Data
    \brief This namespace contains container classes for particle related data.
*/
/*! \namespace Ovito::Plugins::Particles::Objects
    \brief This namespace contains data object classes.
*/
/*! \namespace Ovito::Plugins::Particles::Objects::Display
    \brief This namespace contains display objects for particle-related data.
*/
/*! \namespace Ovito::Plugins::Particles::Import
    \brief This namespace contains file importers for particle data.
*/
/*! \namespace Ovito::Plugins::Particles::Export
    \brief This namespace contains file exporters for particle data.
*/
/*! \namespace Ovito::Plugins::Particles::Modifiers
    \brief This namespace contains modifiers for particle data
*/
/*! \namespace Ovito::Plugins::Particles::Util
    \brief This namespace contains various particle-related utility classes.
*/

namespace Ovito { namespace Plugins { namespace Particles {

	namespace Objects {
		class ParticlePropertyObject;
		namespace Display {}
	}
	namespace Data {
		class ParticleProperty;
	}
	namespace Import {}
	namespace Export {}
	namespace Modifiers {}
	namespace Util {
		class TreeNeighborListBuilder;
		class OnTheFlyNeighborListBuilder;
		class ParticlePropertyComboBox;
	}

	using namespace Objects;
	using namespace Objects::Display;
	using namespace Data;
	using namespace Import;
	using namespace Export;
	using namespace Modifiers;
	using namespace Util;

}}}

#endif
