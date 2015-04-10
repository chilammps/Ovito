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
#include <core/utilities/units/UnitsManager.h>
#include <core/gui/widgets/general/SpinnerWidget.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/scene/SelectionSet.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/SceneRoot.h>
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include <plugins/particles/objects/TrajectoryGeneratorObject.h>
#include "CreateTrajectoryApplet.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_OVITO_OBJECT(Particles, CreateTrajectoryApplet, UtilityApplet);

/******************************************************************************
* Shows the UI of the utility in the given RolloutContainer.
******************************************************************************/
void CreateTrajectoryApplet::openUtility(MainWindow* mainWindow, RolloutContainer* container, const RolloutInsertionParameters& rolloutParams)
{
	_mainWindow = mainWindow;
	DataSet* dataset = mainWindow->datasetContainer().currentSet();

	// Create a rollout.
	_panel = new QWidget();
	container->addRollout(_panel, tr("Create trajectory lines"), rolloutParams, "howto.visualize_particle_trajectories.html");

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(_panel);
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

		layout2->addWidget(new QLabel(tr("Generate trajectories for:")), 0, 0, 1, 2);

		_selectedParticlesButton = new QRadioButton(tr("Selected particles"));
		_selectedParticlesButton->setChecked(true);
		layout2->addWidget(_selectedParticlesButton, 1, 1);

		_allParticlesButton = new QRadioButton(tr("All particles"));
		layout2->addWidget(_allParticlesButton, 2, 1);
	}

	// Periodic boundaries
	{
		QGroupBox* groupBox = new QGroupBox(tr("Peripodic boundary conditions"));
		layout->addWidget(groupBox);

		QGridLayout* layout2 = new QGridLayout(groupBox);
		layout2->setContentsMargins(4,4,4,4);
		layout2->setSpacing(2);

		_unwrapTrajectoryButton = new QCheckBox(tr("Unwrap trajectory"));
		_unwrapTrajectoryButton->setChecked(true);
		layout2->addWidget(_unwrapTrajectoryButton, 0, 0);
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

		_animationIntervalButton = new QRadioButton(tr("Complete trajectory"));
		_animationIntervalButton->setChecked(true);
		layout2c->addWidget(_animationIntervalButton, 0, 0, 1, 5);

		_customIntervalButton = new QRadioButton(tr("Frame interval:"));
		layout2c->addWidget(_customIntervalButton, 1, 0, 1, 5);

		QLineEdit* customRangeStartEdit = new QLineEdit();
		_customRangeStartSpinner = new SpinnerWidget(nullptr, customRangeStartEdit);
		_customRangeStartSpinner->setUnit(dataset->unitsManager().timeUnit());
		_customRangeStartSpinner->setEnabled(false);
		_customRangeStartSpinner->setIntValue(dataset->animationSettings()->animationInterval().start());
		QHBoxLayout* fieldlayout = new QHBoxLayout();
		fieldlayout->setContentsMargins(0,0,0,0);
		fieldlayout->setSpacing(0);
		fieldlayout->addWidget(customRangeStartEdit);
		fieldlayout->addWidget(_customRangeStartSpinner);
		layout2c->addLayout(fieldlayout, 2, 1);
		layout2c->addWidget(new QLabel(tr("to")), 2, 2);
		QLineEdit* customRangeEndEdit = new QLineEdit();
		_customRangeEndSpinner = new SpinnerWidget(nullptr, customRangeEndEdit);
		_customRangeEndSpinner->setUnit(dataset->unitsManager().timeUnit());
		_customRangeEndSpinner->setEnabled(false);
		_customRangeEndSpinner->setIntValue(dataset->animationSettings()->animationInterval().end());
		fieldlayout = new QHBoxLayout();
		fieldlayout->setContentsMargins(0,0,0,0);
		fieldlayout->setSpacing(0);
		fieldlayout->addWidget(customRangeEndEdit);
		fieldlayout->addWidget(_customRangeEndSpinner);
		layout2c->addLayout(fieldlayout, 2, 3);
		layout2c->setColumnMinimumWidth(0, 30);
		layout2c->setColumnStretch(4, 1);
		connect(_customIntervalButton, &QRadioButton::toggled, _customRangeStartSpinner, &SpinnerWidget::setEnabled);
		connect(_customIntervalButton, &QRadioButton::toggled, _customRangeEndSpinner, &SpinnerWidget::setEnabled);

		QGridLayout* layout2a = new QGridLayout();
		layout2a->setContentsMargins(0,6,0,0);
		layout2a->setSpacing(2);
		layout2->addLayout(layout2a);

		QLineEdit* everyNthFrameEdit = new QLineEdit();
		_everyNthFrameSpinner = new SpinnerWidget(nullptr, everyNthFrameEdit);
		_everyNthFrameSpinner->setUnit(dataset->unitsManager().integerIdentityUnit());
		_everyNthFrameSpinner->setIntValue(1);
		_everyNthFrameSpinner->setMinValue(1);
		fieldlayout = new QHBoxLayout();
		fieldlayout->setContentsMargins(0,0,0,0);
		fieldlayout->setSpacing(0);
		fieldlayout->addWidget(everyNthFrameEdit);
		fieldlayout->addWidget(_everyNthFrameSpinner);
		layout2a->addWidget(new QLabel(tr("Every Nth frame:")), 0, 0);
		layout2a->addLayout(fieldlayout, 0, 1);
	}

	QPushButton* createTrajectoryButton = new QPushButton(tr("Create trajectory lines"));
	layout->addWidget(createTrajectoryButton);
	connect(createTrajectoryButton, &QPushButton::clicked, this, &CreateTrajectoryApplet::onCreateTrajectory);
}

