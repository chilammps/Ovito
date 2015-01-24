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
#include <core/gui/mainwin/MainWindow.h>
#include "RolloutContainer.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/******************************************************************************
* Constructs the container.
******************************************************************************/
RolloutContainer::RolloutContainer(QWidget* parent) : QScrollArea(parent)
{
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	setWidgetResizable(true);
	QWidget* widget = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout(widget);
	layout->setContentsMargins(QMargins());
	layout->setSpacing(2);
	layout->addStretch(0);
	widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	setWidget(widget);
}

/******************************************************************************
* Inserts a new rollout into the container.
******************************************************************************/
Rollout* RolloutContainer::addRollout(QWidget* content, const QString& title, const RolloutInsertionParameters& params, const char* helpPage)
{
	OVITO_CHECK_POINTER(content);
	Rollout* rollout = new Rollout(widget(), content, title, params, helpPage);
	QBoxLayout* layout = static_cast<QBoxLayout*>(widget()->layout());
	if(params._afterThisRollout) {
		Rollout* otherRollout = qobject_cast<Rollout*>(params._afterThisRollout->parent());
		for(int i = 0; i < layout->count(); i++) {
			if(layout->itemAt(i)->widget() == otherRollout) {
				layout->insertWidget(i + 1, rollout);
			}
		}
	}
	else if(params._beforeThisRollout) {
		Rollout* otherRollout = qobject_cast<Rollout*>(params._beforeThisRollout->parent());
		for(int i = 0; i < layout->count(); i++) {
			if(layout->itemAt(i)->widget() == otherRollout) {
				layout->insertWidget(i, rollout);
			}
		}
	}
	else layout->insertWidget(layout->count() - 1, rollout);
	return rollout;
}

/******************************************************************************
* Updates the size of all rollouts.
******************************************************************************/
void RolloutContainer::updateRollouts()
{
	for(QObject* child : widget()->children()) {
		if(child->isWidgetType()) {
			static_cast<QWidget*>(child)->updateGeometry();
		}
	}
}

/******************************************************************************
* Constructs a rollout widget.
******************************************************************************/
Rollout::Rollout(QWidget* parent, QWidget* content, const QString& title, const RolloutInsertionParameters& params, const char* helpPage) :
	QWidget(parent), _content(content), _collapseAnimation(this, "visiblePercentage"), _useAvailableSpace(params._useAvailableSpace), _helpPage(helpPage)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	_collapseAnimation.setDuration(350);
	_collapseAnimation.setEasingCurve(QEasingCurve::InOutCubic);

	// Set initial open/collapsed state.
	if(!params._animateFirstOpening && !params._collapsed)
		_visiblePercentage = 100;
	else
		_visiblePercentage = 0;

	// Insert contents.
	_content->setParent(this);
	_content->setVisible(true);
	connect(_content.data(), &QWidget::destroyed, this, &Rollout::deleteLater);

	// Set up title button.
	_titleButton = new QPushButton(title, this);
	_titleButton->setAutoFillBackground(true);
	_titleButton->setFocusPolicy(Qt::NoFocus);
	_titleButton->setStyleSheet("QPushButton { "
							   "  color: white; "
							   "  border-style: solid; "
							   "  border-width: 1px; "
							   "  border-radius: 0px; "
							   "  border-color: black; "
							   "  background-color: grey; "
							   "  padding: 1px; "
							   "}"
							   "QPushButton:pressed { "
							   "  border-color: white; "
							   "}");
	connect(_titleButton, &QPushButton::clicked, this, &Rollout::toggleCollapsed);

	if(helpPage) {
		_helpButton = new QPushButton(QStringLiteral("?"), this);
		_helpButton->setAutoFillBackground(true);
		_helpButton->setFocusPolicy(Qt::NoFocus);
		_helpButton->setToolTip(tr("Open help topic"));
		_helpButton->setStyleSheet("QPushButton { "
								   "  color: white; "
								   "  border-style: solid; "
								   "  border-width: 1px; "
								   "  border-radius: 0px; "
								   "  border-color: black; "
								   "  background-color: rgb(80,130,80); "
								   "  padding: 1px; "
								   "  min-width: 16px; "
								   "}"
								   "QPushButton:pressed { "
								   "  border-color: white; "
								   "}");
		connect(_helpButton, &QPushButton::clicked, this, &Rollout::onHelpButton);
	}
	else _helpButton = nullptr;

	if(params._animateFirstOpening && !params._collapsed)
		setCollapsed(false);
}

/******************************************************************************
* Computes the recommended size for the widget.
******************************************************************************/
QSize Rollout::sizeHint() const
{
	QSize titleSize = _titleButton->sizeHint();
	QSize contentSize(0,0);
	if(_content)
		contentSize = _content->sizeHint();
	if(_useAvailableSpace) {
		int occupiedSpace = 0;
		for(Rollout* rollout : parentWidget()->findChildren<Rollout*>()) {
			if(rollout->_useAvailableSpace) continue;
			occupiedSpace += rollout->sizeHint().height();
		}
		occupiedSpace += parentWidget()->layout()->spacing() * (parentWidget()->findChildren<Rollout*>().size() - 1);
		int totalSpace = parentWidget()->parentWidget()->height();
		int availSpace = totalSpace - occupiedSpace;
		availSpace -= titleSize.height();
		if(availSpace > contentSize.height())
			contentSize.setHeight(availSpace);
	}
	contentSize.setHeight(contentSize.height() * _visiblePercentage / 100);
	return QSize(std::max(titleSize.width(), contentSize.width()), titleSize.height() + contentSize.height());
}

/******************************************************************************
* Handles the resize events of the rollout widget.
******************************************************************************/
void Rollout::resizeEvent(QResizeEvent* event)
{
	int titleHeight = _titleButton->sizeHint().height();
	int contentHeight = 0;
	if(_content)
		contentHeight = _content->sizeHint().height();
	if(_useAvailableSpace) {
		int occupiedSpace = 0;
		for(Rollout* rollout : parentWidget()->findChildren<Rollout*>()) {
			if(rollout->_useAvailableSpace) continue;
			occupiedSpace += rollout->sizeHint().height();
		}
		occupiedSpace += parentWidget()->layout()->spacing() * (parentWidget()->findChildren<Rollout*>().size() - 1);
		int totalSpace = parentWidget()->parentWidget()->height();
		int availSpace = totalSpace - occupiedSpace;
		availSpace -= titleHeight;
		if(availSpace > contentHeight)
			contentHeight = availSpace;
	}
	if(_helpButton) {
		int helpButtonWidth = titleHeight;
		_titleButton->setGeometry(0, 0, width() - helpButtonWidth + 1, titleHeight);
		_helpButton->setGeometry(width() - helpButtonWidth, 0, helpButtonWidth, titleHeight);
	}
	else _titleButton->setGeometry(0, 0, width(), titleHeight);
	if(_content) _content->setGeometry(0, height() - contentHeight, width(), contentHeight);
}

/******************************************************************************
* Paints the border around the rollout contents.
******************************************************************************/
void Rollout::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	int y = _titleButton->height() / 2;
	if(height()-y+1 > 0)
		qDrawShadeRect(&painter, 0, y, width()+1, height()-y+1, palette(), true);
}

/******************************************************************************
* Is called when the user presses the help button.
******************************************************************************/
void Rollout::onHelpButton()
{
	MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
	if(mainWindow)
		mainWindow->openHelpTopic(_helpPage);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
