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

#include "MainImageScene.h"

#include "qgraphicssceneevent.h"
#include <QGraphicsItem>
#include <qdebug.h>

#include "GraphicsRegionSelectionItem.h"

namespace emd
{

const float kRegionHandleRadius = 5.f;

MainImageScene::MainImageScene(QObject *parent)
	: QGraphicsScene(parent),
	m_imageItem(NULL),
	m_xScale(1.f),
	m_yScale(1.f),
	m_panning(false),
	m_regionSelectionItem(NULL),
	m_draggedHandle(NULL),
	m_draggedHandleIndex(-1)
{

}

void MainImageScene::setImageItem(QGraphicsItem *item)
{
	m_imageItem = item;
}

void MainImageScene::setImageRect(const QRect &rect)
{
	m_imageRect = rect;
}

void MainImageScene::setScale(const float &xScale, const float &yScale)
{
	if(xScale > 0)
		m_xScale = xScale;

	if(yScale > 0)
		m_yScale = yScale;

	updateScaledRegionSelection();
}

void MainImageScene::setRegionSelection(const QPolygonF &selection)
{
	m_regionSelection = selection;

	updateScaledRegionSelection();
}

QPolygonF MainImageScene::regionSelection() const
{
	return m_regionSelection;
}

void MainImageScene::setRegionSelectionEnabled(bool enabled)
{
	if(enabled)
	{
		if(!m_regionSelectionItem && m_imageItem)
		{
			m_regionSelectionItem = new GraphicsRegionSelectionItem(m_imageItem);
			m_regionSelectionItem->setHandleRadius(5);

			QPen pen;
			pen.setColor(QColor("white"));
			pen.setWidth(1);
			m_regionSelectionItem->setPen(pen);
		}

		if(m_regionSelection.count() == 0)
		{
			QRectF bounds = m_imageItem->boundingRect();
			bounds.setRight(bounds.width() / m_xScale);
			bounds.setBottom(bounds.height() / m_yScale);

			QPolygonF polygon;

			// CCW winding order
			polygon.append(bounds.topLeft());
			polygon.append(bounds.bottomLeft());
			polygon.append(bounds.bottomRight());
			polygon.append(bounds.topRight());

			this->setRegionSelection(polygon);
		}
	}

	m_regionSelectionItem->setVisible(enabled);

	foreach(QGraphicsItem *handle, m_regionHandles)
		handle->setVisible(enabled);
}

void MainImageScene::updateScaledRegionSelection()
{
	QTransform transform;
	transform.scale(m_xScale, m_yScale);

	m_scaledRegionSelection = transform.map(m_regionSelection);

	while(m_regionHandles.count() < m_regionSelection.count())
	{
		QGraphicsItem *handle = m_regionSelectionItem->handleItem();
		handle->setZValue(10);
		handle->setParentItem(m_imageItem);

		m_regionHandles.append(handle);
	}

	while(m_regionHandles.count() > m_regionSelection.count())
		m_regionHandles.removeLast();

	updateRegionSelectionItems();
}

void MainImageScene::updateRegionSelectionItems()
{
	if(!m_regionSelectionItem)
		return;

	m_regionSelectionItem->setPolygon(m_scaledRegionSelection);

	int index = 0;
	foreach(QGraphicsItem *handle, m_regionHandles)
	{
		handle->setPos(m_scaledRegionSelection.at(index));

		++index;
	}
}

/********************** Mouse Events **************************/
void MainImageScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
	if(e->button() == Qt::LeftButton)
	{
		bool panning = true;

		// Did we hit a handle?
		QGraphicsItem *item = this->itemAt(e->scenePos(), QTransform());
		m_draggedHandle = NULL;
		m_draggedHandleIndex = m_regionHandles.indexOf((QGraphicsEllipseItem*) item);
		if(m_draggedHandleIndex >= 0)
		{
			if(e->modifiers() == 0)
			{
				m_draggedHandle = m_regionHandles.at(m_draggedHandleIndex);
			}
			else if(e->modifiers() == Qt::ALT)
			{
				if(m_regionSelection.count() > 3)
				{
					m_scaledRegionSelection.removeAt(m_draggedHandleIndex);
					m_regionSelection.removeAt(m_draggedHandleIndex);

					m_regionSelectionItem->setPolygon(m_scaledRegionSelection);

					removeItem(m_regionHandles.takeAt(m_draggedHandleIndex));
				}
			}

			panning = false;
		}
		else
		{
			if(m_regionSelectionItem)
			{
				// If we didn't hit a handle, see if we hit a ghost.
				int before, after;
				QGraphicsItem *ghost = m_regionSelectionItem->takeHandleGhost(before, after);
				if(ghost)
				{
					m_scaledRegionSelection.insert(after, ghost->pos());
					m_regionSelectionItem->setPolygon(m_scaledRegionSelection);

					QPointF point(ghost->pos().x() / m_xScale, ghost->pos().y() / m_yScale);
					m_regionSelection.insert(after, point);

					m_regionHandles.insert(after, ghost);
					m_draggedHandle = ghost;
					m_draggedHandleIndex = after;

					panning = false;
				}
			}
		}

		if(e->modifiers() & Qt::ALT)
			panning = false;

		if(panning)
		{
			m_panning = true;
			emit(panningChanged(true));
		}
	}

	e->accept();
}

void MainImageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
	if(e->button() == Qt::LeftButton)
	{
		m_draggedHandle = NULL;
		m_draggedHandleIndex = -1;

		if(m_panning)
		{
			m_panning = false;
			emit(panningChanged(false));
		}
	}

	e->accept();
}

void MainImageScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
	QPoint delta = e->screenPos() - e->lastScreenPos();
	if(m_panning)
	{
		emit(imagePanned(delta));

		e->accept();
	}
	else
	{
		QGraphicsScene::mouseMoveEvent(e);

		if(m_draggedHandle)
		{
			QPointF scaledPoint = m_imageItem->mapFromScene(e->scenePos());

			QPointF point(scaledPoint.x() / m_xScale, scaledPoint.y() / m_yScale);

			if(point.x() < m_imageRect.x())
				point.rx() = m_imageRect.x();
			else if(point.x() > m_imageRect.x() + m_imageRect.width())
				point.rx() = m_imageRect.x() + m_imageRect.width();
			if(point.y() < m_imageRect.y())
				point.ry() = m_imageRect.y();
			else if(point.y() > m_imageRect.y() + m_imageRect.height())
				point.ry() = m_imageRect.y() + m_imageRect.height();

			scaledPoint.rx() = point.x() * m_xScale;
			scaledPoint.ry() = point.y() * m_yScale;

			m_regionSelection.replace(m_draggedHandleIndex, point);
			m_scaledRegionSelection.replace(m_draggedHandleIndex, scaledPoint);

			updateRegionSelectionItems();
			
			QPointF pos = m_imageItem->mapFromScene(e->scenePos());
			emit(cursorPositionChanged(point.x(), point.y()));
		}	
	}
}

void MainImageScene::wheelEvent(QGraphicsSceneWheelEvent *e)
{
	e->accept();
}

} // namespace emd
