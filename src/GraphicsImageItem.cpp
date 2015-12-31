/*
 * emdViewer, a program for working with electron microscopy dataset 
 * (emd) files.
 * Copyright (C) 2015  Phil Ophus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GraphicsImageItem.h"

#include <qdebug.h>
#include <qgraphicssceneevent.h>
#include <qpainter.h>

namespace emd
{

GraphicsImageItem::GraphicsImageItem()
	: QGraphicsRectItem(),
	m_xScale(1.f),
	m_yScale(1.f)
{
	this->setAcceptHoverEvents(true);
}

void GraphicsImageItem::setScale(const float &xScale, const float &yScale)
{
	if(xScale > 0)
		m_xScale = xScale;

	if(yScale > 0)
		m_yScale = yScale;
}

void GraphicsImageItem::displayPointCloud(PointCloud *pointCloud)
{
	m_currentPointCloud = *pointCloud;

	update();
}

void GraphicsImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QGraphicsRectItem::paint(painter, option, widget);

	if(m_currentPointCloud.count() > 0)
	{
		QPen pointPen(QBrush(QColor("black")), 2);
		painter->setPen(pointPen);
		painter->drawPoints(m_currentPointCloud.data(), m_currentPointCloud.count());
	}
}

/*** Mouse Events ***/

void GraphicsImageItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{

}

void GraphicsImageItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
	emit(cursorPositionChanged(-1, -1));
}

void GraphicsImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent *e)
{
	QPointF pos = e->pos();

	emit(cursorPositionChanged((pos.x() / m_xScale), (pos.y() / m_yScale)));
}

} // namespace emd

