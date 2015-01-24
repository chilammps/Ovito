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

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/gui/properties/RefTargetListParameterUI.h>
#include "PatternCatalog.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, PatternCatalog, DataObject);
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, PatternCatalogEditor, PropertiesEditor);
SET_OVITO_OBJECT_EDITOR(PatternCatalog, PatternCatalogEditor);
DEFINE_VECTOR_REFERENCE_FIELD(PatternCatalog, _patterns, "Patterns", StructurePattern);
SET_PROPERTY_FIELD_LABEL(PatternCatalog, _patterns, "Structure patterns");

/******************************************************************************
* Constructs the PatternCatalog object.
******************************************************************************/
PatternCatalog::PatternCatalog(DataSet* dataset) : DataObject(dataset)
{
	INIT_PROPERTY_FIELD(PatternCatalog::_patterns);

	// Create the "undefined" structure.
	OORef<StructurePattern> undefinedAtomType(new StructurePattern(dataset));
	undefinedAtomType->setName(tr("Unidentified structure"));
	undefinedAtomType->setColor(Color(1,1,1));
	_patterns.push_back(undefinedAtomType);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void PatternCatalogEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Atomic structure catalog"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	// Derive a custom class from the list parameter UI to
	// give the items a color.
	class CustomRefTargetListParameterUI : public RefTargetListParameterUI {
	public:

		CustomRefTargetListParameterUI(PropertiesEditor* parentEditor, const PropertyFieldDescriptor& refField, const RolloutInsertionParameters& rolloutParams)
			: RefTargetListParameterUI(parentEditor, refField, rolloutParams, &StructurePatternEditor::OOType) {}

	protected:

		virtual QVariant getItemData(RefTarget* target, const QModelIndex& index, int role) override {
			if(target) {
				if(role == Qt::DisplayRole) {
					if(index.column() == 1)
						return target->objectTitle();
				}
				else if(role == Qt::DecorationRole) {
					if(index.column() == 0)
						return (QColor)static_object_cast<StructurePattern>(target)->color();
				}
			}
			return QVariant();
		}

		/// Returns the number of columns for the table view.
		virtual int tableColumnCount() override { return 2; }

		/// Returns the header data under the given role for the given RefTarget.
		/// This method is part of the data model used by the list widget and can be overriden
		/// by sub-classes.
		virtual QVariant getHorizontalHeaderData(int index, int role) override {
			if(index == 0)
				return tr("Color");
			else
				return tr("Name");
		}
	};

	QWidget* subEditorContainer = new QWidget(rollout);
	QVBoxLayout* sublayout = new QVBoxLayout(subEditorContainer);
	sublayout->setContentsMargins(0,0,0,0);
	layout->addWidget(subEditorContainer);

	layout->insertWidget(0, new QLabel(tr("Structure types:")));
	typesListUI = new CustomRefTargetListParameterUI(this, PROPERTY_FIELD(PatternCatalog::_patterns), RolloutInsertionParameters().insertInto(subEditorContainer));
	layout->insertWidget(1, typesListUI->tableWidget(200));
	typesListUI->tableWidget()->setAutoScroll(false);
	connect(typesListUI->tableWidget(), &QTableWidget::doubleClicked, this, &PatternCatalogEditor::onDoubleClickPattern);
}

/******************************************************************************
* Is called when the user has double-clicked on one of the entries in the
* list widget.
******************************************************************************/
void PatternCatalogEditor::onDoubleClickPattern(const QModelIndex& index)
{
	// Let the user select a color for the structure pattern.
	StructurePattern* pattern = static_object_cast<StructurePattern>(typesListUI->selectedObject());
	if(!pattern) return;

	QColor oldColor = Color(pattern->color());
	QColor newColor = QColorDialog::getColor(oldColor, container());
	if(!newColor.isValid() || newColor == oldColor)
		return;

	undoableTransaction(tr("Change structure type color"), [pattern, &newColor]() {
		pattern->setColor(Color(newColor));
	});
}

}	// End of namespace
}	// End of namespace
}	// End of namespace
