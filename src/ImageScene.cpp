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

#include "ImageScene.h"

#include <qgraphicssceneevent.h>

namespace emd
{

ImageScene::ImageScene(QObject *parent)
	: QGraphicsScene(parent)
{
	// Init
	m_dragging = false;

	// Setup
}

void ImageScene::addItem(QGraphicsItem *item)
{
	m_item = item;
	QGraphicsScene::addItem(item);
}

/********************** Mouse Events **************************/

void ImageScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
	if(e->button() == Qt::LeftButton)
	{
		m_dragging = true;
		emit(panningStateChanged(true));
	}

	e->accept();
}

void ImageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
	if(e->button() == Qt::LeftButton)
	{
		m_dragging = false;
		emit(panningStateChanged(false));
	}

	e->accept();
}

void ImageScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
	if(m_dragging)
	{
		QPoint delta = e->screenPos() - e->lastScreenPos();
		emit(imagePanned(delta.x(), delta.y()));
	}

	e->accept();
}

void ImageScene::wheelEvent(QGraphicsSceneWheelEvent *e)
{

	e->accept();
}

} // namespace emd
