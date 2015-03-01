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

#include <core/Core.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/dataset/UndoStack.h>
#include <core/dataset/DataSetContainer.h>
#include <core/plugins/PluginManager.h>
#include <core/gui/mainwin/MainWindow.h>
#include "OverlayCommandPage.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Initializes the command panel page.
******************************************************************************/
OverlayCommandPage::OverlayCommandPage(MainWindow* mainWindow, QWidget* parent) : QWidget(parent),
		_datasetContainer(mainWindow->datasetContainer())
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(2,2,2,2);
	layout->setSpacing(4);

	_activeViewportLabel = new QLabel(tr("Selected viewport:"));
	layout->addWidget(_activeViewportLabel);

	_newOverlayBox = new QComboBox(this);
    layout->addWidget(_newOverlayBox);
    connect(_newOverlayBox, (void (QComboBox::*)(int))&QComboBox::activated, this, &OverlayCommandPage::onNewOverlay);

    _newOverlayBox->addItem(tr("Add overlay..."));
    _newOverlayBox->insertSeparator(1);
	Q_FOREACH(const OvitoObjectType* clazz, PluginManager::instance().listClasses(ViewportOverlay::OOType)) {
		_newOverlayBox->addItem(clazz->displayName(), QVariant::fromValue(clazz));
	}

	QSplitter* splitter = new QSplitter(Qt::Vertical);
	splitter->setChildrenCollapsible(false);

	class OverlayListWidget : public QListWidget {
	public:
		OverlayListWidget(QWidget* parent) : QListWidget(parent) {}
		virtual QSize sizeHint() const override { return QSize(256, 120); }
	};

	QWidget* upperContainer = new QWidget();
	splitter->addWidget(upperContainer);
	QHBoxLayout* subLayout = new QHBoxLayout(upperContainer);
	subLayout->setContentsMargins(0,0,0,0);
	subLayout->setSpacing(2);

	_overlayListWidget = new OverlayListWidget(upperContainer);
	subLayout->addWidget(_overlayListWidget);

	QToolBar* editToolbar = new QToolBar(this);
	editToolbar->setOrientation(Qt::Vertical);
#ifndef Q_OS_MACX
	editToolbar->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; }");
#endif
	subLayout->addWidget(editToolbar);

	_deleteOverlayAction = new QAction(QIcon(":/core/actions/modify/delete_modifier.png"), tr("Delete Overlay"), this);
	_deleteOverlayAction->setEnabled(false);
	connect(_deleteOverlayAction, &QAction::triggered, this, &OverlayCommandPage::onDeleteOverlay);
	editToolbar->addAction(_deleteOverlayAction);

	editToolbar->addSeparator();
	QAction* overlayHelpAction = new QAction(QIcon(":/core/mainwin/command_panel/help.png"), tr("Open Online Help"), this);
	connect(overlayHelpAction, &QAction::triggered, [mainWindow] {
		mainWindow->openHelpTopic("viewport_overlays.html");
	});
	editToolbar->addAction(overlayHelpAction);

	layout->addWidget(splitter, 1);

	// Create the properties panel.
	_propertiesPanel = new PropertiesPanel(nullptr);
	_propertiesPanel->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	splitter->addWidget(_propertiesPanel);
	splitter->setStretchFactor(1,1);

	connect(&_datasetContainer, &DataSetContainer::viewportConfigReplaced, this, &OverlayCommandPage::onViewportConfigReplaced);
	connect(&_viewportListener, &RefTargetListener<Viewport>::notificationEvent, this, &OverlayCommandPage::viewportEvent);
	connect(_overlayListWidget, &QListWidget::itemSelectionChanged, this, &OverlayCommandPage::onItemSelectionChanged);
}

/******************************************************************************
* Returns the selected overlay.
******************************************************************************/
ViewportOverlay* OverlayCommandPage::selectedOverlay() const
{
	Viewport* vp = activeViewport();
	if(!vp) return nullptr;

	QList<QListWidgetItem*> selItems = _overlayListWidget->selectedItems();
	if(selItems.empty()) return nullptr;

	OverlayListItem* item = static_cast<OverlayListItem*>(selItems.front());
	return item->target();
}

/******************************************************************************
* Constructs an item for the list widget.
******************************************************************************/
OverlayCommandPage::OverlayListItem::OverlayListItem(ViewportOverlay* overlay)
	: RefTargetListener<ViewportOverlay>(),
	  QListWidgetItem(overlay->objectTitle(), nullptr, QListWidgetItem::UserType)
{
	setTarget(overlay);
}

