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
#include <core/rendering/viewport/ViewportSceneRenderer.h>
#include <viz/data/ParticlePropertyObject.h>
#include <viz/data/ParticleTypeProperty.h>
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

	_inputMode = new ParticleInformationInputMode(this);
	ViewportModeAction* pickModeAction = new ViewportModeAction(tr("Pick mode"), this, _inputMode);
	layout->addWidget(pickModeAction->createPushButton());

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
	if(_inputMode->_pickedParticle.objNode) {
		const PipelineFlowState& flowState = _inputMode->_pickedParticle.objNode->evalPipeline(AnimManager::instance().time());

		size_t particleIndex = _inputMode->_pickedParticle.particleIndex;
		if(_inputMode->_pickedParticle.particleId >= 0) {
			for(const auto& sceneObj : flowState.objects()) {
				ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(sceneObj.get());
				if(property && property->type() == ParticleProperty::IdentifierProperty) {
					const int* begin = property->constDataInt();
					const int* end = begin + property->size();
					const int* iter = std::find(begin, end, _inputMode->_pickedParticle.particleId);
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
		pickParticle(vp, event->pos(), _pickedParticle);
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
	renderSelectionMarker(vp, renderer, _pickedParticle);
}

};
