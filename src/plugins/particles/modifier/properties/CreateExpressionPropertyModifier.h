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

#ifndef __OVITO_CREATE_EXPRESSION_PROPERTY_MODIFIER_H
#define __OVITO_CREATE_EXPRESSION_PROPERTY_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <core/gui/properties/StringParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include "../ParticleModifier.h"

namespace Particles {

/**
 * \brief Creates a particle property with values computed by n
 *        user-defined math expression.
 */
class OVITO_PARTICLES_EXPORT CreateExpressionPropertyModifier : public ParticleModifier
{
public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE CreateExpressionPropertyModifier() :
		_propertyType(ParticleProperty::UserProperty),
		_propertyDataType(qMetaTypeId<FloatType>()), _propertyName(tr("Custom property 1")), _expressions(QStringList("0")),
		_onlySelectedParticles(false)
	{
		INIT_PROPERTY_FIELD(CreateExpressionPropertyModifier::_expressions);
		INIT_PROPERTY_FIELD(CreateExpressionPropertyModifier::_propertyType);
		INIT_PROPERTY_FIELD(CreateExpressionPropertyModifier::_propertyName);
		INIT_PROPERTY_FIELD(CreateExpressionPropertyModifier::_propertyDataType);
		INIT_PROPERTY_FIELD(CreateExpressionPropertyModifier::_onlySelectedParticles);
	}

	//////////////////////////// from base classes ////////////////////////////

	/// \brief This virtual method is called by the system when the modifier has been inserted into a PipelineObject.
	virtual void initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp) override;

	/////////////////////////// specific methods ///////////////////////////////

	/// \brief Sets the math expressions that are used to calculate the values of the new property's components.
	/// \param expressions The mathematical formulas, one for each component of the property to create.
	/// \undoable
	/// \sa expressions()
	void setExpressions(const QStringList& expressions) { _expressions = expressions; }

	/// \brief Returns the math expressions that are used to calculate the values of the new property's component.
	/// \return The math formulas.
	/// \sa setExpressions()
	const QStringList& expressions() const { return _expressions; }

	/// \brief Sets the math expression that is used to calculate the values of one of the new property's components.
	/// \param index The property component for which the expression should be set.
	/// \param expression The math formula.
	/// \undoable
	/// \sa expression()
	void setExpression(const QString& expression, int index = 0) {
		if(index < 0 || index >= expressions().size())
			throw Exception("Property component index is out of range.");
		QStringList copy = _expressions;
		copy[index] = expression;
		_expressions = copy;
	}

	/// \brief Returns the math expression that is used to calculate the values of one of the new property's components.
	/// \param index The property component for which the expression should be returned.
	/// \return The math formula used to calculates the channel's values.
	/// \undoable
	/// \sa setExpression()
	const QString& expression(int index = 0) const {
		if(index < 0 || index >= expressions().size())
			throw Exception("Property component index is out of range.");
		return expressions()[index];
	}

	/// \brief Returns the type of the property being created by this modifier.
	/// \return The type.
	/// \sa setPropertyType()
	ParticleProperty::Type propertyType() const { return _propertyType; }

	/// \brief Sets the type of the property being created by this modifier.
	/// \param newType The new type. If this is one of the standard properties then the
	///                other parameters will be set to the defaults according to the standard property type.
	/// \undoable
	/// \sa propertyType()
	void setPropertyType(ParticleProperty::Type newType);

	/// \brief Returns the name of the property being created by this modifier.
	/// \return The name of the new property.
	/// \sa setPropertyName()
	const QString& propertyName() const { return _propertyName; }

	/// \brief Sets the name of the property being created by this modifier.
	/// \param newName The name of the new property.
	/// \undoable
	/// \sa propertyName()
	void setPropertyName(const QString& newName) { _propertyName = newName; }

	/// \brief Returns the data type of the property being created.
	/// \return The id of the Qt data type of the new property.
	/// \sa setPropertyDataType()
	int propertyDataType() const { return _propertyDataType; }

