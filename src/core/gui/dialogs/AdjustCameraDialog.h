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

#ifndef __OVITO_ADJUST_CAMERA_DIALOG_H
#define __OVITO_ADJUST_CAMERA_DIALOG_H

#include <core/Core.h>
#include <core/viewport/Viewport.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * This dialog box lets the user adjust the camera settings of the
 * current viewport.
 */
class AdjustCameraDialog : public QDialog
{
	Q_OBJECT
	
public:

	/// Constructor.
	AdjustCameraDialog(Viewport* viewport, QWidget* parentWindow = nullptr);
	
private Q_SLOTS:	

	/// Event handler for the Cancel button.
	void onCancel();

	/// Is called when the user has changed the camera settings.
	void onAdjustCamera();

	/// Updates the values displayed in the dialog.
	void updateGUI();

private:

	QCheckBox* _camPerspective;

	SpinnerWidget* _camPosXSpinner;
	SpinnerWidget* _camPosYSpinner;
	SpinnerWidget* _camPosZSpinner;

	SpinnerWidget* _camDirXSpinner;
	SpinnerWidget* _camDirYSpinner;
	SpinnerWidget* _camDirZSpinner;

	QLabel* _camFOVLabel;
	SpinnerWidget* _camFOVSpinner;

	Viewport* _viewport;
	Viewport::ViewType _oldViewType;
	AffineTransformation _oldCameraTM;
	FloatType _oldFOV;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_ADJUST_CAMERA_DIALOG_H
