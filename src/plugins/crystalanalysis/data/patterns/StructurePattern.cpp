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

#include <core/Core.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/StringParameterUI.h>
#include "StructurePattern.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, StructurePattern, ParticleType);
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, StructurePatternEditor, PropertiesEditor);
SET_OVITO_OBJECT_EDITOR(StructurePattern, StructurePatternEditor);
DEFINE_PROPERTY_FIELD(StructurePattern, _shortName, "ShortName");
DEFINE_PROPERTY_FIELD(StructurePattern, _structureType, "StructureType");
DEFINE_VECTOR_REFERENCE_FIELD(StructurePattern, _burgersVectorFamilies, "BurgersVectorFamilies", BurgersVectorFamily);
SET_PROPERTY_FIELD_LABEL(StructurePattern, _shortName, "Short name");
SET_PROPERTY_FIELD_LABEL(StructurePattern, _structureType, "Structure type");
SET_PROPERTY_FIELD_LABEL(StructurePattern, _burgersVectorFamilies, "Burgers vector families");

/******************************************************************************
* Constructs the StructurePattern object.
******************************************************************************/
StructurePattern::StructurePattern(DataSet* dataset) : ParticleType(dataset), _structureType(Lattice)
{
	INIT_PROPERTY_FIELD(StructurePattern::_shortName);
	INIT_PROPERTY_FIELD(StructurePattern::_structureType);
	INIT_PROPERTY_FIELD(StructurePattern::_burgersVectorFamilies);

	// Create "unknown" Burgers vector family.
	BurgersVectorFamily* family = new BurgersVectorFamily(dataset);
	family->setColor(Color(0.7f, 0.7f, 0.7f));
	family->setName(tr("Other"));
	family->setBurgersVector(Vector3::Zero());
	addBurgersVectorFamily(family);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void StructurePatternEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Structure type"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(4);

	// Derive a custom class from the list parameter UI to
	// give the items a color.
	class CustomRefTargetListParameterUI : public RefTargetListParameterUI {
	public:
		CustomRefTargetListParameterUI(PropertiesEditor* parentEditor, const PropertyFieldDescriptor& refField)
			: RefTargetListParameterUI(parentEditor, refField, RolloutInsertionParameters(), nullptr) {}
	protected:

		virtual QVariant getItemData(RefTarget* target, const QModelIndex& index, int role) override {
			if(target) {
				if(role == Qt::DisplayRole) {
					if(index.column() == 2)
						return target->objectTitle();
				}
				else if(role == Qt::DecorationRole) {
					if(index.column() == 1)
						return (QColor)static_object_cast<BurgersVectorFamily>(target)->color();
				}
				else if(role == Qt::CheckStateRole) {
					if(index.column() == 0)
						return QVariant(static_object_cast<BurgersVectorFamily>(target)->isVisible() ? Qt::Checked : Qt::Unchecked);
				}
			}
			return QVariant();
		}

		/// Returns the number of columns for the table view.
		virtual int tableColumnCount() override { return 3; }

		/// Returns the header data under the given role for the given RefTarget.
		/// This method is part of the data model used by the list widget and can be overridden
		/// by sub-classes.
		virtual QVariant getHorizontalHeaderData(int index, int role) override {
			if(index == 0)
				return tr("Show");
			else if(index == 1)
				return tr("Color");
			else
				return tr("Name");
		}

		/// Returns the model/view item flags for the given entry.
		virtual Qt::ItemFlags getItemFlags(RefTarget* target, const QModelIndex& index) override {
			if(index.column() == 0)
				return RefTargetListParameterUI::getItemFlags(target, index) | Qt::ItemIsUserCheckable;
			else
				return RefTargetListParameterUI::getItemFlags(target, index);
		}

		/// Sets the role data for the item at index to value.
		virtual bool setItemData(RefTarget* target, const QModelIndex& index, const QVariant& value, int role) override {
			if(index.isValid() && index.column() == 0 && role == Qt::CheckStateRole) {
				UndoableTransaction::handleExceptions(dataset()->undoStack(), tr("Show/hide Burgers vector family"), [target, &value]() {
					static_object_cast<BurgersVectorFamily>(target)->setVisible(value.value<int>() == Qt::Checked);
				});
				return true;
			}
			else return RefTargetListParameterUI::setItemData(target, index, value, role);
		}

		/// Do not open sub-editor for selected item.
		virtual void openSubEditor() override {}
	};

	layout1->addWidget(new QLabel(tr("Burgers vector families:")));
	familiesListUI = new CustomRefTargetListParameterUI(this, PROPERTY_FIELD(StructurePattern::_burgersVectorFamilies));
	layout1->addWidget(familiesListUI->tableWidget(200));
	familiesListUI->tableWidget()->setAutoScroll(false);
	connect(familiesListUI->tableWidget(), &QTableWidget::doubleClicked, this, &StructurePatternEditor::onDoubleClickBurgersFamily);
}

/******************************************************************************
* Is called when the user has double-clicked on one of the entries in the
* list widget.
******************************************************************************/
void StructurePatternEditor::onDoubleClickBurgersFamily(const QModelIndex& index)
{
	// Let the user select a color for the Burgers vector family.
	BurgersVectorFamily* family = static_object_cast<BurgersVectorFamily>(familiesListUI->selectedObject());
	if(!family) return;

	QColor oldColor = Color(family->color());
	QColor newColor = QColorDialog::getColor(oldColor, container());
	if(!newColor.isValid() || newColor == oldColor)
		return;

	undoableTransaction(tr("Change Burgers vector family color"), [family, &newColor]() {
		family->setColor(Color(newColor));
	});
}

}	// End of namespace
}	// End of namespace
}	// End of namespace