/******************************************************************************
* Removes the UI of the utility from the rollout container.
******************************************************************************/
void CreateTrajectoryApplet::closeUtility(RolloutContainer* container)
{
	delete _panel;
}

/******************************************************************************
* Is called when the user clicks the 'Create trajectory' button.
******************************************************************************/
void CreateTrajectoryApplet::onCreateTrajectory()
{
	DataSet* dataset = _mainWindow->datasetContainer().currentSet();
	if(!dataset) return;

	try {
		UndoableTransaction transaction(dataset->undoStack(), tr("Create trajectory lines"));
		AnimationSuspender noAnim(dataset->animationSettings());
		TimePoint time = dataset->animationSettings()->time();

		// Get input particles.
		ParticlePropertyObject* posProperty = nullptr;
		ParticlePropertyObject* selectionProperty = nullptr;
		ObjectNode* inputNode = dynamic_object_cast<ObjectNode>(dataset->selection()->front());
		if(inputNode) {
			if(!inputNode->waitUntilReady(time, tr("Waiting for input particles to become ready.")))
				return;
			const PipelineFlowState& state = inputNode->evalPipeline(time);
			posProperty = ParticlePropertyObject::findInState(state, ParticleProperty::PositionProperty);
			selectionProperty = ParticlePropertyObject::findInState(state, ParticleProperty::SelectionProperty);
		}

		if(!posProperty)
			throw Exception(tr("Currently no particle data object is selected from which trajectory lines can be generated."));

		// Determine number of input particles.
		size_t particleCount = 0;
		if(_selectedParticlesButton->isChecked()) {
			if(selectionProperty)
				particleCount = std::count_if(selectionProperty->constDataInt(), selectionProperty->constDataInt() + selectionProperty->size(), [](int s) { return s != 0; });
			if(!particleCount)
				throw Exception(tr("Currently no particles are selected. No trajectory lines were created."));
		}
		else {
			particleCount = posProperty->size();
			if(!particleCount)
				throw Exception(tr("Input contains no particles. No trajectory lines were created."));
		}

		OORef<ObjectNode> node;
		{
			// Do not create undo records for the following actions.
			UndoSuspender noUndo(dataset);

			// Create trajectory object.
			OORef<TrajectoryGeneratorObject> trajObj = new TrajectoryGeneratorObject(dataset);
			OVITO_CHECK_OBJECT_POINTER(inputNode);
			trajObj->setSource(inputNode);
			trajObj->setOnlySelectedParticles(_selectedParticlesButton->isChecked());
			trajObj->setUseCustomInterval(_customIntervalButton->isChecked());
			trajObj->setCustomIntervalStart(_customRangeStartSpinner->intValue());
			trajObj->setCustomIntervalEnd(_customRangeEndSpinner->intValue());
			trajObj->setEveryNthFrame(_everyNthFrameSpinner->intValue());
			trajObj->setUnwrapTrajectories(_unwrapTrajectoryButton->isChecked());

			// Make sure we are having an actual trajectory.
			TimeInterval interval = trajObj->useCustomInterval() ?
					trajObj->customInterval() : dataset->animationSettings()->animationInterval();
			if(interval.duration() <= 0)
				throw Exception(tr("Current sequence consists only of a single frame. No trajectory lines were created."));

			// Generate trajectories.
			if(!trajObj->generateTrajectories())
				return;

			// Create scene node.
			node = new ObjectNode(dataset);
			TimeInterval validityInterval;
			node->transformationController()->setTransformationValue(time, inputNode->getWorldTransform(time, validityInterval), true);
			node->setDataProvider(trajObj);
		}
		// Insert node into scene.
		dataset->sceneRoot()->addChild(node);

		// Select new scene node.
		dataset->selection()->setNode(node);

		// Commit actions.
		transaction.commit();

		// Switch to the modify tab to show the newly created trajectory object.
		_mainWindow->setCurrentCommandPanelPage(MainWindow::MODIFY_PAGE);
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
