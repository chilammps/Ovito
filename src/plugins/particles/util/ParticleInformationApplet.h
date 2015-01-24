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

#include <plugins/particles/Particles.h>
#include <core/plugins/utility/UtilityApplet.h>
#include <core/viewport/input/ViewportInputMode.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <plugins/particles/util/ParticlePickingHelper.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

class ParticleInformationInputMode;		// defined below

/**
 * \brief This utility applet lets the user select a particle in the viewports
 *        and lists its properties.
 */
class OVITO_PARTICLES_EXPORT ParticleInformationApplet : public UtilityApplet
{
public:

	/// Constructor.
	Q_INVOKABLE ParticleInformationApplet() : UtilityApplet(), _panel(nullptr) {}

	/// Shows the UI of the utility in the given RolloutContainer.
	virtual void openUtility(MainWindow* mainWindow, RolloutContainer* container, const RolloutInsertionParameters& rolloutParams = RolloutInsertionParameters()) override;

	/// Removes the UI of the utility from the RolloutContainer.
	virtual void closeUtility(RolloutContainer* container) override;

public Q_SLOTS:

	/// This is called when new animation settings have been loaded.
	void onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings);

	/// Updates the display of particle properties.
	void updateInformationDisplay();

private:

	MainWindow* _mainWindow;
	QTextEdit* _infoDisplay;
	QWidget* _panel;

	/// The viewport input mode.
	ParticleInformationInputMode* _inputMode;

	QMetaObject::Connection _timeChangeCompleteConnection;

	Q_CLASSINFO("DisplayName", "Inspect particles");

	Q_OBJECT
	OVITO_OBJECT
};

class ParticleInformationInputMode : public ViewportInputMode, ParticlePickingHelper
{
public:

	/// Constructor.
	ParticleInformationInputMode(ParticleInformationApplet* applet) : ViewportInputMode(applet),
		_applet(applet) {}

	/// Returns the activation behavior of this input mode.
	virtual InputModeType modeType() override { return ExclusiveMode; }

	/// Handles the mouse up events for a Viewport.
	virtual void mouseReleaseEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Lets the input mode render its overlay content in a viewport.
	virtual void renderOverlay3D(Viewport* vp, ViewportSceneRenderer* renderer) override;

	/// \brief Indicates whether this input mode renders into the viewports.
	virtual bool hasOverlay() override { return true; }

	/// Computes the bounding box of the 3d visual viewport overlay rendered by the input mode.
	virtual Box3 overlayBoundingBox(Viewport* vp, ViewportSceneRenderer* renderer) override;

private:

	/// The particle information applet.
	ParticleInformationApplet* _applet;

	/// The selected particles whose properties are being displayed.
	std::deque<PickResult> _pickedParticles;

	friend class ParticleInformationApplet;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_PARTICLE_INFORMATION_APPLET_H
