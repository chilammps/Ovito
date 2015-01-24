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

#ifndef __OVITO_CA_BURGERS_VECTOR_FAMILY_H
#define __OVITO_CA_BURGERS_VECTOR_FAMILY_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/reference/RefTarget.h>
#include <core/gui/properties/PropertiesEditor.h>

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/**
 * \brief Stores properties of an atom type.
 */
class OVITO_CRYSTALANALYSIS_EXPORT BurgersVectorFamily : public RefTarget
{
public:

	/// \brief Constructs a new BurgersVectorFamily.
	Q_INVOKABLE BurgersVectorFamily(DataSet* dataset);

	/// Returns the family's display name.
	const QString& name() const { return _name; }

	/// Sets the family's display name.
	void setName(const QString& name) { _name = name; }

	/// Returns the color that is used to display dislocation of this family.
	const Color& color() const { return _color; }

	/// Sets the color that is used to display dislocation of this family.
	void setColor(const Color& color) { _color = color; }

	/// Returns the Burgers vector of this family.
	const Vector3& burgersVector() const { return _burgersVector; }

	/// Sets the Burgers vector of this family.
	void setBurgersVector(const Vector3& v) { _burgersVector = v; }

	/// Returns whether dislocation segments that belong to this family are shown.
	bool isVisible() const { return _isVisible; }

	/// Sets whether dislocation segments that belong to this family are shown.
	void setVisible(bool visible) { _isVisible = visible; }

	/// Checks if the given Burgers vector is a member of this family.
	bool isMember(const Vector3& v) const {
		// Take absolute value of components and sort them.
		Vector3 sc(std::fabs(v.x()), std::fabs(v.y()), std::fabs(v.z()));
		std::sort(sc.data(), sc.data() + 3);
		return burgersVector().equals(sc);
	}

	/// Returns the title of this object.
	virtual QString objectTitle() override { return name(); }

public:

	Q_PROPERTY(QString name READ name WRITE setName)
	Q_PROPERTY(Color color READ color WRITE setColor)
	Q_PROPERTY(bool isVisible READ isVisible WRITE setVisible)

protected:

	/// The name of this atom type.
	PropertyField<QString, QString, ReferenceEvent::TitleChanged> _name;

	/// This visualization color of this family.
	PropertyField<Color> _color;

	/// Controls whether dislocation segments that belong to this family are shown.
	PropertyField<bool> _isVisible;

	/// This Burgers vector of this family.
	PropertyField<Vector3> _burgersVector;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_name);
	DECLARE_PROPERTY_FIELD(_color);
	DECLARE_PROPERTY_FIELD(_isVisible);
	DECLARE_PROPERTY_FIELD(_burgersVector);
};

/**
 * \brief A properties editor for the BurgersVectorFamily class.
 */
class OVITO_CRYSTALANALYSIS_EXPORT BurgersVectorFamilyEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE BurgersVectorFamilyEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

}	// End of namespace
}	// End of namespace
}	// End of namespace

#endif // __OVITO_CA_BURGERS_VECTOR_FAMILY_H
