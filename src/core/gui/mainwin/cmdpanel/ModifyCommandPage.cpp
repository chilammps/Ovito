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
#include <core/scene/objects/DataObject.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/SelectionSet.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/dataset/UndoStack.h>
#include <core/dataset/DataSetContainer.h>
#include <core/gui/actions/ActionManager.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/widgets/selection/SceneNodeSelectionBox.h>
#include "ModifyCommandPage.h"
#include "ModificationListModel.h"
#include "ModifierListBox.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Initializes the modify page.
******************************************************************************/
ModifyCommandPage::ModifyCommandPage(MainWindow* mainWindow, QWidget* parent) : QWidget(parent),
		_datasetContainer(mainWindow->datasetContainer()), _actionManager(mainWindow->actionManager())
{
	QGridLayout* layout = new QGridLayout(this);
	layout->setContentsMargins(2,2,2,2);
	layout->setSpacing(4);
	layout->setColumnStretch(1,1);

	SceneNodeSelectionBox* nodeSelBox = new SceneNodeSelectionBox(_datasetContainer, this);
	layout->addWidget(nodeSelBox, 0, 0, 1, 2);

	_modificationListModel = new ModificationListModel(_datasetContainer, this);
	_modifierSelector = new ModifierListBox(this, _modificationListModel);
    layout->addWidget(_modifierSelector, 1, 0, 1, 2);
    connect(_modifierSelector, (void (QComboBox::*)(int))&QComboBox::activated, this, &ModifyCommandPage::onModifierAdd);

	class ModifierStackListView : public QListView {
	public:
		ModifierStackListView(QWidget* parent) : QListView(parent) {}
		virtual QSize sizeHint() const { return QSize(256, 260); }
	};

	QSplitter* splitter = new QSplitter(Qt::Vertical);
	splitter->setChildrenCollapsible(false);

	QWidget* upperContainer = new QWidget();
	splitter->addWidget(upperContainer);
	QHBoxLayout* subLayout = new QHBoxLayout(upperContainer);
	subLayout->setContentsMargins(0,0,0,0);
	subLayout->setSpacing(2);

	_modificationListWidget = new ModifierStackListView(upperContainer);
	_modificationListWidget->setModel(_modificationListModel);
	_modificationListWidget->setSelectionModel(_modificationListModel->selectionModel());
	connect(_modificationListModel, &ModificationListModel::selectedItemChanged, this, &ModifyCommandPage::onSelectedItemChanged);
	connect(_modificationListWidget, &ModifierStackListView::doubleClicked, this, &ModifyCommandPage::onModifierStackDoubleClicked);
	subLayout->addWidget(_modificationListWidget);

	QToolBar* editToolbar = new QToolBar(this);
	editToolbar->setOrientation(Qt::Vertical);
#ifndef Q_OS_MACX
	editToolbar->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; }");
#endif
	subLayout->addWidget(editToolbar);

	QAction* deleteModifierAction = _actionManager->createCommandAction(ACTION_MODIFIER_DELETE, tr("Delete Modifier"), ":/core/actions/modify/delete_modifier.png");
	connect(deleteModifierAction, &QAction::triggered, this, &ModifyCommandPage::onDeleteModifier);
	editToolbar->addAction(deleteModifierAction);

	editToolbar->addSeparator();

	QAction* moveModifierUpAction = _actionManager->createCommandAction(ACTION_MODIFIER_MOVE_UP, tr("Move Modifier Up"), ":/core/actions/modify/modifier_move_up.png");
	connect(moveModifierUpAction, &QAction::triggered, this, &ModifyCommandPage::onModifierMoveUp);
	editToolbar->addAction(moveModifierUpAction);
	QAction* moveModifierDownAction = mainWindow->actionManager()->createCommandAction(ACTION_MODIFIER_MOVE_DOWN, tr("Move Modifier Down"), ":/core/actions/modify/modifier_move_down.png");
	connect(moveModifierDownAction, &QAction::triggered, this, &ModifyCommandPage::onModifierMoveDown);
	editToolbar->addAction(moveModifierDownAction);

	QAction* toggleModifierStateAction = _actionManager->createCommandAction(ACTION_MODIFIER_TOGGLE_STATE, tr("Enable/Disable Modifier"));
	toggleModifierStateAction->setCheckable(true);
	QIcon toggleStateActionIcon(QString(":/core/actions/modify/modifier_enabled_large.png"));
	toggleStateActionIcon.addFile(QString(":/core/actions/modify/modifier_disabled_large.png"), QSize(), QIcon::Normal, QIcon::On);
	toggleModifierStateAction->setIcon(toggleStateActionIcon);
	connect(toggleModifierStateAction, &QAction::triggered, this, &ModifyCommandPage::onModifierToggleState);

	editToolbar->addSeparator();
	QAction* helpAction = new QAction(QIcon(":/core/mainwin/command_panel/help.png"), tr("Open Online Help"), this);
	connect(helpAction, &QAction::triggered, [mainWindow] {
		mainWindow->openHelpTopic("usage.modification_pipeline.html");
	});
	editToolbar->addAction(helpAction);

	layout->addWidget(splitter, 2, 0, 1, 2);
	layout->setRowStretch(2, 1);

	// Create the properties panel.
	_propertiesPanel = new PropertiesPanel(nullptr);
	_propertiesPanel->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	splitter->addWidget(_propertiesPanel);
	splitter->setStretchFactor(1,1);

	connect(&_datasetContainer, &DataSetContainer::selectionChangeComplete, this, &ModifyCommandPage::onSelectionChangeComplete);
	updateActions(nullptr);

	// Create About panel.
	createAboutPanel();
}

