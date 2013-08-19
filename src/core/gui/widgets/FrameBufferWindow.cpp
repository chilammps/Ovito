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
#include <core/gui/dialogs/SaveImageFileDialog.h>
#include "FrameBufferWindow.h"

namespace Ovito {

/******************************************************************************
* Constructor.
******************************************************************************/
FrameBufferWindow::FrameBufferWindow(QWidget* parent) : QMainWindow(parent, (Qt::WindowFlags)(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint))
{
	frameBufferWidget = new FrameBufferWidget();

	class MyScrollArea : public QScrollArea {
	public:
		MyScrollArea(QWidget* parent) : QScrollArea(parent) {}
		virtual QSize sizeHint() const override {
			int f = 2 * frameWidth();
			QSize sz(f, f);
			if(widget())
				sz += widget()->sizeHint();
			return sz;
		}
	};

	QScrollArea* scrollArea = new MyScrollArea(this);
	scrollArea->setWidget(frameBufferWidget);
	setCentralWidget(scrollArea);

	QToolBar* toolBar = addToolBar(tr("Frame Buffer"));
	toolBar->addAction(QIcon(":/core/framebuffer/save_picture.png"), tr("Save to file"), this, SLOT(saveImage()));
	toolBar->addAction(QIcon(":/core/framebuffer/copy_picture_to_clipboard.png"), tr("Copy to clipboard"), this, SLOT(copyImageToClipboard()));
}

/******************************************************************************
* This opens the file dialog and lets the suer save the current contents of the frame buffer
* to an image file.
******************************************************************************/
void FrameBufferWindow::saveImage()
{
	if(frameBuffer() == nullptr) return;

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
	if(frameBuffer() == nullptr) return;

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setImage(frameBuffer()->image());
}

};
