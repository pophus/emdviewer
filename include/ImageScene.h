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

#ifndef EMD_IMAGESCENE_H
#define EMD_IMAGESCENE_H

#include <qgraphicsscene.h>

namespace emd
{

class ImageScene : public QGraphicsScene
{
	Q_OBJECT
public:
	ImageScene(QObject *parent = 0);

	// Accessors
	void addItem(QGraphicsItem *item);

	// Mouse functions
	void mousePressEvent(QGraphicsSceneMouseEvent *e);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
	void wheelEvent(QGraphicsSceneWheelEvent *e);
	
private:
	QGraphicsItem *m_item;
	bool m_dragging;
	QPoint m_mousePos;

signals:
	void panningStateChanged(bool);
	void imagePanned(float, float);
};

} // namespace emd

#endif