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
#include <core/gui/widgets/RolloutContainer.h>

namespace Ovito {

class RolloutContainerLayout : public QLayout
{
public:
	RolloutContainerLayout(QWidget *parent, int margin = 2, int spacing = 2);
	~RolloutContainerLayout();
	virtual void addItem(QLayoutItem *item);
	void insertWidgetAfter(Rollout* afterThis, QWidget* newWidget);
	void insertWidgetBefore(Rollout* beforeThis, QWidget* newWidget);
	virtual int count() const;
	virtual QLayoutItem *itemAt(int index) const;
	virtual void setGeometry(const QRect &rect);
	virtual QSize sizeHint() const;
	virtual QLayoutItem* takeAt(int index);
	virtual QSize minimumSize() const { return sizeHint(); }

private:
	QList<QLayoutItem*> itemList;
};

class RolloutLayout : public QLayout
{
public:
	RolloutLayout(QWidget* parent, bool collapsed) : QLayout(parent), titleButton(NULL), content(NULL), isCollapsed(collapsed), collapseAnimationTimer(0) {
		setSpacing(0);
		setContentsMargins(0,0,0,0);
		collapseAnimationState = isCollapsed ? 0 : 100;
	}

	QSize sizeHint() const {
		QSize btnSize(0,0), contentSize(0,0);
		if(titleButton) btnSize = titleButton->sizeHint();
		if(content && !content->isEmpty()) contentSize = content->sizeHint();
		contentSize.setHeight(contentSize.height() * collapseAnimationState / 100);
		return QSize(std::max(contentSize.width(), btnSize.width()), btnSize.height() + contentSize.height());
	}

	QSize minimumSize() const {
		QSize btnSize(0,0), contentSize(0,0);
		if(titleButton) btnSize = titleButton->minimumSize();
		if(content && !content->isEmpty()) contentSize = content->minimumSize();
		contentSize.setHeight(contentSize.height() * collapseAnimationState / 100);
		return QSize(contentSize.width(), btnSize.height() + contentSize.height());
	}

	QSize maximumSize() const {
		QSize btnSize(0,0), contentSize(0,0);
		if(titleButton) btnSize = titleButton->maximumSize();
		if(content && !content->isEmpty()) contentSize = content->maximumSize();
		return QSize(std::min(contentSize.width(), btnSize.width()), btnSize.height() + contentSize.height());
	}

    QLayoutItem* itemAt(int i) const {
    	if(i == 0) return titleButton;
    	else if(i == 1) return content;
    	return NULL;
    }

    QLayoutItem* takeAt(int i) {
    	QLayoutItem* item = NULL;
    	if(i == 0) { item = titleButton; titleButton = NULL; }
    	else if(i == 1) { item = content; content = NULL; }
    	invalidate();
    	return item;
    }

    int count() const {
    	if(titleButton != NULL) {
    		if(content != NULL) return 2;
    		else return 1;
    	}
    	return 0;
    }

    void setGeometry(const QRect& rect) {
    	int y = rect.top();
		if(titleButton) {
			QSize btnSize = titleButton->sizeHint();
			titleButton->setGeometry(QRect(rect.x(), y, rect.width(), btnSize.height()));
			y = titleButton->geometry().bottom();
		}
		if(content && !content->isEmpty()) {
			if(collapseAnimationState == 100)
				content->setGeometry(QRect(rect.x(), y, rect.width(), rect.bottom()-y));
			else {
				QSize contentSize = content->sizeHint();
				content->setGeometry(QRect(rect.x(), rect.bottom() - contentSize.height(), rect.width(), contentSize.height()));
			}
		}
    }

    void addItem(QLayoutItem* item) {
    	if(titleButton == NULL) titleButton = item;
    	else content = item;
    	invalidate();
    }

    void addLayout(QLayout* layout) {
    	if(titleButton == NULL) titleButton = layout;
    	else content = layout;
    	addChildLayout(layout);
    	invalidate();
    }

    void setCollapsed(bool c) {
    	if(c == isCollapsed) return;
    	isCollapsed = c;
    	if(!content) return;
    	content->widget()->setVisible(true);
    	update();
    	if(!collapseAnimationTimer)
    		collapseAnimationTimer = startTimer(20);
    }

    void timerEvent(QTimerEvent* event) {
    	if(isCollapsed && collapseAnimationState > 0) {
    		if(collapseAnimationState < 10)
    			collapseAnimationState = std::max(0, collapseAnimationState-2);
    		else
    			collapseAnimationState = collapseAnimationState * 2 / 3;
    	}
    	else if(!isCollapsed && collapseAnimationState < 100) {
    		if(collapseAnimationState > 90)
    			collapseAnimationState = std::min(100, collapseAnimationState+2);
    		else
    			collapseAnimationState = 100 - (100 - collapseAnimationState) * 2 / 3;
    	}
    	else {
    		OVITO_ASSERT(collapseAnimationTimer == collapseAnimationTimer);
    		killTimer(event->timerId());
    		collapseAnimationTimer = 0;
    		return;
    	}
    	update();
    }

