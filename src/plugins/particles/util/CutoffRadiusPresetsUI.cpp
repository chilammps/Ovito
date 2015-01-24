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

#include <plugins/particles/Particles.h>
#include "CutoffRadiusPresetsUI.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

ChemicalElement ChemicalElements[] = { 
    { ChemicalElement::Unknown, 0, NULL}, // X
    { ChemicalElement::Diatom, 0, "H"}, // H
    { ChemicalElement::Atom, 0, "He"}, // He
    { ChemicalElement::BodyCenteredCubic, 3.49f, "Li"}, // Li
    { ChemicalElement::HexagonalClosePacked, 2.29f, "Be"}, // Be
    { ChemicalElement::Tetragonal, 8.73f, "B"}, // B
    { ChemicalElement::Diamond, 3.57f, "C"}, // C
    { ChemicalElement::Diatom, 1.10f, "N"}, // N
    { ChemicalElement::Diatom, 1.21f, "O"}, // O
    { ChemicalElement::Diatom, 1.42f, "F"}, // F
    { ChemicalElement::FaceCenteredCubic, 4.43f, "Ne"}, // Ne
    { ChemicalElement::BodyCenteredCubic, 4.23f, "Na"}, // Na
    { ChemicalElement::HexagonalClosePacked, 3.21f, "Mg"}, // Mg
    { ChemicalElement::FaceCenteredCubic, 4.05f, "Al"}, // Al
    { ChemicalElement::Diamond, 5.43f, "Si"}, // Si
    { ChemicalElement::Cubic, 7.17f, "P"}, // P
    { ChemicalElement::Orthorhombic, 10.47f, "S"}, // S
    { ChemicalElement::Orthorhombic, 6.24f, "Cl"}, // Cl
    { ChemicalElement::FaceCenteredCubic, 5.26f, "Ar"}, // Ar
    { ChemicalElement::BodyCenteredCubic, 5.23f, "K"}, // K
    { ChemicalElement::FaceCenteredCubic, 5.58f, "Ca"}, // Ca
    { ChemicalElement::HexagonalClosePacked, 3.31f, NULL}, 
    { ChemicalElement::HexagonalClosePacked, 2.95f, "Ti"}, // Ti
    { ChemicalElement::BodyCenteredCubic, 3.02f, "V"}, // V
    { ChemicalElement::BodyCenteredCubic, 2.88f, "Cr"}, // Cr
    { ChemicalElement::Cubic, 8.89f, "Mn"}, // Mn
    { ChemicalElement::BodyCenteredCubic, 2.87f, "Fe"}, // Fe
    { ChemicalElement::HexagonalClosePacked, 2.51f, "Co"}, // Co
    { ChemicalElement::FaceCenteredCubic, 3.52f, "Ni"}, // Ni
    { ChemicalElement::FaceCenteredCubic, 3.61f, "Cu"}, // Cu
    { ChemicalElement::HexagonalClosePacked, 2.66f, "Zn"}, // Zn
    { ChemicalElement::Orthorhombic, 4.51f, "Ga"}, // Ga
    { ChemicalElement::Diamond, 5.66f, "Ge"}, // Ge
    { ChemicalElement::Rhombohedral, 0, "As"}, // As
    { ChemicalElement::HexagonalClosePacked, 4.36f, "Se"}, // Se
    { ChemicalElement::Orthorhombic, 6.67f, "Br"}, // Br
    { ChemicalElement::FaceCenteredCubic, 5.72f, "Kr"}, // Kr
    { ChemicalElement::BodyCenteredCubic, 5.59f, "Rb"}, // Rb
    { ChemicalElement::FaceCenteredCubic, 6.08f, "Sr"}, // Sr
    { ChemicalElement::HexagonalClosePacked, 3.65f, "Y"}, // Y
    { ChemicalElement::HexagonalClosePacked, 3.23f, "Zr"}, // Zr
    { ChemicalElement::BodyCenteredCubic, 3.30f, "Nb"}, // Nb
    { ChemicalElement::BodyCenteredCubic, 3.15f, "Mo"}, // Mo
    { ChemicalElement::HexagonalClosePacked, 2.74f, "Tc"}, // Tc
    { ChemicalElement::HexagonalClosePacked, 2.70f, "Ru"}, // Ru
    { ChemicalElement::FaceCenteredCubic, 3.80f, "Rh"}, // Rh
    { ChemicalElement::FaceCenteredCubic, 3.89f, "Pd"}, // Pd
    { ChemicalElement::FaceCenteredCubic, 4.09f, "Ag"}, // Ag
    { ChemicalElement::HexagonalClosePacked, 2.98f, "Cd"}, // Cd
    { ChemicalElement::Tetragonal, 4.59f, "In"}, // In
    { ChemicalElement::Tetragonal, 5.82f, "Sn"}, // Sn
    { ChemicalElement::Rhombohedral, 4.51f, "Sb"}, // Sb
    { ChemicalElement::HexagonalClosePacked, 4.45f, "Te"}, // Te
    { ChemicalElement::Orthorhombic, 7.27f, "I"}, // I
    { ChemicalElement::FaceCenteredCubic, 6.20f, "Xe"}, // Xe
    { ChemicalElement::BodyCenteredCubic, 6.05f, "Cs"}, // Cs
    { ChemicalElement::BodyCenteredCubic, 5.02f, "Ba"}, // Ba
    { ChemicalElement::HexagonalClosePacked, 3.75f, "La"}, // La
    { ChemicalElement::FaceCenteredCubic, 5.16f, "Ce"}, // Ce
    { ChemicalElement::HexagonalClosePacked, 3.67f, "Pr"}, // Pr
    { ChemicalElement::HexagonalClosePacked, 3.66f, "Nd"}, // Nd
    { ChemicalElement::Unknown, 0, "Pm"}, // Pm
    { ChemicalElement::Rhombohedral, 9.00f, "Sm"}, // Sm
    { ChemicalElement::BodyCenteredCubic, 4.61f, "Eu"}, // Eu
    { ChemicalElement::HexagonalClosePacked, 3.64f, "Gd"}, // Gd
    { ChemicalElement::HexagonalClosePacked, 3.60f, "Th"}, // Th
    { ChemicalElement::HexagonalClosePacked, 3.59f, "Dy"}, // Dy
    { ChemicalElement::HexagonalClosePacked, 3.58f, "Ho"}, // Ho
    { ChemicalElement::HexagonalClosePacked, 3.56f, "Er"}, // Er
    { ChemicalElement::HexagonalClosePacked, 3.54f, "Tm"}, // Tm
    { ChemicalElement::FaceCenteredCubic, 5.49f, "Yb"}, // Yb
    { ChemicalElement::HexagonalClosePacked, 3.51f, "Lu"}, // Lu
    { ChemicalElement::HexagonalClosePacked, 3.20f, "Hf"}, // Hf
    { ChemicalElement::BodyCenteredCubic, 3.31f, "Ta"}, // Ta
    { ChemicalElement::BodyCenteredCubic, 3.16f, "W"}, // W
    { ChemicalElement::HexagonalClosePacked, 2.76f, "Re"}, // Re
    { ChemicalElement::HexagonalClosePacked, 2.74f, "Os"}, // Os
    { ChemicalElement::FaceCenteredCubic, 3.84f, "Ir"}, // Ir
    { ChemicalElement::FaceCenteredCubic, 3.92f, "Pt"}, // Pt
    { ChemicalElement::FaceCenteredCubic, 4.08f, "Au"}, // Au
    { ChemicalElement::Rhombohedral, 2.99f, "Hg"}, // Hg
    { ChemicalElement::HexagonalClosePacked, 3.46f, "Tl"}, // Tl
    { ChemicalElement::FaceCenteredCubic, 4.95f, "Pb"}, // Pb
    { ChemicalElement::Rhombohedral, 4.75f, "Bi"}, // Bi
    { ChemicalElement::SimpleCubic, 3.35f, "Po"}, // Po
    { ChemicalElement::Unknown, 0, "At"}, // At
    { ChemicalElement::Unknown, 0, "Rn"}, // Rn
    { ChemicalElement::Unknown, 0, "Fr"}, // Fr
    { ChemicalElement::Unknown, 0, "Ra"}, // Ra
    { ChemicalElement::FaceCenteredCubic, 5.31f, "Ac"}, // Ac
    { ChemicalElement::FaceCenteredCubic, 5.08f, "Th"}, // Th
    { ChemicalElement::Tetragonal, 3.92f, "Pa"}, // Pa
    { ChemicalElement::Orthorhombic, 2.85f, "U"}, // U
    { ChemicalElement::Orthorhombic, 4.72f, "Np"}, // Np
    { ChemicalElement::Monoclinic, 0, "Pu"}, // Pu
};

