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

#ifndef __OVITO_ANIM_SETTINGS_DIALOG_H
#define __OVITO_ANIM_SETTINGS_DIALOG_H

#include <core/animation/AnimationSettings.h>
#include <core/dataset/UndoStack.h>
#include <core/gui/widgets/general/SpinnerWidget.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * This dialog box lets the user manage the animation settings.
 */
class OVITO_CORE_EXPORT AnimationSettingsDialog : public QDialog, private UndoableTransaction
{
	Q_OBJECT
	
public:

	/// Constructor.
	AnimationSettingsDialog(AnimationSettings* animSettings, QWidget* parentWindow = nullptr);
	
	/// Destructor.
	virtual ~AnimationSettingsDialog() {}
	
private Q_SLOTS:	

	/// Event handler for the Ok button.
	void onOk();

	/// Is called when the user has selected a new value for the frames per seconds.
	void onFramesPerSecondChanged(int index);
	
	/// Is called when the user has selected a new value for the playback speed.
	void onPlaybackSpeedChanged(int index);
	
	/// Is called when the user changes the start/end values of the animation interval.
	void onAnimationIntervalChanged();

private:
	
	/// Updates the values shown in the dialog.
	void updateValues();

	/// The animation settings being edited.
	OORef<AnimationSettings> _animSettings;

	QComboBox* fpsBox;
	SpinnerWidget* animStartSpinner;
	SpinnerWidget* animEndSpinner;	
	QComboBox* playbackSpeedBox;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_ANIM_SETTINGS_DIALOG_H