/******************************************************************************
* This is called after all changes to the selection set have been completed.
******************************************************************************/
void ModifyCommandPage::onSelectionChangeComplete(SelectionSet* newSelection)
{
	// Make sure we get informed about any future changes of the selection set.
	_modificationListModel->refreshList();
}

/******************************************************************************
* Is called when a new modification list item has been selected, or if the currently
* selected item has changed.
******************************************************************************/
void ModifyCommandPage::onSelectedItemChanged()
{
	ModificationListItem* currentItem = _modificationListModel->selectedItem();
	RefTarget* object = currentItem ? currentItem->object() : nullptr;

	if(currentItem != nullptr)
		_aboutRollout->hide();

	if(object != _propertiesPanel->editObject()) {
		_propertiesPanel->setEditObject(object);
		if(_datasetContainer.currentSet())
			_datasetContainer.currentSet()->viewportConfig()->updateViewports();
	}
	updateActions(currentItem);

	// Whenever no object is selected, show the About Panel containing information about the program.
	if(currentItem == nullptr)
		_aboutRollout->show();
}

/******************************************************************************
* Updates the state of the actions that can be invoked on the currently selected item.
******************************************************************************/
void ModifyCommandPage::updateActions(ModificationListItem* currentItem)
{
	QAction* deleteModifierAction = _actionManager->getAction(ACTION_MODIFIER_DELETE);
	QAction* moveModifierUpAction = _actionManager->getAction(ACTION_MODIFIER_MOVE_UP);
	QAction* moveModifierDownAction = _actionManager->getAction(ACTION_MODIFIER_MOVE_DOWN);
	QAction* toggleModifierStateAction = _actionManager->getAction(ACTION_MODIFIER_TOGGLE_STATE);

	_modifierSelector->setEnabled(currentItem != nullptr);

	Modifier* modifier = currentItem ? dynamic_object_cast<Modifier>(currentItem->object()) : nullptr;
	if(modifier) {
		deleteModifierAction->setEnabled(true);
		if(currentItem->modifierApplications().size() == 1) {
			ModifierApplication* modApp = currentItem->modifierApplications()[0];
			PipelineObject* pipelineObj = modApp->pipelineObject();
			if(pipelineObj) {
				OVITO_ASSERT(pipelineObj->modifierApplications().contains(modApp));
				moveModifierUpAction->setEnabled(modApp != pipelineObj->modifierApplications().back());
				moveModifierDownAction->setEnabled(modApp != pipelineObj->modifierApplications().front());
			}
		}
		else {
			moveModifierUpAction->setEnabled(false);
			moveModifierDownAction->setEnabled(false);
		}
		if(modifier) {
			toggleModifierStateAction->setEnabled(true);
			toggleModifierStateAction->setChecked(modifier->isEnabled() == false);
		}
		else {
			toggleModifierStateAction->setChecked(false);
			toggleModifierStateAction->setEnabled(false);
		}
	}
	else {
		deleteModifierAction->setEnabled(false);
		moveModifierUpAction->setEnabled(false);
		moveModifierDownAction->setEnabled(false);
		toggleModifierStateAction->setChecked(false);
		toggleModifierStateAction->setEnabled(false);
	}
}

