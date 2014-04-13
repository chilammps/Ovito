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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include "../ParticleModifier.h"

namespace Particles {

/**
 * \brief Computes the values of a particle property from a user-defined math expression.
 */
class OVITO_PARTICLES_EXPORT CreateExpressionPropertyModifier : public ParticleModifier
{
public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE CreateExpressionPropertyModifier(DataSet* dataset) : ParticleModifier(dataset),
		_outputProperty(tr("Custom property")), _expressions(QStringList("0")),
		_onlySelectedParticles(false)
	{
		INIT_PROPERTY_FIELD(CreateExpressionPropertyModifier::_expressions);
		INIT_PROPERTY_FIELD(CreateExpressionPropertyModifier::_outputProperty);
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

	/// \brief Sets the output particle property that receives the computed per-particle values.
	void setOutputProperty(const ParticlePropertyReference& prop) { _outputProperty = prop; }

	/// \brief Returns the output particle property that receives the computed per-particle values.
	const ParticlePropertyReference& outputProperty() const { return _outputProperty; }

	/// \brief Returns the number of vector components of the property to create.
	/// \return The number of vector components.
	/// \sa setPropertyComponentCount()
	int propertyComponentCount() const { return expressions().size(); }

	/// \brief Sets the number of vector components of the property to create.
	/// \param newComponentCount The number of vector components.
	/// \undoable
	void setPropertyComponentCount(int newComponentCount);

	/// \brief Returns whether the math expression is only evaluated for selected particles.
	/// \return \c true if the expression is only evaluated for selected particles; \c false if it is calculated for all particles.
	bool onlySelectedParticles() const { return _onlySelectedParticles; }

	/// \brief Sets whether the math expression is only evaluated for selected particles.
	/// \param enable Specifies the restriction to selected particles.
	/// \undoable
	void setOnlySelectedParticles(bool enable) { _onlySelectedParticles = enable; }

	/// \brief Returns the list of available input variables.
	const QStringList& inputVariableNames() const { return _inputVariableNames; }

	/// \brief Returns a human-readable text listing the input variables.
	const QString& inputVariableTable() const { return _inputVariableTable; }

public:

	Q_PROPERTY(QStringList expressions READ expressions WRITE setExpressions);
	Q_PROPERTY(Particles::ParticlePropertyReference outputProperty READ outputProperty WRITE setOutputProperty);
	Q_PROPERTY(int propertyComponentCount READ propertyComponentCount);
	Q_PROPERTY(bool onlySelectedParticles READ onlySelectedParticles WRITE setOnlySelectedParticles);

protected:

	/// \brief Allows the object to parse the serialized contents of a property field in a custom way.
	virtual bool loadPropertyFieldFromStream(ObjectLoadStream& stream, const ObjectLoadStream::SerializedPropertyField& serializedField) override;

	/// \brief Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Modifies the particle object.
	virtual PipelineStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// The math expressions for calculating the property values. One for every vector component.
	PropertyField<QStringList> _expressions;

	/// Specifies the output property that will receive the computed per-particles values.
	PropertyField<ParticlePropertyReference> _outputProperty;

	/// Controls whether the math expression is evaluated and output only for selected particles.
	PropertyField<bool> _onlySelectedParticles;

	/// The list of input variables during the last evaluation.
	QStringList _inputVariableNames;

	/// Human-readable text listing the input variables during the last evaluation.
	QString _inputVariableTable;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Compute property");
	Q_CLASSINFO("ModifierCategory", "Modification");

	DECLARE_PROPERTY_FIELD(_expressions);
	DECLARE_PROPERTY_FIELD(_outputProperty);
	DECLARE_PROPERTY_FIELD(_onlySelectedParticles);
};

/**
 * \brief A properties editor for the CreateExpressionPropertyModifier class.
 */
class CreateExpressionPropertyModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE CreateExpressionPropertyModifierEditor() {}

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