const size_t NumberOfChemicalElements = (sizeof(ChemicalElements) / sizeof(ChemicalElements[0]));

IMPLEMENT_OVITO_OBJECT(Particles, CutoffRadiusPresetsUI, PropertyParameterUI);

/******************************************************************************
* Constructor.
******************************************************************************/
CutoffRadiusPresetsUI::CutoffRadiusPresetsUI(PropertiesEditor* parentEditor, const PropertyFieldDescriptor& propField) :
	PropertyParameterUI(parentEditor, propField)
{
	_comboBox = new QComboBox();
	
	QMap<ChemicalElement::CrystalStructure,QPair<QString,double> > HandledTypes;
	
	HandledTypes.insert(ChemicalElement::FaceCenteredCubic, qMakePair(QString("%1 (fcc) - %2"), 0.5 * (1.0/sqrt(2.0) + 1.0)));			// halfway between first and second nn shell
	HandledTypes.insert(ChemicalElement::BodyCenteredCubic, qMakePair(QString("%1 (bcc) - %2"), 0.5 * (1.0 + sqrt(2.0))  ));			// halfway between second and third nn shell
	HandledTypes.insert(ChemicalElement::Diamond,           qMakePair(QString("%1 (dia) - %2"), 0.5 * (1.0/sqrt(2.0) + sqrt(11)/4)));	// halfway between second and third nn shell

	for(size_t i = 0; i < NumberOfChemicalElements; i++) {
		QMap<ChemicalElement::CrystalStructure,QPair<QString,double> >::iterator e = HandledTypes.find(ChemicalElements[i].structure);
		
		if(e != HandledTypes.end()) {
			FloatType r = ChemicalElements[i].latticeParameter * e.value().second;
			comboBox()->addItem(e.value().first.arg(ChemicalElements[i].elementName).arg(r, 0, 'f', 2), r);
		}
	}
	comboBox()->model()->sort(0);
	comboBox()->insertItem(0, tr("Presets..."));
	comboBox()->setCurrentIndex(0);

	connect(comboBox(), (void (QComboBox::*)(int))&QComboBox::activated, this, &CutoffRadiusPresetsUI::onSelect);
}

/******************************************************************************
* Destructor.
******************************************************************************/
CutoffRadiusPresetsUI::~CutoffRadiusPresetsUI()
{
	// Release GUI controls.
	delete _comboBox;
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void CutoffRadiusPresetsUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(comboBox()) comboBox()->setEnabled(editObject() != NULL && isEnabled());
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to.
******************************************************************************/
void CutoffRadiusPresetsUI::resetUI()
{
	PropertyParameterUI::resetUI();

	if(comboBox())
		comboBox()->setEnabled(editObject() != NULL && isEnabled());
}

/******************************************************************************
* Is called when the user has selected an item in the preset box.
******************************************************************************/
void CutoffRadiusPresetsUI::onSelect(int index)
{
	FloatType r = comboBox()->itemData(index).value<FloatType>();
	if(r != 0) {
		if(editObject() && propertyField()) {
			undoableTransaction(tr("Change cutoff radius"), [this, r]() {
				editObject()->setPropertyFieldValue(*propertyField(), r);
			});
			Q_EMIT valueEntered();
		}
	}
	comboBox()->setCurrentIndex(0);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
