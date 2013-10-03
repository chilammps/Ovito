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

#ifndef __OVITO_ROLLOUT_CONTAINER_H
#define __OVITO_ROLLOUT_CONTAINER_H

#include <core/Core.h>

namespace Ovito {

class RolloutContainer;		// defined below

/**
 * This data structure specifies how and where a new rollout is inserted into a RolloutContainer.
 */
class RolloutInsertionParameters
{
public:

	/// Default constructor that sets all parameters to default values.
	RolloutInsertionParameters() :
		_collapsed(false),
		_animateFirstOpening(false),
		_useAvailableSpace(false),
		_afterThisRollout(nullptr),
		_beforeThisRollout(nullptr),
		_intoThisContainer(nullptr) {}

	RolloutInsertionParameters after(QWidget* afterThisRollout) const { 
		RolloutInsertionParameters p;
		p._collapsed = this->_collapsed;
		p._useAvailableSpace = this->_useAvailableSpace;
		p._intoThisContainer = this->_intoThisContainer;
		p._afterThisRollout = afterThisRollout;
		return p;
	}

	RolloutInsertionParameters before(QWidget* beforeThisRollout) const { 
		RolloutInsertionParameters p;
		p._collapsed = this->_collapsed;
		p._useAvailableSpace = this->_useAvailableSpace;
		p._intoThisContainer = this->_intoThisContainer;
		p._beforeThisRollout = beforeThisRollout;
		return p;
	}

	RolloutInsertionParameters collapse() const {
		RolloutInsertionParameters p(*this);
		p._collapsed = true;
		return p;
	}

	RolloutInsertionParameters useAvailableSpace() const {
		RolloutInsertionParameters p(*this);
		p._useAvailableSpace = true;
		return p;
	}

	RolloutInsertionParameters animate() const {
		RolloutInsertionParameters p(*this);
		p._animateFirstOpening = true;
		return p;
	}

	RolloutInsertionParameters insertInto(QWidget* intoThisContainer) const {
		RolloutInsertionParameters p;
		p._intoThisContainer = intoThisContainer;
		return p;
	}

	RolloutInsertionParameters setTitle(const QString& title) const {
		RolloutInsertionParameters p(*this);
		p._title = title;
		return p;
	}
	
	/// Returns the container set by insertInto() into which the properties editor should inserted.
	QWidget* container() const {
		return _intoThisContainer;
	}

	const QString& title() const { return _title; }

private:
	
	bool _collapsed;
	bool _animateFirstOpening;
	bool _useAvailableSpace;
	QPointer<QWidget> _afterThisRollout;
	QPointer<QWidget> _beforeThisRollout;
	QPointer<QWidget> _intoThisContainer;
	QString _title;

	friend class Rollout;
	friend class RolloutContainer;
};

/******************************************************************************
* A rollout widget in a RolloutContainer.
* This is part of the implementation of the RolloutContainer and
* should not be used outside of a RolloutContainer.
******************************************************************************/
class OVITO_CORE_EXPORT Rollout : public QWidget
{
	Q_OBJECT

public:

	/// Constructor.
	Rollout(QWidget* parent, QWidget* content, const QString& title, const RolloutInsertionParameters& params);

	/// Returns true if this rollout is currently in the collapsed state.
	bool isCollapsed() const { return visiblePercentage() != 100; }

	/// Returns the child widget that is contained in the rollout.
	QWidget* content() const { return _content; }

	/// Returns how much of rollout contents is visible.
	int visiblePercentage() const { return _visiblePercentage; }

	/// Sets how much of rollout contents is visible.
	void setVisiblePercentage(int p) {
		_visiblePercentage = p;
		updateGeometry();
	}
	
	/// Computes the recommended size for the widget.
	virtual QSize sizeHint() const override;

	Q_PROPERTY(int visiblePercentage READ visiblePercentage WRITE setVisiblePercentage);

public Q_SLOTS:

	/// Opens the rollout if it is collapsed; or collapses it if it is open.
	void toggleCollapsed() { setCollapsed(!isCollapsed()); }

	/// Collapses or opens the rollout.
	void setCollapsed(bool collapsed) {
		_collapseAnimation.stop();
		_collapseAnimation.setStartValue(_visiblePercentage);
		_collapseAnimation.setEndValue(collapsed ? 0 : 100);
		_collapseAnimation.start();
	}

	/// Changes the title of the rollout.
	void setTitle(const QString& title) {
		_titleButton->setText(title);
	}

protected:

	/// Handles the resize events of the rollout widget.
	virtual void resizeEvent(QResizeEvent* event) override;

	/// Paints the border around the contents widget.
	virtual void paintEvent(QPaintEvent* event) override;
	
private:

	/// The button that allows to collapse the rollout.
	QPushButton* _titleButton;

	/// The widget that is inside the rollout.
	QPointer<QWidget> _content;

	/// Internal property that controls how much of rollout contents is visible.
	int _visiblePercentage;

	/// The object that animates the collapse/opening of the rollout.
	QPropertyAnimation _collapseAnimation;

	/// Indicates that this rollout should automatically expand to use all available space in the container.
	bool _useAvailableSpace;
};

/******************************************************************************
* This container manages multiple rollouts.
******************************************************************************/
class OVITO_CORE_EXPORT RolloutContainer : public QScrollArea
{
	Q_OBJECT

public:

	/// Constructs the rollout container.
	RolloutContainer(QWidget* parent = 0);

	/// Adds a new rollout to the container.
	Rollout* addRollout(QWidget* content, const QString& title, const RolloutInsertionParameters& param = RolloutInsertionParameters());
	
	virtual QSize minimumSizeHint() const { 
		return QSize(QFrame::minimumSizeHint().width(), 10); 
	}

protected:

	/// Handles the resize events of the rollout container widget.
	virtual void resizeEvent(QResizeEvent* event) override {
		QScrollArea::resizeEvent(event);
		updateRollouts();
	}

public Q_SLOTS:

	/// Updates the size of all rollouts.
	void updateRollouts();

	/// Updates the size of all rollouts soon.
	void updateRolloutsLater() {
		QTimer::singleShot(0, this, SLOT(updateRollouts()));
	}

};

};

#endif // __OVITO_ROLLOUT_CONTAINER_H
