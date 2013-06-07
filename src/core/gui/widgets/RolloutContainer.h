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

/******************************************************************************
* Specifies how a new rollout is inserted into the container.
******************************************************************************/
class RolloutInsertionParameters
{
public:
	RolloutInsertionParameters() : collapsed(false), animateOpening(false), afterThisRollout(NULL), beforeThisRollout(NULL), intoThisContainer(NULL) {}
	RolloutInsertionParameters(bool _collapsed) : collapsed(_collapsed), animateOpening(false), afterThisRollout(NULL), beforeThisRollout(NULL), intoThisContainer(NULL) {}
	RolloutInsertionParameters(QWidget* _afterThisRollout, bool _collapsed = false) : collapsed(_collapsed), animateOpening(false), afterThisRollout(_afterThisRollout), beforeThisRollout(NULL), intoThisContainer(NULL) {}

	RolloutInsertionParameters after(QWidget* afterThisRollout) const { 
		RolloutInsertionParameters p(collapsed);
		p.afterThisRollout = afterThisRollout;
		return p;
	}
	RolloutInsertionParameters before(QWidget* beforeThisRollout) const { 
		RolloutInsertionParameters p(collapsed);
		p.beforeThisRollout = beforeThisRollout;
		return p;
	}
	RolloutInsertionParameters collapse() const {
		RolloutInsertionParameters p(*this);
		p.collapsed = true;
		return p;
	}
	RolloutInsertionParameters animate() const {
		RolloutInsertionParameters p(*this);
		p.animateOpening = true;
		return p;
	}
	RolloutInsertionParameters insertInto(QWidget* intoThisContainer) const {
		RolloutInsertionParameters p;
		p.intoThisContainer = intoThisContainer;
		return p;
	}
	
public:
	
	bool collapsed;
	bool animateOpening;
	QPointer<QWidget> afterThisRollout;
	QPointer<QWidget> beforeThisRollout;
	QPointer<QWidget> intoThisContainer;
};

/******************************************************************************
* A minimizable rollout window that encapsulates a control in 
* the RolloutContainer component.
******************************************************************************/
class Rollout : public QWidget
{
	friend class RolloutContainer;

	Q_OBJECT

private:
	
	/// Constructor.
	Rollout(QWidget* parent, QWidget* content, const QString& title, const RolloutInsertionParameters& params);
	
	QPushButton* _titleButton;
	QPointer<QWidget> _content;

protected Q_SLOTS:
	
	void onCollapseButton();
	void onContentDestroyed();

public:

	/// Returns true if this rollout is currently collapsed.
	bool isCollapsed() const;

	/// Callapses this rollout.
	void setCollapsed(bool collapsed);

	/// Returns the child widget that is contained in the rollout.
	QWidget* content() const { return _content; }
	
protected:

	/// Paints the border around the rollout.
	virtual void paintEvent(QPaintEvent* event);
	
};

/******************************************************************************
* This container manages multiple rollouts.
******************************************************************************/
class RolloutContainer : public QScrollArea
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
};

};

#endif // __OVITO_ROLLOUT_CONTAINER_H