/******************************************************************************
* This is called whenever the current viewport configuration of current dataset
* has been replaced by a new one.
******************************************************************************/
void OverlayCommandPage::onViewportConfigReplaced(ViewportConfiguration* newViewportConfiguration)
{
	disconnect(_activeViewportChangedConnection);
	if(newViewportConfiguration) {
		_activeViewportChangedConnection = connect(newViewportConfiguration, &ViewportConfiguration::activeViewportChanged, this, &OverlayCommandPage::onActiveViewportChanged);
		onActiveViewportChanged(newViewportConfiguration->activeViewport());
	}
	else onActiveViewportChanged(nullptr);
}

/******************************************************************************
* This is called when another viewport became active.
******************************************************************************/
void OverlayCommandPage::onActiveViewportChanged(Viewport* activeViewport)
{
	if(activeViewport)
		_activeViewportLabel->setText(tr("Selected viewport: %1").arg(activeViewport->viewportTitle()));
	else
		_activeViewportLabel->setText(tr("Selected viewport: <none>"));

	_viewportListener.setTarget(activeViewport);
	_overlayListWidget->clear();

	// Populate overlay list.
	if(activeViewport) {
		for(ViewportOverlay* overlay : activeViewport->overlays()) {
			QListWidgetItem* item = new OverlayListItem(overlay);
			_overlayListWidget->addItem(item);
		}
		if(_overlayListWidget->count() != 0)
			_overlayListWidget->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
	}

	_newOverlayBox->setEnabled(activeViewport != nullptr && _newOverlayBox->count() > 1);
}

/******************************************************************************
* This is called when the viewport generates a reference event.
******************************************************************************/
void OverlayCommandPage::viewportEvent(ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::ReferenceAdded) {
		ReferenceFieldEvent* refEvent = static_cast<ReferenceFieldEvent*>(event);
		if(refEvent->field() == PROPERTY_FIELD(Viewport::_overlays)) {
			ViewportOverlay* overlay = static_object_cast<ViewportOverlay>(refEvent->newTarget());
			QListWidgetItem* item = new OverlayListItem(overlay);
			_overlayListWidget->insertItem(refEvent->index(), item);
			_overlayListWidget->setCurrentRow(refEvent->index(), QItemSelectionModel::ClearAndSelect);
		}
	}
	else if(event->type() == ReferenceEvent::ReferenceRemoved) {
		ReferenceFieldEvent* refEvent = static_cast<ReferenceFieldEvent*>(event);
		if(refEvent->field() == PROPERTY_FIELD(Viewport::_overlays)) {
			delete _overlayListWidget->item(refEvent->index());
		}
	}
	else if(event->type() == ReferenceEvent::TitleChanged) {
		_activeViewportLabel->setText(tr("Selected viewport: %1").arg(activeViewport()->viewportTitle()));
	}
}

/******************************************************************************
* Is called when a new overlay has been selected in the list box.
******************************************************************************/
void OverlayCommandPage::onItemSelectionChanged()
{
	ViewportOverlay* overlay = selectedOverlay();
	_propertiesPanel->setEditObject(overlay);
	_deleteOverlayAction->setEnabled(overlay != nullptr);
}

/******************************************************************************
* This inserts a new overlay.
******************************************************************************/
void OverlayCommandPage::onNewOverlay(int index)
{
	if(index > 0) {
		const OvitoObjectType* descriptor = _newOverlayBox->itemData(index).value<const OvitoObjectType*>();
		Viewport* vp = activeViewport();
		if(descriptor && vp) {
			int index = _overlayListWidget->currentRow();
			if(index < 0) index = 0;
			UndoableTransaction::handleExceptions(_datasetContainer.currentSet()->undoStack(), tr("Add overlay"), [descriptor, vp, index]() {
				// Create an instance of the overlay class.
				OORef<ViewportOverlay> overlay = static_object_cast<ViewportOverlay>(descriptor->createInstance(vp->dataset()));
				// Load user-defined default parameters.
				overlay->loadUserDefaults();
				// Insert it.
				vp->insertOverlay(index, overlay);
				// Automatically activate preview mode to make the overlay visible.
				vp->setRenderPreviewMode(true);
			});
		}
		_newOverlayBox->setCurrentIndex(0);
	}
}

/******************************************************************************
* This deletes the selected overlay.
******************************************************************************/
void OverlayCommandPage::onDeleteOverlay()
{
	ViewportOverlay* overlay = selectedOverlay();
	if(!overlay) return;

	UndoableTransaction::handleExceptions(_datasetContainer.currentSet()->undoStack(), tr("Delete overlay"), [overlay]() {
		overlay->deleteReferenceObject();
	});
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
