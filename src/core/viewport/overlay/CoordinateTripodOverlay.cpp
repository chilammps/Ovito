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
#include <core/viewport/Viewport.h>
#include "CoordinateTripodOverlay.h"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, CoordinateTripodOverlay, ViewportOverlay);

/******************************************************************************
* Constructor.
******************************************************************************/
CoordinateTripodOverlay::CoordinateTripodOverlay(DataSet* dataset) : ViewportOverlay(dataset)
{
}

/******************************************************************************
* This method asks the overlay to paint its contents over the given viewport.
******************************************************************************/
void CoordinateTripodOverlay::render(Viewport* viewport, QPainter& painter, const ViewProjectionParameters& projParams, RenderSettings* renderSettings)
{
	static const QColor axisColors[3] = { QColor(255, 0, 0), QColor(0, 255, 0), QColor(50, 50, 255) };
	static const QString labels[3] = { QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("z") };

	//	painter.drawText(10, 10, QString("Hello World!"));
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);
	qreal lineWidth = 4;
	FloatType tripodSize = 40.0f;
	FloatType arrowSize = 0.17f;
	QPointF origin(tripodSize, tripodSize);
	// Order axes from back to front.
	std::array<int,3> orderedAxes{0,1,2};
	std::sort(orderedAxes.begin(), orderedAxes.end(), [&projParams](int a, int b) {
		return projParams.viewMatrix.column(a).z() < projParams.viewMatrix.column(b).z();
	});
	for(int axis : orderedAxes) {
		QPen pen(axisColors[axis]);
		pen.setWidthF(lineWidth);
		pen.setJoinStyle(Qt::MiterJoin);
		pen.setCapStyle(Qt::RoundCap);
		painter.setPen(pen);
		Vector3 dir = projParams.viewMatrix.column(axis).resized(tripodSize);
		painter.drawLine(origin, origin + QPointF(dir.x(), -dir.y()));
		QPointF head[3];
		head[1] = origin + QPointF(dir.x(), -dir.y());
		head[0] = head[1] + QPointF(arrowSize * (dir.y() - dir.x()), -arrowSize * (-dir.x() - dir.y()));
		head[2] = head[1] + QPointF(arrowSize * (-dir.y() - dir.x()), -arrowSize * (dir.x() - dir.y()));
		painter.drawPolyline(head, 3);
	}
	//painter.setBrush(Qt::SolidPattern);
	//painter.setPen(Qt::NoPen);
	//painter.drawEllipse(origin, 0.5*lineWidth, 0.5*lineWidth);
}

};
