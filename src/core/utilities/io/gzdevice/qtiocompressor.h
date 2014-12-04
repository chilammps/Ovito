/****************************************************************************
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
** 
** This file is part of a Qt Solutions component.
**
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact Nokia at qt-info@nokia.com.
** 
****************************************************************************/

#ifndef QTIOCOMPRESSOR_H
#define QTIOCOMPRESSOR_H

#include <core/Core.h>

namespace Ovito { namespace Util {

class QtIOCompressorPrivate;

class OVITO_CORE_EXPORT QtIOCompressor : public QIODevice
{
	Q_OBJECT
public:

	enum StreamFormat {
		ZlibFormat,
		GzipFormat,
		RawZipFormat
	};

    QtIOCompressor(QIODevice *device, int compressionLevel = 6, int bufferSize = 65500);
    virtual ~QtIOCompressor();

    void setStreamFormat(StreamFormat format);
    StreamFormat streamFormat() const;
    static bool isGzipSupported();

    bool isSequential() const override;
    bool open(OpenMode mode) override;
    void close() override;
    void flush();
    qint64 bytesAvailable() const override;
    bool seek(qint64 pos) override;

protected:

    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);

private:

    static bool checkGzipSupport(const char * const versionString);
    QtIOCompressorPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QtIOCompressor);
    Q_DISABLE_COPY(QtIOCompressor);
};

}} // End of namespace

#endif