    QLayoutItem* titleButton;
    QLayoutItem* content;
	int collapseAnimationState;
	int collapseAnimationTimer;
	bool isCollapsed;
};

/******************************************************************************
* Constructs the container.
******************************************************************************/
RolloutContainer::RolloutContainer(QWidget* parent) : QScrollArea(parent)
{
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	setWidgetResizable(true);
	QWidget* widget = new QWidget();
	widget->setLayout(new RolloutContainerLayout(widget));
	widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	setWidget(widget);
}

/******************************************************************************
* Inserts a new rollout into the container.
*		collapsed - Controls whether the new rollout should set to collapsed state.
******************************************************************************/
Rollout* RolloutContainer::addRollout(QWidget* content, const QString& title, const RolloutInsertionParameters& params)
{
	OVITO_CHECK_POINTER(content);
	Rollout* rollout = new Rollout(widget(), content, title, params);
	if(params.afterThisRollout != NULL) {
		Rollout* otherRollout = qobject_cast<Rollout*>(params.afterThisRollout->parent());
		static_cast<RolloutContainerLayout*>(widget()->layout())->insertWidgetAfter(otherRollout, rollout);
	}
	else if(params.beforeThisRollout != NULL) {
		Rollout* otherRollout = qobject_cast<Rollout*>(params.beforeThisRollout->parent());
		static_cast<RolloutContainerLayout*>(widget()->layout())->insertWidgetBefore(otherRollout, rollout);
	}
	else widget()->layout()->addWidget(rollout);
	return rollout;
}

/// Constructor.
Rollout::Rollout(QWidget* parent, QWidget* _content, const QString& title, const RolloutInsertionParameters& params) :
	QWidget(parent), _content(_content)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
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

	_content->setParent(this);
	_content->setVisible(!params.animateOpening && !params.collapsed);
	connect(_titleButton, SIGNAL(clicked(bool)), this, SLOT(onCollapseButton()));
	connect(_content, SIGNAL(destroyed(QObject*)), this, SLOT(onContentDestroyed()));
	_content->stackUnder(_titleButton);

	RolloutLayout* layout = new RolloutLayout(this, params.collapsed || params.animateOpening);
	layout->addWidget(_titleButton);
	layout->addWidget(_content);

	if(params.animateOpening && !params.collapsed)
		setCollapsed(false);
}

/// Paints the border around the rollout.
void Rollout::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	int y = _titleButton->height() / 2;
	qDrawShadeRect(&painter, 0, y, width()+1, height()-y+1, palette(), true);
}

/// Callapses this rollout.
void Rollout::setCollapsed(bool collapsed)
{
	((RolloutLayout*)layout())->setCollapsed(collapsed);
}

bool Rollout::isCollapsed() const
{
	return ((RolloutLayout*)layout())->isCollapsed;
}

void Rollout::onCollapseButton()
{
	setCollapsed(!isCollapsed());
}

void Rollout::onContentDestroyed()
{
	deleteLater();
	updateGeometry();
}

RolloutContainerLayout::RolloutContainerLayout(QWidget *parent, int margin, int spacing)
	: QLayout(parent)
{
	setSpacing(spacing);
	setContentsMargins(margin, margin, margin, margin);
}

RolloutContainerLayout::~RolloutContainerLayout()
{
	QLayoutItem *item;
	while ((item = takeAt(0)))
		delete item;
}

void RolloutContainerLayout::addItem(QLayoutItem *item)
{
	itemList.append(item);
}

void RolloutContainerLayout::insertWidgetAfter(Rollout* afterThis, QWidget* newWidget)
{
	addChildWidget(newWidget);
	for(int i=0; i<itemList.size(); i++) {
		if(itemList[i]->widget() == afterThis) {
			itemList.insert(i + 1, new QWidgetItem(newWidget));
			return;
		}
	}
	itemList.append(new QWidgetItem(newWidget));
}

void RolloutContainerLayout::insertWidgetBefore(Rollout* beforeThis, QWidget* newWidget)
{
	addChildWidget(newWidget);
	for(int i=0; i<itemList.size(); i++) {
		if(itemList[i]->widget() == beforeThis) {
			itemList.insert(i, new QWidgetItem(newWidget));
			return;
		}
	}
	itemList.append(new QWidgetItem(newWidget));
}

int RolloutContainerLayout::count() const
{
	return itemList.size();
}

QLayoutItem* RolloutContainerLayout::itemAt(int index) const
{
	return itemList.value(index);
}

QLayoutItem* RolloutContainerLayout::takeAt(int index)
{
	if (index >= 0 && index < itemList.size())
		return itemList.takeAt(index);
	else
		return NULL;
}

void RolloutContainerLayout::setGeometry(const QRect &rect)
{
	QLayout::setGeometry(rect);
	int left, right, top, bottom;
	getContentsMargins(&left, &top, &right, &bottom);
	QRect r = rect.adjusted(left, top, -right, -bottom);
	int y = r.y();

	QLayoutItem *item;
	Q_FOREACH(item, itemList) {
		int height = item->sizeHint().height();
		item->setGeometry(QRect(r.x(), y, r.width(), height));
		y += height;
		y += spacing();
	}
}

QSize RolloutContainerLayout::sizeHint() const
{
	QSize size(0,spacing() * itemList.size());
	QLayoutItem *item;
	Q_FOREACH(item, itemList) {
		size += item->sizeHint();
	}

	int left, right, top, bottom;
	getContentsMargins(&left, &top, &right, &bottom);

	size.setWidth(0);
	size += QSize(left+right, top+bottom);
	return size;
}

};
