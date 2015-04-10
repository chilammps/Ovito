///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2015) Alexander Stukowski
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
#include <core/animation/AnimationSettings.h>
#include <core/scene/ObjectNode.h>
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/StringParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/IntegerRadioButtonParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <core/gui/widgets/general/ElidedTextLabel.h>
#include <core/viewport/ViewportConfiguration.h>
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include <plugins/particles/objects/SimulationCellObject.h>
#include "TrajectoryGeneratorObject.h"

namespace Ovito { namespace Particles {

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, TrajectoryGeneratorObjectEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, TrajectoryGeneratorObject, TrajectoryObject);
SET_OVITO_OBJECT_EDITOR(TrajectoryGeneratorObject, TrajectoryGeneratorObjectEditor);
DEFINE_FLAGS_REFERENCE_FIELD(TrajectoryGeneratorObject, _source, "ParticleSource", ObjectNode, PROPERTY_FIELD_NEVER_CLONE_TARGET | PROPERTY_FIELD_NO_SUB_ANIM);
DEFINE_PROPERTY_FIELD(TrajectoryGeneratorObject, _onlySelectedParticles, "OnlySelectedParticles");
DEFINE_PROPERTY_FIELD(TrajectoryGeneratorObject, _useCustomInterval, "UseCustomInterval");
DEFINE_PROPERTY_FIELD(TrajectoryGeneratorObject, _customIntervalStart, "CustomIntervalStart");
DEFINE_PROPERTY_FIELD(TrajectoryGeneratorObject, _customIntervalEnd, "CustomIntervalEnd");
DEFINE_PROPERTY_FIELD(TrajectoryGeneratorObject, _everyNthFrame, "EveryNthFrame");
DEFINE_PROPERTY_FIELD(TrajectoryGeneratorObject, _unwrapTrajectories, "UnwrapTrajectories");
SET_PROPERTY_FIELD_LABEL(TrajectoryGeneratorObject, _source, "Source");
SET_PROPERTY_FIELD_LABEL(TrajectoryGeneratorObject, _onlySelectedParticles, "Only selected particles");
SET_PROPERTY_FIELD_LABEL(TrajectoryGeneratorObject, _useCustomInterval, "Custom time interval");
SET_PROPERTY_FIELD_LABEL(TrajectoryGeneratorObject, _customIntervalStart, "Custom interval start");
SET_PROPERTY_FIELD_LABEL(TrajectoryGeneratorObject, _customIntervalEnd, "Custom interval end");
SET_PROPERTY_FIELD_LABEL(TrajectoryGeneratorObject, _everyNthFrame, "Every Nth frame");
SET_PROPERTY_FIELD_LABEL(TrajectoryGeneratorObject, _unwrapTrajectories, "Unwrap trajectories");
SET_PROPERTY_FIELD_UNITS(TrajectoryGeneratorObject, _customIntervalStart, TimeParameterUnit);
SET_PROPERTY_FIELD_UNITS(TrajectoryGeneratorObject, _customIntervalEnd, TimeParameterUnit);

/******************************************************************************
* Default constructor.
******************************************************************************/
TrajectoryGeneratorObject::TrajectoryGeneratorObject(DataSet* dataset) : TrajectoryObject(dataset),
		_onlySelectedParticles(true), _useCustomInterval(false),
		_customIntervalStart(dataset->animationSettings()->animationInterval().start()),
		_customIntervalEnd(dataset->animationSettings()->animationInterval().end()),
		_everyNthFrame(1), _unwrapTrajectories(true)
{
	INIT_PROPERTY_FIELD(TrajectoryGeneratorObject::_source);
	INIT_PROPERTY_FIELD(TrajectoryGeneratorObject::_onlySelectedParticles);
	INIT_PROPERTY_FIELD(TrajectoryGeneratorObject::_useCustomInterval);
	INIT_PROPERTY_FIELD(TrajectoryGeneratorObject::_customIntervalStart);
	INIT_PROPERTY_FIELD(TrajectoryGeneratorObject::_customIntervalEnd);
	INIT_PROPERTY_FIELD(TrajectoryGeneratorObject::_everyNthFrame);
	INIT_PROPERTY_FIELD(TrajectoryGeneratorObject::_unwrapTrajectories);
}

/******************************************************************************
* Updates the stored trajectories from the source particle object.
******************************************************************************/
bool TrajectoryGeneratorObject::generateTrajectories(QProgressDialog* progressDialog)
{
	// Suspend viewports while loading simulation frames.
	ViewportSuspender noVPUpdates(this);

	// Show progress dialog.
	std::unique_ptr<QProgressDialog> localProgressDialog;
	if(!progressDialog && Application::instance().guiMode()) {
		localProgressDialog.reset(new QProgressDialog(dataset()->mainWindow()));
		localProgressDialog->setWindowModality(Qt::WindowModal);
		localProgressDialog->setAutoClose(false);
		localProgressDialog->setAutoReset(false);
		localProgressDialog->setMinimumDuration(0);
		localProgressDialog->setValue(0);
		progressDialog = localProgressDialog.get();
	}

	TimePoint currentTime = dataset()->animationSettings()->time();

	// Get input particles.
	if(!source())
		throw Exception(tr("No input particle data object is selected from which trajectory lines can be generated."));

	if(!source()->waitUntilReady(currentTime, tr("Waiting for input particles to become ready."), progressDialog))
		return false;

	const PipelineFlowState& state = source()->evalPipeline(currentTime);
	ParticlePropertyObject* posProperty = ParticlePropertyObject::findInState(state, ParticleProperty::PositionProperty);
	ParticlePropertyObject* selectionProperty = ParticlePropertyObject::findInState(state, ParticleProperty::SelectionProperty);
	ParticlePropertyObject* identifierProperty = ParticlePropertyObject::findInState(state, ParticleProperty::IdentifierProperty);

	if(!posProperty)
		throw Exception(tr("The input object contains no particles."));

	// Determine set of input particles.
	std::vector<int> selectedIndices;
	std::set<int> selectedIdentifiers;
	int particleCount = 0;
	if(onlySelectedParticles()) {
		if(selectionProperty) {
			if(identifierProperty && identifierProperty->size() == selectionProperty->size()) {
				const int* s = selectionProperty->constDataInt();
				for(int id : identifierProperty->constIntRange())
					if(*s++) selectedIdentifiers.insert(id);
				particleCount = selectedIdentifiers.size();
			}
			else {
				const int* s = selectionProperty->constDataInt();
				for(int index = 0; index < selectionProperty->size(); index++)
					if(*s++) selectedIndices.push_back(index);
				particleCount = selectedIndices.size();
			}
		}
	}
	else {
		if(identifierProperty) {
			for(int id : identifierProperty->constIntRange())
				selectedIdentifiers.insert(id);
			particleCount = selectedIdentifiers.size();
		}
		else {
			selectedIndices.resize(posProperty->size());
			std::iota(selectedIndices.begin(), selectedIndices.end(), 0);
			particleCount = selectedIndices.size();
		}
	}

	// Iterate over the simulation frames.
	TimeInterval interval = useCustomInterval() ? customInterval() : dataset()->animationSettings()->animationInterval();
	QVector<TimePoint> sampleTimes;
	for(TimePoint time = interval.start(); time <= interval.end(); time += everyNthFrame() * dataset()->animationSettings()->ticksPerFrame()) {
		sampleTimes.push_back(time);
	}
	if(progressDialog) {
		progressDialog->setMaximum(sampleTimes.size());
		progressDialog->setValue(0);
	}

	QVector<Point3> points;
	points.reserve(particleCount * sampleTimes.size());
	for(TimePoint time : sampleTimes) {
		if(!source()->waitUntilReady(time, tr("Waiting for input particles to become ready."), progressDialog))
			return false;
		const PipelineFlowState& state = source()->evalPipeline(time);
		ParticlePropertyObject* posProperty = ParticlePropertyObject::findInState(state, ParticleProperty::PositionProperty);
		if(!posProperty)
			throw Exception(tr("Input particle set is empty at frame %1.").arg(dataset()->animationSettings()->timeToFrame(time)));

		if(!selectedIdentifiers.empty()) {
			ParticlePropertyObject* identifierProperty = ParticlePropertyObject::findInState(state, ParticleProperty::IdentifierProperty);
			if(!identifierProperty || identifierProperty->size() != posProperty->size())
				throw Exception(tr("Input particles do not possess identifiers at frame %1.").arg(dataset()->animationSettings()->timeToFrame(time)));

			// Create a mapping from IDs to indices.
			std::map<int,int> idmap;
			int index = 0;
			for(int id : identifierProperty->constIntRange())
				idmap.insert(std::make_pair(id, index++));

			for(int id : selectedIdentifiers) {
				auto entry = idmap.find(id);
				if(entry == idmap.end())
					throw Exception(tr("Input particle with ID=%1 does not exist at frame %2.").arg(id).arg(dataset()->animationSettings()->timeToFrame(time)));
				points.push_back(posProperty->getPoint3(entry->second));
			}
		}
		else {
			for(int index : selectedIndices) {
				if(index >= posProperty->size())
					throw Exception(tr("Input particle at index %1 does not exist at frame %2.").arg(index+1).arg(dataset()->animationSettings()->timeToFrame(time)));
				points.push_back(posProperty->getPoint3(index));
			}
		}

		// Unwrap trajectory points at periodic boundaries of the simulation cell.
		if(unwrapTrajectories() && points.size() > particleCount) {
			if(SimulationCellObject* simCellObj = state.findObject<SimulationCellObject>()) {
				SimulationCell cell = simCellObj->data();
				if(cell.pbcFlags() != std::array<bool,3>{false, false, false}) {
					auto previousPos = points.cbegin() + (points.size() - 2 * particleCount);
					auto currentPos = points.begin() + (points.size() - particleCount);
					for(int i = 0; i < particleCount; i++, ++previousPos, ++currentPos) {
						Vector3 delta = cell.wrapVector(*currentPos - *previousPos);
						*currentPos = *previousPos + delta;
					}
					OVITO_ASSERT(currentPos == points.end());
				}
			}
		}

		if(progressDialog) {
			progressDialog->setValue(progressDialog->value() + 1);
			if(progressDialog->wasCanceled()) return false;
		}
	}

	setTrajectories(particleCount, points, sampleTimes);

	// Jump back to current animation time.
	if(!source()->waitUntilReady(currentTime, tr("Waiting for input particles to become ready."), progressDialog))
		return false;

	return true;
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void TrajectoryGeneratorObjectEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Generate trajectory"), rolloutParams, "howto.visualize_particle_trajectories.html");

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(6);

	// Particle set
	{
		QGroupBox* groupBox = new QGroupBox(tr("Input particles"));
		layout->addWidget(groupBox);

		QGridLayout* layout2 = new QGridLayout(groupBox);
		layout2->setContentsMargins(4,4,4,4);
		layout2->setSpacing(2);
		layout2->setColumnStretch(1, 1);
		layout2->setColumnMinimumWidth(0, 15);

		layout2->addWidget(new QLabel(tr("Source:")), 0, 0, 1, 2);
		QLabel* dataSourceLabel = new ElidedTextLabel();
		layout2->addWidget(dataSourceLabel, 1, 1);

	    connect(this, &PropertiesEditor::contentsChanged, [dataSourceLabel](RefTarget* editObject) {
	    	if(TrajectoryGeneratorObject* trajObj = static_object_cast<TrajectoryGeneratorObject>(editObject)) {
	    		if(trajObj->source()) {
	    			dataSourceLabel->setText(trajObj->source()->objectTitle());
	    			return;
	    		}
	    	}
			dataSourceLabel->setText(QString());
	    });

		layout2->addWidget(new QLabel(tr("Generate trajectories for:")), 2, 0, 1, 2);

		BooleanRadioButtonParameterUI* onlySelectedParticlesUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(TrajectoryGeneratorObject::_onlySelectedParticles));

		QRadioButton* allParticlesButton = onlySelectedParticlesUI->buttonFalse();
		allParticlesButton->setText(tr("All particles"));
		layout2->addWidget(allParticlesButton, 3, 1);

		QRadioButton* selectedParticlesButton = onlySelectedParticlesUI->buttonTrue();
		selectedParticlesButton->setText(tr("Selected particles"));
		layout2->addWidget(selectedParticlesButton, 4, 1);
	}