/******************************************************************************
* Is called when the user has selected an item in the modifier class list.
******************************************************************************/
void ModifyCommandPage::onModifierAdd(int index)
{
	if(index >= 0 && _modificationListModel->isUpToDate()) {
		const OvitoObjectType* descriptor = static_cast<const OvitoObjectType*>(_modifierSelector->itemData(index).value<void*>());
		if(descriptor) {
			UndoableTransaction::handleExceptions(_datasetContainer.currentSet()->undoStack(), tr("Apply modifier"), [descriptor, this]() {
				// Create an instance of the modifier.
				OORef<Modifier> modifier = static_object_cast<Modifier>(descriptor->createInstance(_datasetContainer.currentSet()));
				OVITO_CHECK_OBJECT_POINTER(modifier);
				// Load user-defined default parameters.
				modifier->loadUserDefaults();
				// Apply it.
				_modificationListModel->applyModifier(modifier);
			});
			_modificationListModel->requestUpdate();
		}
		_modifierSelector->setCurrentIndex(0);
	}
}

/******************************************************************************
* Handles the ACTION_MODIFIER_DELETE command.
******************************************************************************/
void ModifyCommandPage::onDeleteModifier()
{
	// Get the currently selected modifier.
	ModificationListItem* selectedItem = _modificationListModel->selectedItem();
	if(!selectedItem) return;

	Modifier* modifier = dynamic_object_cast<Modifier>(selectedItem->object());
	if(!modifier) return;

	UndoableTransaction::handleExceptions(_datasetContainer.currentSet()->undoStack(), tr("Delete modifier"), [selectedItem, modifier]() {

		// Remove each ModifierApplication from the corresponding PipelineObject.
		Q_FOREACH(ModifierApplication* modApp, selectedItem->modifierApplications()) {
			OVITO_ASSERT(modApp->modifier() == modifier);
			OVITO_CHECK_OBJECT_POINTER(modApp->pipelineObject());
			modApp->pipelineObject()->removeModifier(modApp);
		}

		// Delete modifier.
		modifier->deleteReferenceObject();
	});
}

/******************************************************************************
* This called when the user double clicks on an item in the modifier stack.
******************************************************************************/
void ModifyCommandPage::onModifierStackDoubleClicked(const QModelIndex& index)
{
	ModificationListItem* item = _modificationListModel->item(index.row());
	OVITO_CHECK_OBJECT_POINTER(item);

	Modifier* modifier = dynamic_object_cast<Modifier>(item->object());
	if(modifier) {
		// Toggle enabled state of modifier.
		UndoableTransaction::handleExceptions(_datasetContainer.currentSet()->undoStack(), tr("Toggle modifier state"), [modifier]() {
			modifier->setEnabled(!modifier->isEnabled());
		});
	}
}

/******************************************************************************
* Handles the ACTION_MODIFIER_MOVE_UP command, which moves the selected
* modifier up one position in the stack.
******************************************************************************/
void ModifyCommandPage::onModifierMoveUp()
{
	// Get the currently selected modifier.
	ModificationListItem* selectedItem = _modificationListModel->selectedItem();
	if(!selectedItem) return;

	if(selectedItem->modifierApplications().size() != 1)
		return;

	OORef<ModifierApplication> modApp = selectedItem->modifierApplications()[0];
	OORef<PipelineObject> pipelineObj = modApp->pipelineObject();
	if(!pipelineObj)
		return;

	OVITO_ASSERT(pipelineObj->modifierApplications().contains(modApp));
	if(modApp == pipelineObj->modifierApplications().back())
		return;

	UndoableTransaction::handleExceptions(_datasetContainer.currentSet()->undoStack(), tr("Move modifier up"), [pipelineObj, modApp]() {
		// Determine old position in stack.
		int index = pipelineObj->modifierApplications().indexOf(modApp);
		// Remove ModifierApplication from the PipelineObject.
		pipelineObj->removeModifier(modApp);
		// Re-insert ModifierApplication into the PipelineObject.
		pipelineObj->insertModifierApplication(modApp, index+1);
	});
}

/******************************************************************************
* Handles the ACTION_MODIFIER_MOVE_DOWN command, which moves the selected
* modifier down one position in the stack.
******************************************************************************/
void ModifyCommandPage::onModifierMoveDown()
{
	// Get the currently selected modifier.
	ModificationListItem* selectedItem = _modificationListModel->selectedItem();
	if(!selectedItem) return;

	if(selectedItem->modifierApplications().size() != 1)
		return;

	OORef<ModifierApplication> modApp = selectedItem->modifierApplications()[0];
	OORef<PipelineObject> pipelineObj = modApp->pipelineObject();
	if(!pipelineObj) return;

	OVITO_ASSERT(pipelineObj->modifierApplications().contains(modApp));
	if(modApp == pipelineObj->modifierApplications().front())
		return;

	UndoableTransaction::handleExceptions(_datasetContainer.currentSet()->undoStack(), tr("Move modifier down"), [pipelineObj, modApp]() {
		// Determine old position in stack.
		int index = pipelineObj->modifierApplications().indexOf(modApp);
		// Remove ModifierApplication from the PipelineObject.
		pipelineObj->removeModifier(modApp);
		// Re-insert ModifierApplication into the PipelineObject.
		pipelineObj->insertModifierApplication(modApp, index-1);
	});
}

