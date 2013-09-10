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

#ifndef __OVITO_PARTICLE_INFORMATION_APPLET_H
#define __OVITO_PARTICLE_INFORMATION_APPLET_H

#include <core/Core.h>
#include <core/gui/mainwin/cmdpanel/UtilityApplet.h>
#include <core/viewport/input/ViewportInputHandler.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/rendering/LineGeometryBuffer.h>
#include <core/rendering/ParticleGeometryBuffer.h>

namespace Viz {

using namespace Ovito;

class ParticleInformationApplet;

class ParticleInformationInputMode : public ViewportInputHandler
{
public:

	/// Constructor.
	ParticleInformationInputMode(ParticleInformationApplet* applet) :
		_applet(applet) {}

	/// Returns the activation behavior of this input handler.
	virtual InputHandlerType handlerType() override { return ViewportInputHandler::NORMAL; }

	/// Handles the mouse up events for a Viewport.
	virtual void mouseReleaseEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Lets the input mode render its overlay content in a viewport.
	virtual void renderOverlay(Viewport* vp, ViewportSceneRenderer* renderer, bool isActive) override;

	/// \brief Indicates whether this input mode renders into the viewports.
	virtual bool hasOverlay() override { return true; }

private:

	/// The particle information applet.
	ParticleInformationApplet* _applet;

	/// The index of the selected particle whose properties are being displayed.
	size_t _particleIndex;

	/// The identifier of the selected particle whose properties are being displayed.
	int _particleId;

	/// The scene node to which the particle belongs.
	QPointer<ObjectNode> _selectedNode;

	/// Used to render the marker for the selected particle.
	OORef<LineGeometryBuffer> _markerBuffer;

	/// Used to render the marker for the selected particle.
	OORef<ParticleGeometryBuffer> _markerBuffer2;

	friend class ParticleInformationApplet;
};

/**
 * \brief This utility applet lets the user select a particle in the viewports
 *        and lists its properties.
 */
class ParticleInformationApplet : public UtilityApplet
{
public:

	/// Default constructor.
	Q_INVOKABLE ParticleInformationApplet();

	/// Shows the UI of the utility in the given RolloutContainer.
	virtual void openUtility(RolloutContainer* container, const RolloutInsertionParameters& rolloutParams = RolloutInsertionParameters()) override;

	/// Removes the UI of the utility from the RolloutContainer.
	virtual void closeUtility(RolloutContainer* container) override;

public Q_SLOTS:

	/// Updates the display of particle properties.
	void updateInformationDisplay();

private:

	QLabel* _captionLabel;
	QTableWidget* _table;
	QWidget* _panel;

	/// The viewport input mode.
	OORef<ParticleInformationInputMode> _inputMode;

	Q_CLASSINFO("DisplayName", "Inspect particle");

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_PARTICLE_INFORMATION_APPLET_H