	// Periodic boundaries
	{
		QGroupBox* groupBox = new QGroupBox(tr("Periodic boundary conditions"));
		layout->addWidget(groupBox);

		QGridLayout* layout2 = new QGridLayout(groupBox);
		layout2->setContentsMargins(4,4,4,4);
		layout2->setSpacing(2);

		BooleanParameterUI* unwrapTrajectoriesUI = new BooleanParameterUI(this, PROPERTY_FIELD(TrajectoryGeneratorObject::_unwrapTrajectories));
		layout2->addWidget(unwrapTrajectoriesUI->checkBox(), 0, 0);
	}

	// Time range
	{
		QGroupBox* groupBox = new QGroupBox(tr("Time range"));
		layout->addWidget(groupBox);

		QVBoxLayout* layout2 = new QVBoxLayout(groupBox);
		layout2->setContentsMargins(4,4,4,4);
		layout2->setSpacing(2);
		QGridLayout* layout2c = new QGridLayout();
		layout2c->setContentsMargins(0,0,0,0);
		layout2c->setSpacing(2);
		layout2->addLayout(layout2c);

		BooleanRadioButtonParameterUI* useCustomIntervalUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(TrajectoryGeneratorObject::_useCustomInterval));

		QRadioButton* animationIntervalButton = useCustomIntervalUI->buttonFalse();
		animationIntervalButton->setText(tr("Complete trajectory"));
		layout2c->addWidget(animationIntervalButton, 0, 0, 1, 5);

