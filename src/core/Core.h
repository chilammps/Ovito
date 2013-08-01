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

#ifndef __OVITO_CORE_H
#define __OVITO_CORE_H

/******************************************************************************
* The Base module is required by the Core module.
******************************************************************************/
#include <base/Base.h>
#include <base/linalg/LinAlg.h>
#include <base/utilities/Color.h>

#ifdef OVITO_CORE_LIBRARY
#  define OVITO_CORE_EXPORT Q_DECL_EXPORT
#else
#  define OVITO_CORE_EXPORT Q_DECL_IMPORT
#endif

/******************************************************************************
* Include some basic headers.
******************************************************************************/
#include <core/object/OvitoObject.h>

#endif // __OVITO_CORE_H
