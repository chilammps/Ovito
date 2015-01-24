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

#ifndef __OVITO_CREATE_BONDS_MODIFIER_H
#define __OVITO_CREATE_BONDS_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/objects/BondsDisplay.h>
#include <plugins/particles/modifier/AsynchronousParticleModifier.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Modify)

/**
 * \brief A modifier that creates bonds between pairs of particles based on their distance.
 */
class OVITO_PARTICLES_EXPORT CreateBondsModifier : public AsynchronousParticleModifier
{
public:

	enum CutoffMode {
		UniformCutoff,		///< A single cutoff radius for all particles.
		PairCutoff,			///< Individual cutoff radius for each pair of particle types.
	};
	Q_ENUMS(CutoffMode);

	/// The container type used to store the pair-wise cutoffs.
	typedef QMap<QPair<QString,QString>, FloatType> PairCutoffsList;

private:

	/// Engine that determines the bonds between particles.
	class BondsEngine : public ComputeEngine
	{
	public:

		/// Constructor.
		BondsEngine(const TimeInterval& validityInterval, ParticleProperty* positions, ParticleProperty* particleTypes, const SimulationCell& simCell, CutoffMode cutoffMode,
				FloatType uniformCutoff, std::vector<std::vector<FloatType>>&& pairCutoffs) :
					ComputeEngine(validityInterval),
					_positions(positions), _particleTypes(particleTypes), _simCell(simCell), _cutoffMode(cutoffMode),
					_uniformCutoff(uniformCutoff), _pairCutoffs(std::move(pairCutoffs)), _bonds(new BondsStorage()) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void perform() override;

		/// Returns the generated bonds.
		BondsStorage* bonds() { return _bonds.data(); }

		/// Returns the input particle positions.
		ParticleProperty* positions() const { return _positions.data(); }

	private:

		CutoffMode _cutoffMode;
		FloatType _uniformCutoff;
		std::vector<std::vector<FloatType>> _pairCutoffs;
		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<ParticleProperty> _particleTypes;
		QExplicitlySharedDataPointer<BondsStorage> _bonds;
		SimulationCell _simCell;
	};

public:

	/// Constructor.
	Q_INVOKABLE CreateBondsModifier(DataSet* dataset);

	/// Returns the mode of choosing the cutoff radius.
	CutoffMode cutoffMode() const { return _cutoffMode; }

	/// Sets the mode of choosing the cutoff radius.
	void setCutoffMode(CutoffMode mode) { _cutoffMode = mode; }

	/// \brief Returns the uniform cutoff radius used to determine which particles are bonded.
	/// \return The uniform cutoff radius in world units.
	FloatType uniformCutoff() const { return _uniformCutoff; }

	/// \brief Sets the cutoff radius that is used for generating bonds.
	/// \param newCutoff The new cutoff radius in world units.
	void setUniformCutoff(FloatType newCutoff) { _uniformCutoff = newCutoff; }

	/// Returns the cutoff radii for pairs of particle types.
	const PairCutoffsList& pairCutoffs() const { return _pairCutoffs; }

	/// Sets the cutoff radii for pairs of particle types.
	void setPairCutoffs(const PairCutoffsList& pairCutoffs);

	/// \brief Returns the display object that is responsible for rendering the bonds.
	BondsDisplay* bondsDisplay() const { return _bondsDisplay; }

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

	/// Handles reference events sent by reference targets of this object.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Resets the modifier's result cache.
	virtual void invalidateCachedResults() override;

	/// Creates a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<ComputeEngine> createEngine(TimePoint time, TimeInterval validityInterval) override;

	/// Unpacks the results of the computation engine and stores them in the modifier.
	virtual void transferComputationResults(ComputeEngine* engine) override;

	/// Lets the modifier insert the cached computation results into the modification pipeline.
	virtual PipelineStatus applyComputationResults(TimePoint time, TimeInterval& validityInterval) override;

private:

	/// The mode of choosing the cutoff radius.
	PropertyField<CutoffMode, int> _cutoffMode;

	/// The cutoff radius for bond generation.
	PropertyField<FloatType> _uniformCutoff;

	/// The cutoff radii for pairs of particle types.
	PairCutoffsList _pairCutoffs;

	/// The display object for rendering the bonds.
	ReferenceField<BondsDisplay> _bondsDisplay;

	/// This stores the cached results of the modifier, i.e. the bonds information.
	QExplicitlySharedDataPointer<BondsStorage> _bonds;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Create bonds");
	Q_CLASSINFO("ModifierCategory", "Modification");

	DECLARE_PROPERTY_FIELD(_cutoffMode);
	DECLARE_PROPERTY_FIELD(_uniformCutoff);
	DECLARE_REFERENCE_FIELD(_bondsDisplay);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A properties editor for the CreateBondsModifier class.
 */
class CreateBondsModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE CreateBondsModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

protected Q_SLOTS:

	/// Updates the contents of the pair-wise cutoff table.
	void updatePairCutoffList();

	/// Updates the cutoff values in the pair-wise cutoff table.
	void updatePairCutoffListValues();

private:

	class PairCutoffTableModel : public QAbstractTableModel {
	public:
		typedef QVector<QPair<QString,QString>> ContentType;

		PairCutoffTableModel(QObject* parent) : QAbstractTableModel(parent) {}
		virtual int	rowCount(const QModelIndex& parent) const override { return _data.size(); }
		virtual int	columnCount(const QModelIndex& parent) const override { return 3; }
		virtual QVariant data(const QModelIndex& index, int role) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
			if(orientation != Qt::Horizontal || role != Qt::DisplayRole) return QVariant();
			switch(section) {
			case 0: return CreateBondsModifierEditor::tr("1st type");
			case 1: return CreateBondsModifierEditor::tr("2nd type");
			case 2: return CreateBondsModifierEditor::tr("Cutoff");
			default: return QVariant();
			}
		}
		virtual Qt::ItemFlags flags(const QModelIndex& index) const override {
			if(index.column() != 2)
				return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			else
				return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
		}
		virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
		void setContent(CreateBondsModifier* modifier, const ContentType& data) {
			beginResetModel();
			_modifier = modifier;
			_data = data;
			endResetModel();
		}
		void updateContent() { Q_EMIT dataChanged(index(0,2), index(_data.size()-1,2)); }
	private:
		ContentType _data;
		OORef<CreateBondsModifier> _modifier;
	};

	QTableView* _pairCutoffTable;
	PairCutoffTableModel* _pairCutoffTableModel;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Particles::CreateBondsModifier::CutoffMode);
Q_DECLARE_TYPEINFO(Ovito::Particles::CreateBondsModifier::CutoffMode, Q_PRIMITIVE_TYPE);

#endif // __OVITO_CREATE_BONDS_MODIFIER_H
