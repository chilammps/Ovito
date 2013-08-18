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

/**
 * \file Base.h
 * \brief This header file includes the standard system headers used by the basic classes.
 */

#ifndef __OVITO_BASE_H
#define __OVITO_BASE_H

/******************************************************************************
* Standard Template Library (STL)
******************************************************************************/
#include <iostream>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <stack>
#include <array>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <random>
#include <memory>
#include <mutex>
#include <thread>
#include <clocale>

/******************************************************************************
* QT Library
******************************************************************************/
#include <QApplication>
#include <QException>
#include <QStringList>
#include <QSettings>
#include <QMenuBar>
#include <QMenu>
#include <QUrl>
#include <QPointer>
#include <QFileInfo>
#include <QResource>
#include <QDir>
#include <QtWidgets>
#include <QtDebug>
#include <QtXml>
#include <QtGui>
#include <QtNetwork>
#include <qopengl.h>

#ifdef OVITO_BASE_LIBRARY
#  define OVITO_BASE_EXPORT Q_DECL_EXPORT
#else
#  define OVITO_BASE_EXPORT Q_DECL_IMPORT
#endif

// Defines the number type used for numerical computations.
#include "utilities/Debugging.h"
#include "utilities/FloatType.h"
#include "utilities/Exception.h"

#endif // __OVITO_BASE_H
