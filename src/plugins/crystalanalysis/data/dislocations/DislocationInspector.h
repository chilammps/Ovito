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

#ifndef __OVITO_CA_DISLOCATION_INSPECTOR_H
#define __OVITO_CA_DISLOCATION_INSPECTOR_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/viewport/input/ViewportInputHandler.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/gui/actions/ViewportModeAction.h>
#include <core/gui/properties/RefTargetListParameterUI.h>
#include <core/scene/ObjectNode.h>

namespace CrystalAnalysis {

using namespace Ovito;

/******************************************************************************
* A special properties editor for the Dislocations class.
******************************************************************************/
class OVITO_CRYSTALANALYSIS_EXPORT DislocationInspector : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE DislocationInspector(ObjectNode* sceneNode) {
		INIT_PROPERTY_FIELD(DislocationInspector::_sceneNode);
		_sceneNode = sceneNode;
	}

	RefTargetListParameterUI* dislocationListUI() const { return _dislocationListUI; }

	virtual ~DislocationInspector() {
		//if(_pickDislocationHandler)
		//	VIEWPORT_INPUT_MANAGER.removeInputHandler(_pickDislocationHandler.get());
	}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private Q_SLOTS:

	void onHideAll();
	void onShowAll();
	void onHideSelected();
	void onHideUnselected();
	void onShowSelected();

private:

	//OORef<ViewportInputHandler> _pickDislocationHandler;
	//OORef<ViewportModeAction> _pickDislocationAction;
	RefTargetListParameterUI* _dislocationListUI;
	QSortFilterProxyModel* _sortedModel;

	/// The scene node being loaded in the editor.
	ReferenceField<ObjectNode> _sceneNode;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_sceneNode);
};

};	// End of namespace

#endif // __OVITO_CA_DISLOCATIONS_EDITOR_H
