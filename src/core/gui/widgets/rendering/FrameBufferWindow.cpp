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

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/******************************************************************************
* Constructor.
******************************************************************************/
FrameBufferWindow::FrameBufferWindow(QWidget* parent) :
#if !defined(Q_OS_MAC) || QT_VERSION >= QT_VERSION_CHECK(5, 3, 1)
	QMainWindow(parent, (Qt::WindowFlags)(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint))
#else
	QMainWindow(parent, (Qt::WindowFlags)(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint))
#endif
{
#if defined(Q_OS_MAC) && QT_VERSION < QT_VERSION_CHECK(5, 3, 1)
	setWindowModality(Qt::ApplicationModal);
#endif
	_frameBufferWidget = new FrameBufferWidget();

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
	scrollArea->setWidget(_frameBufferWidget);
	setCentralWidget(scrollArea);

	QToolBar* toolBar = addToolBar(tr("Frame Buffer"));
	toolBar->addAction(QIcon(":/core/framebuffer/save_picture.png"), tr("Save to file"), this, SLOT(saveImage()));
	toolBar->addAction(QIcon(":/core/framebuffer/copy_picture_to_clipboard.png"), tr("Copy to clipboard"), this, SLOT(copyImageToClipboard()));
	toolBar->addAction(QIcon(":/core/framebuffer/auto_crop.png"), tr("Auto-crop image"), this, SLOT(autoCrop()));

	// Disable context menu in toolbar.
	setContextMenuPolicy(Qt::NoContextMenu);
}

/******************************************************************************
* This opens the file dialog and lets the suer save the current contents of the frame buffer
* to an image file.
******************************************************************************/
void FrameBufferWindow::saveImage()
{
	if(!frameBuffer())
		return;

	SaveImageFileDialog fileDialog(this, tr("Save image"));
	if(fileDialog.exec()) {
		QString imageFilename = fileDialog.imageInfo().filename();
		if(!frameBuffer()->image().save(imageFilename, fileDialog.imageInfo().format())) {
			Exception ex(tr("Failed to save image to file '%1'.").arg(imageFilename));
			ex.showError();
		}
	}
}

/******************************************************************************
* This copies the current image to the clipboard.
******************************************************************************/
void FrameBufferWindow::copyImageToClipboard()
{
	if(!frameBuffer())
		return;

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setImage(frameBuffer()->image());
}

/******************************************************************************
* Removes unnecessary pixels at the outer edges of the rendered image.
******************************************************************************/
void FrameBufferWindow::autoCrop()
{
	if(!frameBuffer())
		return;

	QImage image = frameBuffer()->image().convertToFormat(QImage::Format_ARGB32);
	auto determineCropRect = [&image](QRgb backgroundColor) -> QRect {
		int x1 = 0, y1 = 0;
		int x2 = image.width() - 1, y2 = image.height() - 1;
		bool significant;
		for(;; x1++) {
			significant = false;
			for(int y = y1; y <= y2; y++) {
				if(reinterpret_cast<const QRgb*>(image.constScanLine(y))[x1] != backgroundColor) {
					significant = true;
					break;
				}
			}
			if(significant || x1 > x2)
				break;
		}
		for(; x2 >= x1; x2--) {
			significant = false;
			for(int y = y1; y <= y2; y++) {
				if(reinterpret_cast<const QRgb*>(image.constScanLine(y))[x2] != backgroundColor) {
					significant = true;
					break;
				}
			}
			if(significant || x1 > x2)
				break;
		}
		for(;; y1++) {
			significant = false;
			const QRgb* s = reinterpret_cast<const QRgb*>(image.constScanLine(y1));
			for(int x = x1; x <= x2; x++) {
				if(s[x] != backgroundColor) {
					significant = true;
					break;
				}
			}
			if(significant || y1 > y2)
				break;
		}
		for(; y2 >= y1; y2--) {
			significant = false;
			const QRgb* s = reinterpret_cast<const QRgb*>(image.constScanLine(y2));
			for(int x = x1; x <= x2; x++) {
				if(s[x] != backgroundColor) {
					significant = true;
					break;
				}
			}
			if(significant || y1 > y2)
				break;
		}
		return QRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	};

	// Use the pixel colors in the four corners of the images as candidate background colors.
	// Compute the crop rect for each candidate color and use the one that leads
	// to the smallest crop rect.
	QRect cropRect = determineCropRect(image.pixel(0,0));
	QRect r;
	r = determineCropRect(image.pixel(image.width()-1,0));
	if(r.width()*r.height() < cropRect.width()*cropRect.height()) cropRect = r;
	r = determineCropRect(image.pixel(image.width()-1,image.height()-1));
	if(r.width()*r.height() < cropRect.width()*cropRect.height()) cropRect = r;
	r = determineCropRect(image.pixel(0,image.height()-1));
	if(r.width()*r.height() < cropRect.width()*cropRect.height()) cropRect = r;

	if(cropRect != image.rect() && cropRect.width() > 0 && cropRect.height() > 0) {
		frameBuffer()->image() = frameBuffer()->image().copy(cropRect);
		frameBuffer()->update();
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