	/// \brief Sets the data type of the property being created.
	/// \param newDataType The id of the Qt data type of the new property.
	/// \undoable
	/// \sa propertyDataType()
	void setPropertyDataType(int newDataType) { _propertyDataType = newDataType; }

	/// \brief Returns the number of vector components of the property to create.
	/// \return The number of vector components.
	/// \sa setPropertyComponentCount()
	int propertyComponentCount() const { return expressions().size(); }

	/// \brief Sets the number of vector components of the property to create.
	/// \param newComponentCount The number of vector components.
	/// \undoable
	/// \sa propertyComponentCount()
	void setPropertyComponentCount(int newComponentCount);

	/// \brief Returns whether the math expression is only evaluated for selected particles.
	/// \return \c true if the expression is only evaluated for selected particles; \c false if it is calculated for all particles.
	/// \sa setOnlySelectedParticles()
	bool onlySelectedParticles() const { return _onlySelectedParticles; }

	/// \brief Sets whether the math expression is only evaluated for selected particles.
	/// \param enable Specifies the restriction to selected particles.
	/// \undoable
	/// \sa onlySelectedParticles()
	void setOnlySelectedParticles(bool enable) { _onlySelectedParticles = enable; }

	/// \brief Returns the list of variables during the last evaluation.
	const QStringList& lastVariableNames() const { return _variableNames; }

public:

	Q_PROPERTY(QStringList expressions READ expressions WRITE setExpressions)
	Q_PROPERTY(Particles::ParticleProperty::Type propertyType READ propertyType WRITE setPropertyType)
	Q_PROPERTY(QString propertyName READ propertyName WRITE setPropertyName)
	Q_PROPERTY(int propertyDataType READ propertyDataType WRITE setPropertyDataType)
	Q_PROPERTY(int propertyComponentCount READ propertyComponentCount WRITE setPropertyComponentCount)
	Q_PROPERTY(bool onlySelectedParticles READ onlySelectedParticles WRITE setOnlySelectedParticles)

protected:

	/// Modifies the particle object.
	virtual ObjectStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// Determines the available variable names.
	QStringList getVariableNames(const PipelineFlowState& inputState);

	/// The math expressions that are used to calculate the values of the property.
	PropertyField<QStringList> _expressions;

	/// The type of the property to create.
	PropertyField<ParticleProperty::Type, int> _propertyType;

	/// The name of the particle property to create.
	PropertyField<QString> _propertyName;

	/// The data type of the particle property to create.
	PropertyField<int> _propertyDataType;

	/// Controls whether the math expression is evaluated only for selected particles.
	PropertyField<bool> _onlySelectedParticles;

	/// The list of variables during the last evaluation.
	QStringList _variableNames;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Compute property");
	Q_CLASSINFO("ModifierCategory", "Modification");

	DECLARE_PROPERTY_FIELD(_expressions);
	DECLARE_PROPERTY_FIELD(_propertyType);
	DECLARE_PROPERTY_FIELD(_propertyName);
	DECLARE_PROPERTY_FIELD(_propertyDataType);
	DECLARE_PROPERTY_FIELD(_onlySelectedParticles);
};

/**
 * \brief A properties editor for the CreateExpressionPropertyModifier class.
 */
class CreateExpressionPropertyModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE CreateExpressionPropertyModifierEditor() {
		connect(this, &PropertiesEditor::contentsReplaced, this, &CreateExpressionPropertyModifierEditor::updateEditorFields);
	}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

protected Q_SLOTS:

	/// Is called when the user has typed in an expression.
	void onExpressionEditingFinished();

	/// Updates the enabled/disabled status of the editor's controls.
	void updateEditorFields();

private:

	StringParameterUI* propertyNameUI;
	VariantComboBoxParameterUI* propertyDataTypeUI;
	IntegerParameterUI* numComponentsUI;

	QWidget* rollout;
	QGroupBox* expressionsGroupBox;
	QList<QLineEdit*> expressionBoxes;
	QList<QLabel*> expressionBoxLabels;

	QVBoxLayout* expressionsLayout;

	QLabel* variableNamesList;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_CREATE_EXPRESSION_PROPERTY_MODIFIER_H
