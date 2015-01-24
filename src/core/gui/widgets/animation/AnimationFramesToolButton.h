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

#ifndef __OVITO_ANIMATION_FRAMES_TOOL_BUTTON_H
#define __OVITO_ANIMATION_FRAMES_TOOL_BUTTON_H

#include <core/Core.h>
#include <core/animation/AnimationSettings.h>
#include <core/dataset/DataSetContainer.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A combo-box widget that allows to select the current animation frame.
 */
class AnimationFramesToolButton : public QToolButton
{
	Q_OBJECT
	
public:
	
	/// Constructs the widget.
	AnimationFramesToolButton(DataSetContainer& datasetContainer, QWidget* parent = 0) : QToolButton(parent), _datasetContainer(datasetContainer) {
		setIcon(QIcon(QString(":/core/actions/animation/named_frames.png")));
		setToolTip(tr("Jump to animation frame"));
		setFocusPolicy(Qt::NoFocus);
		connect(this, &QToolButton::clicked, this, &AnimationFramesToolButton::onClicked);
	}

protected Q_SLOTS:

	void onClicked() {
		QMenu menu;

		AnimationSettings* animSettings = _datasetContainer.currentSet()->animationSettings();
		int currentFrame = animSettings->time() / animSettings->ticksPerFrame();
		for(auto entry = animSettings->namedFrames().cbegin(); entry != animSettings->namedFrames().cend(); ++entry) {
			QAction* action = menu.addAction(entry.value());
			action->setCheckable(true);
			action->setData(entry.key());
			if(currentFrame == entry.key()) {
				action->setChecked(true);
				menu.setActiveAction(action);
			}
		}
		if(animSettings->namedFrames().isEmpty()) {
			QAction* action = menu.addAction(tr("No animation frames loaded"));
			action->setEnabled(false);
		}

		connect(&menu, &QMenu::triggered, this, &AnimationFramesToolButton::onActionTriggered);
		menu.exec(mapToGlobal(QPoint(0, 0)));
	}

	void onActionTriggered(QAction* action) {
		if(action->data().isValid()) {
			int frameIndex = action->data().value<int>();
			AnimationSettings* animSettings = _datasetContainer.currentSet()->animationSettings();
			animSettings->setTime(animSettings->frameToTime(frameIndex));
		}
	}

private:

	DataSetContainer& _datasetContainer;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_ANIMATION_FRAMES_TOOL_BUTTON_H
