///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2008) Alexander Stukowski
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
#include <core/rendering/FrameBufferWindow.h>
#include <core/gui/dialogs/SaveImageFileDialog.h>

namespace Core {

/******************************************************************************
* Constructor.
******************************************************************************/
FrameBufferWindow::FrameBufferWindow(QWidget* parent) : QMainWindow(parent, (Qt::WindowFlags)(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint))
{
	//setAttribute(Qt::WA_DeleteOnClose);

	frameBufferWidget = new FrameBufferWidget(this);
	setCentralWidget(frameBufferWidget);

	QToolBar* toolBar = addToolBar(tr("Frame Buffer"));
	toolBar->addAction(QIcon(":/core/rendering/save_picture.png"), tr("Save to file"), this, SLOT(saveImage()));
	toolBar->addAction(QIcon(":/core/rendering/copy_picture_to_clipboard.png"), tr("Copy to clipboard"), this, SLOT(copyImageToClipboard()));
}

/******************************************************************************
* This opens the file dialog and lets the suer save the current contents of the frame buffer
* to an image file.
******************************************************************************/
void FrameBufferWindow::saveImage()
{
	if(frameBuffer() == NULL) return;

	SaveImageFileDialog fileDialog(this, tr("Save image"));
	if(fileDialog.exec()) {
		QString imageFilename = fileDialog.imageInfo().filename();
		if(!frameBuffer()->image().save(imageFilename, fileDialog.imageInfo().format())) {
			Exception ex(tr("Failed to save rendered image to image file '%1'.").arg(imageFilename));
			ex.showError();
		}
	}
}

/******************************************************************************
* This copies the current image to the clipboard.
******************************************************************************/
void FrameBufferWindow::copyImageToClipboard()
{
	if(frameBuffer() == NULL) return;

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setImage(frameBuffer()->image());
}

};
