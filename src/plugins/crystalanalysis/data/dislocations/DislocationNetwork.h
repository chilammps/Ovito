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

#ifndef __OVITO_CA_DISLOCATION_NETWORK_H
#define __OVITO_CA_DISLOCATION_NETWORK_H

#include <core/Core.h>
#include <core/scene/objects/DataObject.h>
#include <core/gui/properties/PropertiesEditor.h>
#include "DislocationSegment.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/**
 * \brief Stores a collection of dislocation segments.
 */
class OVITO_CRYSTALANALYSIS_EXPORT DislocationNetwork : public DataObject
{
public:

	/// \brief Constructor that creates an empty object.
	Q_INVOKABLE DislocationNetwork(DataSet* dataset);

	/// Returns the title of this object.
	virtual QString objectTitle() override { return tr("Dislocations"); }

	/// Returns the list of dislocation segments.
	const QVector<DislocationSegment*>& segments() const { return _segments; }

	/// Discards all existing dislocation segments.
	void clear() { _segments.clear(); }

	/// Adds a dislocation segment to this container.
	void addSegment(DislocationSegment* segment) { _segments.push_back(segment); }

protected:

	/// Stores the list of dislocation segments.
	VectorReferenceField<DislocationSegment> _segments;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_segments);
};

/******************************************************************************
* A properties editor for the DislocationNetwork class.
******************************************************************************/
class OVITO_CRYSTALANALYSIS_EXPORT DislocationNetworkEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE DislocationNetworkEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

protected Q_SLOTS:

	/// Is called when the user presses the "Open Inspector" button.
	void onOpenInspector();

private:

	Q_OBJECT
	OVITO_OBJECT
};

}	// End of namespace
}	// End of namespace
}	// End of namespace

#endif // __OVITO_CA_DISLOCATION_NETWORK_H
