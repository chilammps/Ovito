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

/******************************************************************************
*
******************************************************************************/
/**
 * \file
 * \brief Defines several macros for debugging purposes.
 */

#ifndef __OVITO_DEBUGGING_H
#define __OVITO_DEBUGGING_H

namespace Ovito {

/******************************************************************************
* This macro performs a runtime-time assertion check.
******************************************************************************/
#ifdef OVITO_DEBUG
#define OVITO_ASSERT(condition) Q_ASSERT(condition)
#else
#define OVITO_ASSERT(condition)
#endif

/******************************************************************************
* This macro performs a runtime-time assertion check.
******************************************************************************/
#ifdef OVITO_DEBUG
#define OVITO_ASSERT_MSG(condition, where, what) Q_ASSERT_X(condition, where, what)
#else
#define OVITO_ASSERT_MSG(condition, where, what)
#endif

/******************************************************************************
* This macro performs a compile-time check.
******************************************************************************/
#define OVITO_STATIC_ASSERT(condition) Q_STATIC_ASSERT(condition)

/******************************************************************************
* This macro validates a memory pointer in debug mode.
* If the given pointer does not point to a valid position in memory then
* the debugger is activated.
******************************************************************************/
#define OVITO_CHECK_POINTER(pointer) OVITO_ASSERT_MSG((pointer), "OVITO_CHECK_POINTER", "Invalid object pointer.");

}	// End of namespace

#endif // __OVITO_DEBUGGING_H
