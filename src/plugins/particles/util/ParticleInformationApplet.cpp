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
#include <core/gui/actions/ViewportModeAction.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/animation/AnimationSettings.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include <plugins/particles/objects/ParticleTypeProperty.h>
#include "ParticleInformationApplet.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_OVITO_OBJECT(Particles, ParticleInformationApplet, UtilityApplet);

/******************************************************************************
* Shows the UI of the utility in the given RolloutContainer.
******************************************************************************/
void ParticleInformationApplet::openUtility(MainWindow* mainWindow, RolloutContainer* container, const RolloutInsertionParameters& rolloutParams)
{
	OVITO_ASSERT(_panel == nullptr);
	_mainWindow = mainWindow;

	// Create a rollout.
	_panel = new QWidget();
	container->addRollout(_panel, tr("Particle information"), rolloutParams.useAvailableSpace());

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(_panel);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	_inputMode = new ParticleInformationInputMode(this);
	ViewportModeAction* pickModeAction = new ViewportModeAction(_mainWindow, tr("Selection mode"), this, _inputMode);
	layout->addWidget(pickModeAction->createPushButton());

	_infoDisplay = new QTextEdit(_panel);
	_infoDisplay->setReadOnly(true);
	_infoDisplay->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
#ifndef Q_OS_MACX
	_infoDisplay->setText(tr("Pick a particle in the viewports. Hold down the CONTROL key to select multiple particles."));
#else
	_infoDisplay->setText(tr("Pick a particle in the viewports. Hold down the COMMAND key to select multiple particles."));
#endif
	layout->addWidget(_infoDisplay, 1);

	connect(&_mainWindow->datasetContainer(), &DataSetContainer::animationSettingsReplaced, this, &ParticleInformationApplet::onAnimationSettingsReplaced);
	_timeChangeCompleteConnection = connect(_mainWindow->datasetContainer().currentSet()->animationSettings(), &AnimationSettings::timeChangeComplete, this, &ParticleInformationApplet::updateInformationDisplay);
	_mainWindow->viewportInputManager()->pushInputMode(_inputMode);
}

/******************************************************************************
* Removes the UI of the utility from the rollout container.
******************************************************************************/
void ParticleInformationApplet::closeUtility(RolloutContainer* container)
{
	delete _panel;
}

/******************************************************************************
* This is called when new animation settings have been loaded.
******************************************************************************/
void ParticleInformationApplet::onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings)
{
	disconnect(_timeChangeCompleteConnection);
	if(newAnimationSettings) {
		_timeChangeCompleteConnection = connect(newAnimationSettings, &AnimationSettings::timeChangeComplete, this, &ParticleInformationApplet::updateInformationDisplay);
	}
	updateInformationDisplay();
}

