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
 * \file ViewportConfiguration.h
 * \brief Contains the definition of the Ovito::ViewportConfiguration class.
 */

#ifndef __OVITO_VIEWPORT_CONFIGURATION_H
#define __OVITO_VIEWPORT_CONFIGURATION_H

#include <core/Core.h>
#include <core/viewport/Viewport.h>

namespace Ovito {

/**
 * \brief This class holds a collection of Viewport objects.
 *
 * It also keeps track of the current viewport and the maximized viewport.
 */
class OVITO_CORE_EXPORT ViewportConfiguration : public RefTarget
{
public:

	/// Constructor.
	Q_INVOKABLE ViewportConfiguration() {
		INIT_PROPERTY_FIELD(ViewportConfiguration::_viewports);
		INIT_PROPERTY_FIELD(ViewportConfiguration::_activeViewport);
		INIT_PROPERTY_FIELD(ViewportConfiguration::_maximizedViewport);
	}

	/// Returns the list of viewports.
	const QVector<Viewport*>& viewports() const { return _viewports; }

	/// Add a record for a new viewport.
	void addViewport(const OORef<Viewport>& vp) { _viewports.push_back(vp); }

	/// \brief Returns the active viewport.
	/// \return The active Viewport or \c NULL if no viewport is currently active.
	Viewport* activeViewport() { return _activeViewport; }

	/// \brief Sets the active viewport.
	/// \param vp The viewport to be made active.
	void setActiveViewport(Viewport* vp) {
		OVITO_ASSERT_MSG(vp == NULL || _viewports.contains(vp), "ViewportConfiguration::setActiveViewport", "Viewport is not in current configuration.");
		_activeViewport = vp;
	}

	/// \brief Returns the maximized viewport.
	/// \return The maximized viewport or \c NULL if no one is currently maximized.
	Viewport* maximizedViewport() { return _maximizedViewport; }

	/// \brief Maximizes a viewport.
	/// \param vp The viewport to be maximized or \c NULL to restore the currently maximized viewport to
	///           its original state.
	void setMaximizedViewport(Viewport* vp) {
		OVITO_ASSERT_MSG(vp == NULL || _viewports.contains(vp), "ViewportConfiguration::setMaximizedViewport", "Viewport is not in current configuration.");
		_maximizedViewport = vp;
	}

protected:

	/// Is called when the value of a reference field of this RefMaker changes.
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) override;

Q_SIGNALS:

	/// This signal is emitted when another viewport became active.
	void activeViewportChanged(Viewport* activeViewport);

	/// This signal is emitted when one of the viewports has been maximized.
	void maximizedViewportChanged(Viewport* maximizedViewport);

private:

	/// The list of viewports.
	VectorReferenceField<Viewport> _viewports;

	/// The active viewport. May be NULL.
	ReferenceField<Viewport> _activeViewport;

	/// The maximized viewport or NULL.
	ReferenceField<Viewport> _maximizedViewport;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_viewports)
	DECLARE_REFERENCE_FIELD(_activeViewport)
	DECLARE_REFERENCE_FIELD(_maximizedViewport)
};

};

#endif		// __OVITO_VIEWPORT_CONFIGURATION_H