		QRadioButton* customIntervalButton = useCustomIntervalUI->buttonTrue();
		customIntervalButton->setText(tr("Frame interval:"));
		layout2c->addWidget(customIntervalButton, 1, 0, 1, 5);

		IntegerParameterUI* customRangeStartUI = new IntegerParameterUI(this, PROPERTY_FIELD(TrajectoryGeneratorObject::_customIntervalStart));
		customRangeStartUI->setEnabled(false);
		layout2c->addLayout(customRangeStartUI->createFieldLayout(), 2, 1);
		layout2c->addWidget(new QLabel(tr("to")), 2, 2);
		IntegerParameterUI* customRangeEndUI = new IntegerParameterUI(this, PROPERTY_FIELD(TrajectoryGeneratorObject::_customIntervalEnd));
		customRangeEndUI->setEnabled(false);
		layout2c->addLayout(customRangeEndUI->createFieldLayout(), 2, 3);
		layout2c->setColumnMinimumWidth(0, 30);
		layout2c->setColumnStretch(4, 1);
		connect(customIntervalButton, &QRadioButton::toggled, customRangeStartUI, &IntegerParameterUI::setEnabled);
		connect(customIntervalButton, &QRadioButton::toggled, customRangeEndUI, &IntegerParameterUI::setEnabled);

