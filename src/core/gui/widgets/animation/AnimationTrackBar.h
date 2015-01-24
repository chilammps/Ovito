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

#ifndef __OVITO_ANIMATION_TRACK_BAR_H
#define __OVITO_ANIMATION_TRACK_BAR_H

#include <core/Core.h>
#include <core/reference/RefTargetListener.h>
#include <core/animation/controller/KeyframeController.h>
#include "AnimationTimeSlider.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * The track bar displays the animation keys of the selected scene node.
 */
class AnimationTrackBar : public QFrame
{
	Q_OBJECT

public:

	/// Constructor.
	AnimationTrackBar(MainWindow* mainWindow, AnimationTimeSlider* timeSlider, QWidget* parentWindow = nullptr);

protected:

	/// Handles paint events.
	virtual void paintEvent(QPaintEvent* event) override;
	
	/// Returns the recommended size of the widget.
	virtual QSize sizeHint() const override;

	/// Returns the minimum size of the widget.
	virtual QSize minimumSizeHint() const override { return sizeHint(); }
	
	/// Handles mouse press events.
	virtual void mousePressEvent(QMouseEvent * event) override;

	/// Handles mouse move events.
	virtual void mouseMoveEvent(QMouseEvent * event) override;

	/// Handles mouse release events.
	virtual void mouseReleaseEvent(QMouseEvent * event) override;

	/// Recursive function that finds all controllers in the object graph.
	void findControllers(RefTarget* target);

	/// Paints the symbol for a single animation key.
	void paintKey(QPainter& painter, AnimationKey* key, KeyframeController* ctrl) const;

	/// Computes the display rectangle of an animation key.
	QRect keyRect(AnimationKey* key, bool forDisplay) const;

	/// Finds all keys under the mouse cursor.
	QVector<AnimationKey*> hitTestKeys(QPoint pos) const;

	/// Returns the index of the controller that owns the given key.
	int controllerIndexFromKey(AnimationKey* key) const;

	/// Returns a text representation of a key's value.
	QString keyValueString(AnimationKey* key) const;

	/// Checks if the given ref target is a controller, and, if yes, add it to our list of controllers.
	void addController(RefTarget* target, RefTarget* owner, const PropertyFieldDescriptor* field);

	/// Displays the context menu.
	void showKeyContextMenu(const QPoint& pos, const QVector<AnimationKey*>& clickedKeys);

protected Q_SLOTS:

	/// This is called when new animation settings have been loaded.
	void onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings);

	/// This is called when the current scene node selection has changed.
	void onRebuildControllerList();

	/// Is called whenever one of the objects being monitored sends a notification signal.
	void onObjectNotificationEvent(RefTarget* source, ReferenceEvent* event);

	/// Is called whenever one of the controller being monitored sends a notification signal.
	void onControllerNotificationEvent(RefTarget* source, ReferenceEvent* event);

	/// Deletes the selected animation keys.
	void onDeleteSelectedKeys();

private:

	/// Pointer to the animation time slider widget.
	AnimationTimeSlider* _timeSlider;

	/// The current animation settings object.
	AnimationSettings* _animSettings;

	/// This list of animation controllers that are shown in the track bar.
	VectorRefTargetListener<KeyframeController> _controllers;

	/// List of all reference targets in the selected object reference tree.
	VectorRefTargetListener<RefTarget> _objects;

	/// This list of selected animation keys.
	VectorRefTargetListener<AnimationKey> _selectedKeys;

	/// The names of the animated parameters that are controlled by the keyframe controllers.
	QStringList _parameterNames;

	/// The brushes used to paint animation keys.
	std::array<QBrush,7> _keyBrushes;

	/// The pen used to paint animation keys.
	QPen _keyPen;

	/// The pen used to paint selected animation keys.
	QPen _selectedKeyPen;

	/// The cursor to show when the mouse is over a key.
	QCursor _selectionCursor;

	/// The mouse position when starting a key dragging operation.
	int _dragStartPos;

	/// Indicates that a drag operation is in progress.
	bool _isDragging;

	QMetaObject::Connection _animIntervalChangedConnection;
	QMetaObject::Connection _timeFormatChangedConnection;
	QMetaObject::Connection _timeChangedConnection;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_ANIMATION_TRACK_BAR_H