/******************************************************************************
* Handles the ACTION_MODIFIER_TOGGLE_STATE command, which toggles the
* enabled/disable state of the selected modifier.
******************************************************************************/
void ModifyCommandPage::onModifierToggleState(bool newState)
{
	// Get the selected modifier from the modifier stack box.
	QModelIndexList selection = _modificationListWidget->selectionModel()->selectedRows();
	if(selection.empty())
		return;

	onModifierStackDoubleClicked(selection.front());
}

/******************************************************************************
* Creates the rollout panel that shows information about the application
* whenever no object is selected.
******************************************************************************/
void ModifyCommandPage::createAboutPanel()
{
	QWidget* rollout = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(8,8,8,8);

	QTextBrowser* aboutLabel = new QTextBrowser(rollout);
	aboutLabel->setObjectName("AboutLabel");
	aboutLabel->setOpenExternalLinks(true);
	aboutLabel->setMinimumHeight(600);
	aboutLabel->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	aboutLabel->viewport()->setAutoFillBackground(false);
	layout->addWidget(aboutLabel);

	QSettings settings;
	QByteArray newsPage;
	if(settings.value("updates/check_for_updates", true).toBool()) {
		// Retrieve cached news page from settings store.
		newsPage = settings.value("news/cached_webpage").toByteArray();
	}
	if(newsPage.isEmpty()) {
		QResource res("/core/mainwin/command_panel/about_panel.html");
		newsPage = QByteArray((const char *)res.data(), (int)res.size());
	}
	aboutLabel->setHtml(QString::fromUtf8(newsPage.constData()));

	_aboutRollout = _propertiesPanel->addRollout(rollout, QCoreApplication::applicationName());

	if(settings.value("updates/check_for_updates", true).toBool()) {

		// Retrieve/generate unique installation id.
		QByteArray id;
		if(settings.value("updates/transmit_id", true).toBool()) {
			if(settings.contains("installation/id")) {
				id = settings.value("id").toByteArray();
				if(id == QByteArray(16, '\0') || id.size() != 16)
					id.clear();
			}
			if(id.isEmpty()) {
				// Generate a new unique ID.
				id.fill('0', 16);
				std::random_device rdev;
				std::uniform_int_distribution<> rdist(0, 0xFF);
				for(auto& c : id)
					c = (char)rdist(rdev);
				settings.setValue("installation/id", id);
			}
		}
		else {
			id.fill(0, 16);
		}

		QString operatingSystemString;
#if defined(Q_OS_MAC)
		operatingSystemString = QStringLiteral("macosx");
#elif defined(Q_OS_WIN)
		operatingSystemString = QStringLiteral("win");
#elif defined(Q_OS_LINUX)
		operatingSystemString = QStringLiteral("linux");
#endif

		// Fetch newest web page from web server.
		QNetworkAccessManager* networkAccessManager = new QNetworkAccessManager(_aboutRollout);
		QString urlString = QString("http://www.ovito.org/appnews/v%1.%2.%3/?ovito=%4&OS=%5%6")
				.arg(OVITO_VERSION_MAJOR)
				.arg(OVITO_VERSION_MINOR)
				.arg(OVITO_VERSION_REVISION)
				.arg(QString(id.toHex()))
				.arg(operatingSystemString)
				.arg(QT_POINTER_SIZE*8);
		QNetworkReply* networkReply = networkAccessManager->get(QNetworkRequest(QUrl(urlString)));
		connect(networkAccessManager, &QNetworkAccessManager::finished, this, &ModifyCommandPage::onWebRequestFinished);
	}
}

/******************************************************************************
* Is called by the system when fetching the news web page from the server is
* completed.
******************************************************************************/
void ModifyCommandPage::onWebRequestFinished(QNetworkReply* reply)
{
	if(reply->error() == QNetworkReply::NoError) {
		QByteArray page = reply->readAll();
		reply->close();
		if(page.startsWith("<html><!--OVITO-->")) {

			QTextBrowser* aboutLabel = _aboutRollout->findChild<QTextBrowser*>("AboutLabel");
			OVITO_CHECK_POINTER(aboutLabel);
			aboutLabel->setHtml(QString::fromUtf8(page.constData()));

			QSettings settings;
			settings.setValue("news/cached_webpage", page);
		}
#if 0
		else {
			qDebug() << "News page fetched from server is invalid.";
		}
#endif
	}
#if 0
	else {
		qDebug() << "Failed to fetch news page from server: " << reply->errorString();
	}
#endif
	reply->deleteLater();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
