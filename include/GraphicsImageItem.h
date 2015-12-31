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

#ifndef EMD_GRAPHICSIMAGEITEM_H
#define EMD_GRAPHICSIMAGEITEM_H

#include <QGraphicsRectItem>
#include <qobject.h>

#include "PointCloud.h"

namespace emd
{

class GraphicsImageItem : public QObject, public QGraphicsRectItem
{
	Q_OBJECT

public:
	GraphicsImageItem();

	void setScale(const float &xScale, const float &yScale);

	void displayPointCloud(PointCloud *pointCloud);

	// From QGraphicsItem
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *e);
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *e);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *e);

private:
	float m_xScale;
	float m_yScale;

	PointCloud m_currentPointCloud;

signals:
	void cursorPositionChanged(float x, float y);
};

} // namespace emd


#endif