///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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
 * \file
 * \brief This file includes STL and third-party library headers required by OVITO.
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
#include <atomic>
#include <tuple>

/******************************************************************************
* Boost Library
******************************************************************************/
#include <boost/dynamic_bitset.hpp>

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

#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
#  error "OVITO requires at least Qt 5.2"
#endif

#ifdef OVITO_BASE_LIBRARY
#  define OVITO_BASE_EXPORT Q_DECL_EXPORT
#else
#  define OVITO_BASE_EXPORT Q_DECL_IMPORT
#endif

/*! \namespace Ovito
    \brief The root namespace of OVITO.
*/
/*! \namespace Ovito::Util
    \brief This namespace contains utility classes and typedefs used throughout OVITO's codebase.
*/
/*! \namespace Ovito::Math
    \brief This namespace contains classes related to linear algebra and geometry (vectors, transformation matrices, etc).
*/

// Pull all sub-namespaces into the root Ovito namespace.
namespace Ovito {
	namespace Util {}
	namespace Math {}
	using namespace Util;
	using namespace Math;
}

/******************************************************************************
* Our own basic headers
******************************************************************************/
#include "utilities/Debugging.h"
#include "utilities/FloatType.h"
#include "utilities/Exception.h"

#endif // __OVITO_BASE_H
