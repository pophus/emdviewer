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

#ifndef EMD_MAINIMAGESCENE_H
#define EMD_MAINIMAGESCENE_H

#include <qgraphicsscene.h>

namespace emd
{

class GraphicsRegionSelectionItem;

class MainImageScene : public QGraphicsScene
{
	Q_OBJECT
public:
	MainImageScene(QObject *parent = 0);

	// Accessors
	void setImageItem(QGraphicsItem *item);
	void setImageRect(const QRect &rect);

	void setScale(const float &xScale, const float &yScale);

	void setRegionSelection(const QPolygonF &selection);
	QPolygonF regionSelection() const;
	void setRegionSelectionEnabled(bool enabled);

protected:
	// Mouse functions
	void mousePressEvent(QGraphicsSceneMouseEvent *e);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
	void wheelEvent(QGraphicsSceneWheelEvent *e);
	
private:
	QGraphicsItem *m_imageItem;
	QRect m_imageRect;
	bool m_panning;
	QPoint m_mousePos;

	float m_xScale;
	float m_yScale;

	GraphicsRegionSelectionItem *m_regionSelectionItem;
	QPolygonF m_regionSelection;
	QPolygonF m_scaledRegionSelection;
	QVector<QGraphicsItem*> m_regionHandles;

	QGraphicsItem* m_draggedHandle;
	int m_draggedHandleIndex;

	void updateScaledRegionSelection();
	void updateRegionSelectionItems();

signals:
	void panningChanged(bool state);
	void imagePanned(QPoint delta);
	void regionSelectionChanged();
	void cursorPositionChanged(float x, float y);
};

} // namespace emd

#endif