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

#ifndef __OVITO_SAVE_IMAGE_FILE_DIALOG_H
#define __OVITO_SAVE_IMAGE_FILE_DIALOG_H

#include <core/Core.h>
#include <core/rendering/FrameBuffer.h>
#include "HistoryFileDialog.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Dialogs)

/**
 * \brief This file chooser dialog lets the user select an image file for output.
 */
class OVITO_CORE_EXPORT SaveImageFileDialog : public HistoryFileDialog
{
	Q_OBJECT
	
public:

	/// \brief Constructs the dialog window.
	SaveImageFileDialog(QWidget* parent = nullptr, const QString& caption = QString(), bool includeVideoFormats = false, const ImageInfo& imageInfo = ImageInfo());

	/// \brief Returns the file info after the dialog has been closed with "OK".
	const ImageInfo& imageInfo() const { return _imageInfo; }

private Q_SLOTS:

	/// This is called when the user has pressed the OK button of the dialog box.
	void onFileSelected(const QString& file);

	/// This is called when the user has selected a file format.
	void onFilterSelected(const QString& filter);

private:

	QList<QByteArray> _formatList;
	ImageInfo _imageInfo;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_SAVE_IMAGE_FILE_DIALOG_H
