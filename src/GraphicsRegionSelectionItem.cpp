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

#include "GraphicsRegionSelectionItem.h"

#include <cmath>
#include <limits>

#include <qdebug.h>
#include <QGraphicsSceneHoverEvent>
#include <qpainter.h>

namespace emd
{

float dotProduct(const QPointF &p1, const QPointF &p2)
{
	return p1.x() * p2.x() + p1.y() * p2.y();
}

float pointDistance(const QPointF &p1, const QPointF &p2)
{
	float dx = p2.x() - p1.x();
	float dy = p2.y() - p1.y();
	return sqrtf(dx * dx + dy * dy);
}

float pointMagnitude(const QPointF &p)
{
	return sqrtf(p.x() * p.x() + p.y() * p.y());
}

GraphicsRegionSelectionItem::GraphicsRegionSelectionItem(QGraphicsItem *parent)
	: QGraphicsPolygonItem(parent),
	m_handleGhost(NULL)
{
	this->setAcceptedMouseButtons(0);
	this->setAcceptHoverEvents(true);
}

void GraphicsRegionSelectionItem::setHandleRadius(const float &radius)
{
	m_handleRadius = radius;
}

QGraphicsItem *GraphicsRegionSelectionItem::handleItem() const
{
	QRadialGradient handleGradient(0, 0, m_handleRadius, m_handleRadius / 2, -m_handleRadius / 2);
	handleGradient.setColorAt(0, QColor("white"));
	handleGradient.setColorAt(0.8, QColor(200, 220, 255));
	handleGradient.setColorAt(1, QColor("black"));

	QBrush handleBrush(handleGradient);

	QGraphicsEllipseItem *handleItem = new QGraphicsEllipseItem(-m_handleRadius, -m_handleRadius, 2 * m_handleRadius, 2 * m_handleRadius);
	handleItem->setPen(Qt::NoPen);
	handleItem->setBrush(handleBrush);
	handleItem->setAcceptedMouseButtons(0);

	return handleItem;
}

QGraphicsItem *GraphicsRegionSelectionItem::takeHandleGhost(int &beforeIndex, int &afterIndex)
{
	if(!m_handleGhost || !m_handleGhost->isVisible())
		return NULL;

	beforeIndex = m_ghostFirstIndex;
	afterIndex = m_ghostSecondIndex;

	QGraphicsItem *ghost = m_handleGhost;
	m_handleGhost = NULL;
	return ghost;
}

// Reimplemented from QGraphicsPolygonItem
void GraphicsRegionSelectionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	painter->setCompositionMode(QPainter::RasterOp_SourceAndNotDestination);
	painter->setPen(this->pen());
	painter->drawPolygon(this->polygon());
}

QPainterPath GraphicsRegionSelectionItem::shape() const
{
	QPolygonF polygon = this->polygon();
	QPainterPath basePath = QGraphicsPolygonItem::shape();
	basePath.moveTo(polygon.at(polygon.count() - 1));
	basePath.lineTo(polygon.at(0));

	QPainterPathStroker stroker;
	stroker.setWidth(2 * m_handleRadius);

	return stroker.createStroke(basePath);
}

// Mouse events
void GraphicsRegionSelectionItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{

}

void GraphicsRegionSelectionItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
	if(m_handleGhost)
		m_handleGhost->setVisible(false);
}

void GraphicsRegionSelectionItem::hoverMoveEvent(QGraphicsSceneHoverEvent *e)
{
	if(!m_handleGhost)
	{
		m_handleGhost = handleItem();
		m_handleGhost->setZValue(10);
		m_handleGhost->setParentItem(this->parentItem());
	}

	updateGhostPosition(e->pos());

	if(e->modifiers() & Qt::ALT)
		m_handleGhost->setVisible(false);
}

//void GraphicsRegionSelectionItem::mousePressEvent(QGraphicsSceneMouseEvent *e)
//{
//
//}
//
//void GraphicsRegionSelectionItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
//{
//
//}
//
//void GraphicsRegionSelectionItem::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
//{
//
//}

// Utility
void GraphicsRegionSelectionItem::updateGhostPosition(const QPointF &p)
{
	if(!m_handleGhost)
		return;

	QPointF ghostPos = findGhostPoint(p);

	if(ghostPos.x() != std::numeric_limits<float>::max())
	{
		m_handleGhost->setPos(ghostPos);
		m_handleGhost->setVisible(true);
	}
	else
		m_handleGhost->setVisible(false);
}

QPointF GraphicsRegionSelectionItem::findGhostPoint(const QPointF &hoverPoint)
{
	int first = 0;
	int second = 1;

	QPolygonF polygon = this->polygon();

	QPointF ghostPoint;
	float minDistance = std::numeric_limits<float>::max();

	while(first < polygon.count())
	{
		float distance;
		QPointF point = closestPointOnLineSegment(polygon.at(first), polygon.at(second), hoverPoint, distance);

		if(distance < minDistance)
		{
			minDistance = distance;
			ghostPoint = point;
			m_ghostFirstIndex = first;
			m_ghostSecondIndex = second;
		}

		++first;
		++second;
		if(second == polygon.count())
			second = 0;
	}

	// We allow a bit of wiggle room because the shape used for hover events 
	// is slightly larger than expected.
	if(minDistance < m_handleRadius + 2)
	{
		// Make sure the point is not too close to a handle
		foreach(QPointF point, polygon)
		{
			if(pointDistance(point, ghostPoint) < 2 * m_handleRadius)
				return QPointF(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		}

		return ghostPoint;
	}

	return QPointF(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
}

QPointF GraphicsRegionSelectionItem::closestPointOnLineSegment(const QPointF &v1, const QPointF &v2, const QPointF &p, float &distance)
{
	const QPointF segment = v2 - v1;
	const float segmentLengthSquared = segment.x() * segment.x() + segment.y() * segment.y();

	if(segmentLengthSquared == 0.f)
	{
		// v1 and v2 coincide
		distance = pointDistance(v1, p);
		return v1;
	}

	// Consider the line extending the segment, parameterized as v1 + t (v2 - v1).
	// We find projection of point p onto the line. 
	// It falls where t = [(p-v1) . (v2-v1)] / |v2-v1|^2
	const float t = dotProduct(p - v1, segment) / segmentLengthSquared;

	if(t < 0.0)
	{
		distance = pointDistance(v1, p);
		return v1;       // Beyond the 'v1' end of the segment
	}
	else if(t > 1.0) 
	{
		return v2;  // Beyond the 'v2' end of the segment
	}

	const QPointF projection = v1 + t * segment;  // Projection falls on the segment

	distance = pointDistance(projection, p);

	return projection;
}

} // namespace emd
