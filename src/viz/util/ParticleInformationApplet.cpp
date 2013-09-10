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
#include <core/gui/widgets/RolloutContainer.h>
#include <core/gui/actions/ViewportModeAction.h>
#include <core/animation/AnimManager.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <viz/data/ParticlePropertyObject.h>
#include <viz/data/ParticleTypeProperty.h>
#include <viz/data/ParticleDisplay.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>
#include "ParticleInformationApplet.h"

namespace Viz {

IMPLEMENT_OVITO_OBJECT(Viz, ParticleInformationApplet, UtilityApplet);

/******************************************************************************
* Initializes the utility applet.
******************************************************************************/
ParticleInformationApplet::ParticleInformationApplet() : _panel(nullptr)
{
	connect(&AnimManager::instance(), SIGNAL(timeChanged(TimePoint)), this, SLOT(updateInformationDisplay()));
}

/******************************************************************************
* Shows the UI of the utility in the given RolloutContainer.
******************************************************************************/
void ParticleInformationApplet::openUtility(RolloutContainer* container, const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	_panel = new QWidget();
	container->addRollout(_panel, tr("Particle information"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(_panel);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QPushButton* pickModeButton = new QPushButton(tr("Picking mode"));
	pickModeButton->setCheckable(true);
	layout->addWidget(pickModeButton);

	_captionLabel = new QLabel(tr("Double-click on a particle in the viewports."), _panel);
	layout->addWidget(_captionLabel);
	_captionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

	class MyTableWidget : public QTableWidget {
	public:
		MyTableWidget(QWidget* parent) : QTableWidget(parent) {}
		virtual QSize sizeHint() const override { return QTableWidget::sizeHint().expandedTo(QSize(0, 420)); }
	};

	_table = new MyTableWidget(_panel);
	_table->setEnabled(false);
	_table->verticalHeader()->setVisible(false);
	_table->setCornerButtonEnabled(false);
	_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	layout->addWidget(_table);

	_inputMode = new ParticleInformationInputMode(this);
	ViewportModeAction* pickModeAction = new ViewportModeAction(tr("Pick mode"), this, _inputMode);
	connect(pickModeAction, SIGNAL(toggled(bool)), pickModeButton, SLOT(setChecked(bool)));
	connect(pickModeButton, SIGNAL(clicked(bool)), pickModeAction, SLOT(trigger()));
	ViewportInputManager::instance().pushInputHandler(_inputMode);
}

/******************************************************************************
* Removes the UI of the utility from the rollout container.
******************************************************************************/
void ParticleInformationApplet::closeUtility(RolloutContainer* container)
{
	ViewportInputManager::instance().removeInputHandler(_inputMode.get());
	_inputMode = nullptr;
	delete _panel;
	_panel = nullptr;
}

/******************************************************************************
* Updates the display of atom properties.
******************************************************************************/
void ParticleInformationApplet::updateInformationDisplay()
{
	if(_inputMode->_selectedNode) {
		const PipelineFlowState& flowState = _inputMode->_selectedNode->evalPipeline(AnimManager::instance().time());

		size_t particleIndex = _inputMode->_particleIndex;
		if(_inputMode->_particleId >= 0) {
			for(const auto& sceneObj : flowState.objects()) {
				ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(sceneObj.get());
				if(property && property->type() == ParticleProperty::IdentifierProperty) {
					const int* begin = property->constDataInt();
					const int* end = begin + property->size();
					const int* iter = std::find(begin, end, _inputMode->_particleId);
					if(iter != end)
						particleIndex = (iter - begin);
				}
			}
		}

		_captionLabel->setText(tr("Particle %1:").arg(particleIndex + 1));

		QVector< QPair<QTableWidgetItem*, QTableWidgetItem*> > tableItems;
		for(const auto& sceneObj : flowState.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(sceneObj.get());
			if(!property || property->size() <= particleIndex) continue;
			if(property->dataType() != qMetaTypeId<int>() && property->dataType() != qMetaTypeId<FloatType>()) continue;
			for(size_t component = 0; component < property->componentCount(); component++) {
				QString propertyName = property->name();
				if(property->componentNames().empty() == false) {
					propertyName.append(".");
					propertyName.append(property->componentNames()[component]);
				}
				QString valueString;
				if(property->dataType() == qMetaTypeId<int>()) {
					valueString = QString::number(property->getIntComponent(particleIndex, component));
					ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(property);
					if(typeProperty && typeProperty->particleTypes().empty() == false) {
						ParticleType* ptype = typeProperty->particleType(property->getIntComponent(particleIndex, component));
						if(ptype) {
							valueString.append(" (" + ptype->name() + ")");
						}
					}
				}
				else if(property->dataType() == qMetaTypeId<FloatType>())
					valueString = QString::number(property->getFloatComponent(particleIndex, component));

				tableItems.push_back(qMakePair(new QTableWidgetItem(propertyName), new QTableWidgetItem(valueString)));
			}
		}
		_table->setEnabled(true);
		_table->setColumnCount(2);
		_table->setRowCount(tableItems.size());
		QStringList headerItems;
		headerItems << tr("Property");
		headerItems << tr("Value");
		_table->setHorizontalHeaderLabels(headerItems);
		_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
		_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
		for(int i = 0; i < tableItems.size(); i++) {
			_table->setItem(i, 0, tableItems[i].first);
			_table->setItem(i, 1, tableItems[i].second);
		}
	}
	else {
		_captionLabel->setText(tr("You did not click on a particle."));
		_table->setEnabled(false);
		_table->setColumnCount(0);
		_table->setRowCount(0);
	}
}

/******************************************************************************
* Handles the mouse up events for a Viewport.
******************************************************************************/
void ParticleInformationInputMode::mouseReleaseEvent(Viewport* vp, QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton && temporaryNavigationMode() == nullptr) {

		_selectedNode = nullptr;
		ViewportPickResult pickResult = vp->pick(event->pos());
		if(pickResult.valid) {
			// Check if user has really clicked on a particle.
			OORef<ParticlePropertyObject> posProperty = dynamic_object_cast<ParticlePropertyObject>(pickResult.sceneObject);
			if(posProperty && posProperty->type() == ParticleProperty::PositionProperty) {

				// Save reference to the selected particle.
				_selectedNode = pickResult.objectNode.get();
				_particleIndex = pickResult.subobjectId;

				// Also save particle ID in case ordering of particles changes.
				_particleId = -1;
				const PipelineFlowState& flowState = pickResult.objectNode->evalPipeline(AnimManager::instance().time());
				for(const auto& sceneObj : flowState.objects()) {
					ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(sceneObj.get());
					if(property && property->type() == ParticleProperty::IdentifierProperty && _particleIndex < property->size()) {
						_particleId = property->getInt(_particleIndex);
					}
				}
			}
		}
		_applet->updateInformationDisplay();
		ViewportManager::instance().updateViewports();
	}
	ViewportInputHandler::mouseReleaseEvent(vp, event);
}

/******************************************************************************
* Lets the input mode render its overlay content in a viewport.
******************************************************************************/
void ParticleInformationInputMode::renderOverlay(Viewport* vp, ViewportSceneRenderer* renderer, bool isActive)
{
	ViewportInputHandler::renderOverlay(vp, renderer, isActive);

	if(!renderer->isInteractive() || renderer->isPicking())
		return;

	if(!_selectedNode)
		return;

	const PipelineFlowState& flowState = _selectedNode->evalPipeline(AnimManager::instance().time());

	// If particle selection is based on ID, find particle with the given ID.
	size_t particleIndex = _particleIndex;
	if(_particleId >= 0) {
		for(const auto& sceneObj : flowState.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(sceneObj.get());
			if(property && property->type() == ParticleProperty::IdentifierProperty) {
				const int* begin = property->constDataInt();
				const int* end = begin + property->size();
				const int* iter = std::find(begin, end, _particleId);
				if(iter != end)
					particleIndex = (iter - begin);
			}
		}
	}

	// Fetch properties of selected particle needed to render the overlay.
	ParticlePropertyObject* posProperty = nullptr;
	ParticlePropertyObject* radiusProperty = nullptr;
	ParticlePropertyObject* typeProperty = nullptr;
	for(const auto& sceneObj : flowState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(sceneObj.get());
		if(!property) continue;
		if(property->type() == ParticleProperty::PositionProperty && property->size() >= particleIndex)
			posProperty = property;
		else if(property->type() == ParticleProperty::RadiusProperty && property->size() >= particleIndex)
			radiusProperty = property;
		if(property->type() == ParticleProperty::ParticleTypeProperty && property->size() >= particleIndex)
			typeProperty = property;
	}
	if(!posProperty)
		return;
	ParticleDisplay* particleDisplay = dynamic_object_cast<ParticleDisplay>(posProperty->displayObject());
	if(!particleDisplay)
		return;

	// Determine position of selected particle.
	Point3 pos = posProperty->getPoint3(particleIndex);

	// Determine radius of selected particle.
	FloatType radius = particleDisplay->particleRadius(particleIndex, radiusProperty, dynamic_object_cast<ParticleTypeProperty>(typeProperty));
	if(radius <= 0)
		return;

	TimeInterval iv;
	const AffineTransformation& nodeTM = _selectedNode->getWorldTransform(AnimManager::instance().time(), iv);

	// Prepare marker geometry buffer.
	if(!_markerBuffer2 || !_markerBuffer2->isValid(renderer)
			|| !_markerBuffer2->setShadingMode(particleDisplay->shadingMode())
			|| !_markerBuffer2->setRenderingQuality(particleDisplay->renderingQuality())) {
		Color markerColor(1.0, 0.0, 0.0);
		_markerBuffer2 = renderer->createParticleGeometryBuffer(particleDisplay->shadingMode(), particleDisplay->renderingQuality());
		_markerBuffer2->setSize(1);
		_markerBuffer2->setParticleColor(markerColor);
	}
	_markerBuffer2->setParticlePositions(&pos);
	_markerBuffer2->setParticleRadius(radius);

	renderer->setWorldTransform(nodeTM);
	int oldDepthFunc;
	glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);
	glDepthFunc(GL_LEQUAL);
	_markerBuffer2->render(renderer);
	glDepthFunc(oldDepthFunc);

	// Prepare marker geometry buffer.
	if(!_markerBuffer || !_markerBuffer->isValid(renderer)) {
		ColorA markerColor(1.0, 1.0, 1.0);
		_markerBuffer = renderer->createLineGeometryBuffer();
		Point3 vertices[64];
		for(int i = 0; i < sizeof(vertices)/sizeof(vertices[0])/2; i++) {
			FloatType angle = (FloatType)i * 2.0 * FLOATTYPE_PI / (sizeof(vertices)/sizeof(vertices[0])/2);
			vertices[i*2] = Point3(0, cos(angle), sin(angle));
		}
		for(int i = 0; i < sizeof(vertices)/sizeof(vertices[0])/2; i++) {
			vertices[i*2+1] = vertices[(i*2+2)%(sizeof(vertices)/sizeof(vertices[0]))];
		}
		_markerBuffer->setSize(sizeof(vertices)/sizeof(vertices[0]));
		_markerBuffer->setVertexPositions(vertices);
		_markerBuffer->setVertexColor(markerColor);
	}
	AffineTransformation particleTM = nodeTM * AffineTransformation::translation(pos - Point3::Origin()) * AffineTransformation::scaling(radius);
	glDisable(GL_DEPTH_TEST);
	for(int i = 0; i < 6; i++) {
		renderer->setWorldTransform(particleTM * AffineTransformation::rotationZ(FLOATTYPE_PI/6 * i));
		_markerBuffer->render(renderer);
	}
#if 0
	ViewProjectionParameters vparams = renderer->projParams();
	AffineTransformation oldViewMatrix = vparams.viewMatrix;
	vparams.viewMatrix = nodeTM * AffineTransformation::translation(oldViewMatrix * pos - Point3::Origin()) * AffineTransformation::scaling(radius);
	vparams.inverseViewMatrix = vparams.viewMatrix.inverse();
	renderer->setProjParams(vparams);
	renderer->setWorldTransform(AffineTransformation::Identity());
	_markerBuffer->render(renderer);
	vparams.viewMatrix = oldViewMatrix;
	vparams.inverseViewMatrix = oldViewMatrix.inverse();
	renderer->setProjParams(vparams);
#endif
	glEnable(GL_DEPTH_TEST);
}

};
