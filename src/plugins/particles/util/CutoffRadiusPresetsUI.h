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

#ifndef __OVITO_CUTOFF_RADIUS_PRESETS_H
#define __OVITO_CUTOFF_RADIUS_PRESETS_H

#include <plugins/particles/Particles.h>
#include <core/gui/properties/ParameterUI.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

class OVITO_PARTICLES_EXPORT ChemicalElement
{
public:

	enum CrystalStructure {
		Unknown, SimpleCubic, FaceCenteredCubic, BodyCenteredCubic,
		HexagonalClosePacked, Tetragonal, Diatom, Diamond, Orthorhombic,
		Cubic, Monoclinic, Atom, Rhombohedral
	};

	CrystalStructure structure;
	FloatType latticeParameter;

	const char* elementName;
};

extern ChemicalElement ChemicalElements[];
extern const size_t NumberOfChemicalElements;

class OVITO_PARTICLES_EXPORT CutoffRadiusPresetsUI : public PropertyParameterUI
{
public:

	/// Constructor for a PropertyField property.
	CutoffRadiusPresetsUI(PropertiesEditor* parentEditor, const PropertyFieldDescriptor& propField);

	/// Destructor.
	virtual ~CutoffRadiusPresetsUI();

	/// This returns the QComboBox managed by this ParameterUI.
	QComboBox* comboBox() const { return _comboBox; }

	/// Sets the enabled state of the UI.
	virtual void setEnabled(bool enabled) override;

	/// This method is called when a new editable object has been assigned to the properties owner this
	/// parameter UI belongs to.
	virtual void resetUI() override;

public:

	Q_PROPERTY(QComboBox comboBox READ comboBox)

protected Q_SLOTS:

	/// Is called when the user has selected an item in the cutoff presets box.
	void onSelect(int index);

protected:

	/// The combo box control of the UI component.
	QPointer<QComboBox> _comboBox;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_CUTOFF_RADIUS_PRESETS_H