/******************************************************************************
* Updates the display of atom properties.
******************************************************************************/
void ParticleInformationApplet::updateInformationDisplay()
{
	DataSet* dataset = _mainWindow->datasetContainer().currentSet();
	if(!dataset) return;

	QString infoText;
	QTextStream stream(&infoText, QIODevice::WriteOnly);
	for(auto& pickedParticle : _inputMode->_pickedParticles) {
		OVITO_ASSERT(pickedParticle.objNode);
		const PipelineFlowState& flowState = pickedParticle.objNode->evalPipeline(dataset->animationSettings()->time());

		// If selection is based on particle ID, update the stored particle index in case order has changed.
		if(pickedParticle.particleId >= 0) {
			for(DataObject* dataObj : flowState.objects()) {
				ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(dataObj);
				if(property && property->type() == ParticleProperty::IdentifierProperty) {
					const int* begin = property->constDataInt();
					const int* end = begin + property->size();
					const int* iter = std::find(begin, end, pickedParticle.particleId);
					if(iter != end)
						pickedParticle.particleIndex = (iter - begin);
				}
			}
		}

		stream << QStringLiteral("<b>") << tr("Particle index") << QStringLiteral(" ") << (pickedParticle.particleIndex + 1) << QStringLiteral(":</b>");
		stream << QStringLiteral("<table border=\"0\">");

		for(DataObject* dataObj : flowState.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(dataObj);
			if(!property || property->size() <= pickedParticle.particleIndex) continue;

			// Update saved particle position in case it has changed.
			if(property->type() == ParticleProperty::PositionProperty)
				pickedParticle.localPos = property->getPoint3(pickedParticle.particleIndex);

			if(property->dataType() != qMetaTypeId<int>() && property->dataType() != qMetaTypeId<FloatType>()) continue;
			for(size_t component = 0; component < property->componentCount(); component++) {
				QString propertyName = property->name();
				if(property->componentNames().empty() == false) {
					propertyName.append(".");
					propertyName.append(property->componentNames()[component]);
				}
				QString valueString;
				if(property->dataType() == qMetaTypeId<int>()) {
					valueString = QString::number(property->getIntComponent(pickedParticle.particleIndex, component));
					ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(property);
					if(typeProperty && typeProperty->particleTypes().empty() == false) {
						ParticleType* ptype = typeProperty->particleType(property->getIntComponent(pickedParticle.particleIndex, component));
						if(ptype) {
							valueString.append(" (" + ptype->name() + ")");
						}
					}
				}
				else if(property->dataType() == qMetaTypeId<FloatType>())
					valueString = QString::number(property->getFloatComponent(pickedParticle.particleIndex, component));

				stream << QStringLiteral("<tr><td>") << propertyName << QStringLiteral(":</td><td>") << valueString << QStringLiteral("</td></tr>");
			}
		}
		stream << QStringLiteral("</table><hr>");
	}
	if(_inputMode->_pickedParticles.empty())
		infoText = tr("No particles selected.");
	else if(_inputMode->_pickedParticles.size() >= 2) {
		stream << QStringLiteral("<b>") << tr("Distances:") << QStringLiteral("</b>");
		stream << QStringLiteral("<table border=\"0\">");
		for(size_t i = 0; i < _inputMode->_pickedParticles.size(); i++) {
			const auto& p1 = _inputMode->_pickedParticles[i];
			for(size_t j = i + 1; j < _inputMode->_pickedParticles.size(); j++) {
				const auto& p2 = _inputMode->_pickedParticles[j];
				stream << QStringLiteral("<tr><td>(") <<
						(p1.particleIndex+1) << QStringLiteral(",") << (p2.particleIndex+1) <<
						QStringLiteral("):</td><td>") << (p1.localPos - p2.localPos).length() << QStringLiteral("</td></tr>");
			}
		}
		stream << QStringLiteral("</table><hr>");
	}
	if(_inputMode->_pickedParticles.size() >= 3) {
		stream << QStringLiteral("<b>") << tr("Angles:") << QStringLiteral("</b>");
		stream << QStringLiteral("<table border=\"0\">");
		for(size_t i = 0; i < _inputMode->_pickedParticles.size(); i++) {
			const auto& p1 = _inputMode->_pickedParticles[i];
			for(size_t j = 0; j < _inputMode->_pickedParticles.size(); j++) {
				if(j == i) continue;
				const auto& p2 = _inputMode->_pickedParticles[j];
				for(size_t k = j + 1; k < _inputMode->_pickedParticles.size(); k++) {
					if(k == i) continue;
					const auto& p3 = _inputMode->_pickedParticles[k];
					Vector3 v1 = p2.localPos - p1.localPos;
					Vector3 v2 = p3.localPos - p1.localPos;
					v1.normalizeSafely();
					v2.normalizeSafely();
					FloatType angle = acos(v1.dot(v2));
					stream << QStringLiteral("<tr><td>(") <<
							(p2.particleIndex+1) << QStringLiteral(" - ") << (p1.particleIndex+1) << QStringLiteral(" - ") << (p3.particleIndex+1) <<
							QStringLiteral("):</td><td>") << (angle * 180.0f / FLOATTYPE_PI) << QStringLiteral("</td></tr>");
				}
			}
		}
		stream << QStringLiteral("</table><hr>");
	}
	_infoDisplay->setText(infoText);
}

/******************************************************************************
* Handles the mouse up events for a Viewport.
******************************************************************************/
void ParticleInformationInputMode::mouseReleaseEvent(Viewport* vp, QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton) {
		PickResult pickResult;
		pickParticle(vp, event->pos(), pickResult);
		if(!event->modifiers().testFlag(Qt::ControlModifier))
			_pickedParticles.clear();
		if(pickResult.objNode) {
			// Don't select the same particle twice.
			bool alreadySelected = false;
			for(auto p = _pickedParticles.begin(); p != _pickedParticles.end(); ++p) {
				if(p->objNode == pickResult.objNode && p->particleIndex == pickResult.particleIndex) {
					alreadySelected = true;
					_pickedParticles.erase(p);
					break;
				}
			}
			if(!alreadySelected)
				_pickedParticles.push_back(pickResult);
		}
		_applet->updateInformationDisplay();
		vp->dataset()->viewportConfig()->updateViewports();
	}
	ViewportInputMode::mouseReleaseEvent(vp, event);
}

/******************************************************************************
* Lets the input mode render its overlay content in a viewport.
******************************************************************************/
void ParticleInformationInputMode::renderOverlay3D(Viewport* vp, ViewportSceneRenderer* renderer)
{
	ViewportInputMode::renderOverlay3D(vp, renderer);
	for(const auto& pickedParticle : _pickedParticles)
		renderSelectionMarker(vp, renderer, pickedParticle);
}

/******************************************************************************
* Computes the bounding box of the 3d visual viewport overlay rendered by the input mode.
******************************************************************************/
Box3 ParticleInformationInputMode::overlayBoundingBox(Viewport* vp, ViewportSceneRenderer* renderer)
{
	Box3 bbox = ViewportInputMode::overlayBoundingBox(vp, renderer);
	for(const auto& pickedParticle : _pickedParticles)
		bbox.addBox(selectionMarkerBoundingBox(vp, pickedParticle));
	return bbox;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
