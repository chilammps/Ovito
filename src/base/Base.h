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

/******************************************************************************
* QT Library
******************************************************************************/
#include <QApplication>
#include <QException>
#include <QStringList>
#include <QIcon>
#include <QMessageBox>
#include <QMainWindow>
#include <QSettings>
#include <QToolBar>
#include <QStatusBar>
#include <QStatusTipEvent>
#include <QMenuBar>
#include <QMenu>
#include <QUndoStack>
#include <QDesktopServices>
#include <QUrl>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyleOptionMenuItem>
#include <QDockWidget>
#include <QLineEdit>
#include <QPointer>
#include <QStyle>
#include <QStylePainter>
#include <QDesktopWidget>

// Defines the number type used for numerical computations.
#include "utilities/Debugging.h"
#include "utilities/FloatType.h"
#include "utilities/Exception.h"

#endif // __OVITO_BASE_H
