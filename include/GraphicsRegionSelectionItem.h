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

#ifndef EMD_GRAPHICSREGIONSELECTIONITEM_H
#define EMD_GRAPHICSREGIONSELECTIONITEM_H

#include <qgraphicsitem.h>

namespace emd
{

class GraphicsRegionSelectionItem : public QGraphicsPolygonItem
{
public:
	GraphicsRegionSelectionItem(QGraphicsItem *parent = 0);

	void setHandleRadius(const float &radius);

	QGraphicsItem *handleItem() const;

	QGraphicsItem *takeHandleGhost(int &beforeIndex, int &afterIndex);

	// From QGraphicsPolygonItem
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
	virtual QPainterPath shape() const;

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *e);
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *e);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *e);

	//virtual void mousePressEvent(QGraphicsSceneMouseEvent *e);
	//virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
	//virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *e);

private:
	float m_handleRadius;
	QGraphicsItem *m_handleGhost;
	int m_ghostFirstIndex;
	int m_ghostSecondIndex;

	void updateGhostPosition(const QPointF &p);
	QPointF findGhostPoint(const QPointF &hoverPoint);
	QPointF closestPointOnLineSegment(const QPointF &v1, const QPointF &v2, const QPointF &p, float &distance);

};

} // namespace emd


#endif