		QGridLayout* layout2a = new QGridLayout();
		layout2a->setContentsMargins(0,6,0,0);
		layout2a->setSpacing(2);
		layout2->addLayout(layout2a);
		IntegerParameterUI* everyNthFrameUI = new IntegerParameterUI(this, PROPERTY_FIELD(TrajectoryGeneratorObject::_everyNthFrame));
		layout2a->addWidget(everyNthFrameUI->label(), 0, 0);
		layout2a->addLayout(everyNthFrameUI->createFieldLayout(), 0, 1);
		everyNthFrameUI->setMinValue(1);
		layout2a->setColumnStretch(2, 1);
	}

	QPushButton* createTrajectoryButton = new QPushButton(tr("Regenerate trajectory lines"));
	layout->addWidget(createTrajectoryButton);
	connect(createTrajectoryButton, &QPushButton::clicked, this, &TrajectoryGeneratorObjectEditor::onRegenerateTrajectory);
}

/******************************************************************************
* Is called when the user clicks the 'Regenerate trajectory' button.
******************************************************************************/
void TrajectoryGeneratorObjectEditor::onRegenerateTrajectory()
{
	TrajectoryGeneratorObject* trajObj = static_object_cast<TrajectoryGeneratorObject>(editObject());
	if(!trajObj) return;

	undoableTransaction(tr("Generate trajectory"), [trajObj]() {
		trajObj->generateTrajectories();
	});
}

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace
