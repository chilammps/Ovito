///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
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

#include <core/Core.h>
#include "ParticlePropertyComboBox.h"

namespace Ovito { namespace Plugins { namespace Particles { namespace Util {

/******************************************************************************
* Returns the particle property that is currently selected in the combo box.
******************************************************************************/
ParticlePropertyReference ParticlePropertyComboBox::currentProperty() const
{
	int index = currentIndex();
	if(!isEditable()) {
		if(index < 0)
			return ParticlePropertyReference();
		return itemData(index).value<ParticlePropertyReference>();
	}
	else {
		if(index >= 0) {
			QVariant data = itemData(index);
			if(data.canConvert<ParticlePropertyReference>())
				return data.value<ParticlePropertyReference>();
		}
		QString name = currentText().simplified();
		if(!name.isEmpty()) {
			QMap<QString, ParticleProperty::Type> stdplist = ParticleProperty::standardPropertyList();
			auto entry = stdplist.find(name);
			if(entry != stdplist.end())
				return ParticlePropertyReference(entry.value());
			return ParticlePropertyReference(name);
		}
		return ParticlePropertyReference();
	}
}

}}}}	// End of namespace
