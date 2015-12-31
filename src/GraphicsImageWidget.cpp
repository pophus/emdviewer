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

#include "GraphicsImageWidget.h"

#include <cmath>

#include "ImageScene.h"

#include "Frame.h"

namespace emd
{

static const int UPDATE_DELAY = 50;

GraphicsImageWidget::GraphicsImageWidget(QWidget *parent)
	: ImageWidget(parent)
{
	// Set up the scene
	m_scene = new ImageScene(this);
	m_imageItem = new QGraphicsRectItem();
	m_scene->addItem(m_imageItem);
	m_scene->setSceneRect(-960, -540, 1920, 1080);
	connect(m_scene, SIGNAL(panningStateChanged(bool)), 
		this, SLOT(setPanning(bool)));
	connect(m_scene, SIGNAL(imagePanned(float, float)), 
		this, SLOT(panImage(float, float)));

	// Image positioning display
	m_viewRectItem = m_scene->addRect(0, 0, 0, 0);
	m_viewRectItem->setOpacity(0.5);
	m_viewRectItem->setZValue(1.0f);
	m_viewRectItem->setBrush(QBrush(QColor(220, 220, 220)));
	QPen vrPen(QColor(180, 180, 180));
	vrPen.setJoinStyle(Qt::RoundJoin);
	vrPen.setWidth(3);
	m_viewRectItem->setPen(vrPen);
	m_viewRectItem->setVisible(false);

	m_imageRectItem = new QGraphicsRectItem(m_viewRectItem);
	m_imageRectItem->setOpacity(0.5);
	m_imageRectItem->setZValue(0.9f);
	m_imageRectItem->setBrush(QBrush(QColor(220, 240, 255)));
	QPen irPen(QColor(140, 160, 190));
	irPen.setWidth(3);
	irPen.setJoinStyle(Qt::RoundJoin);
	m_imageRectItem->setPen(irPen);
	m_imageRectItem->setVisible(false);

	// Set up the view
	m_view = new QGraphicsView(this);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//view->setMouseTracking(true);
	m_view->setScene(m_scene);

	QHBoxLayout *mainLayout = new QHBoxLayout();
	mainLayout->addWidget(m_view);
	setLayout(mainLayout);

	m_imageRect = QRect(0, 0, 0, 0);
}

void GraphicsImageWidget::update()
{
	if(!m_currentImage)
		return;

	if(m_displayState & DisplayStateScaleChanged)
		rescale();

	if(m_displayState & DisplayStateImageChanged
		|| m_displayState & DisplayStateTilingChanged)
		resizeItem();

	if(m_displayState & DisplayStatePositionChanged)
		updateOffset();
	
	m_displayState = 0;

	m_imageItem->setPos(-m_imageRect.width()/2 + m_xPosition, 
						-m_imageRect.height()/2 + m_yPosition);

	m_scene->update();
}

void GraphicsImageWidget::resizeEvent(QResizeEvent *)
{
	m_displayState |= (DisplayStatePositionChanged);
	update();
	m_view->centerOn(0, 0);
}

/******************************* Image update methods ******************************/

void GraphicsImageWidget::rescale()
{
	if(!m_currentImage)
		return;

    if(std::abs(m_xScale - 1.0f) < 1E-5 && std::abs(m_yScale - 1.0f) < 1E-5)
	{
		m_imageItem->setBrush(*m_currentImage);
		m_imageRect = m_currentImage->rect();
	}
	else
	{
		QImage scaledImage = m_currentImage->scaled((int) (m_xScale * m_currentImage->width()),
											(int) (m_yScale * m_currentImage->height()),
											Qt::IgnoreAspectRatio,
											Qt::SmoothTransformation);
		m_imageItem->setBrush(scaledImage);
		m_imageRect = scaledImage.rect();
	}
	
	m_displayState |= DisplayStateImageChanged;
}

void GraphicsImageWidget::resizeItem()
{
	QRect itemRect = m_imageRect;

	if(m_xTiled)
	{
		int vw = m_view->rect().width();
		int iw = m_imageRect.width();
		int nx = (vw - iw)/(2*iw) + ((vw - iw)/2 % iw > 0 ? 1 : 0);
		if(nx <= 0)
			nx = 1;
		int t = (nx*iw - vw)/2;
		int x = (nx+1)*iw - t;

		itemRect.setX(-x);
		itemRect.setWidth(2*x + iw);

		while(m_xPosition > m_imageRect.width())
			m_xPosition -= m_imageRect.width();
		while(-m_xPosition > m_imageRect.width())
			m_xPosition += m_imageRect.width();
	}

	if(m_yTiled)
	{
		int vh = m_view->rect().height();
		int ih = m_imageRect.height();
		int ny = (vh - ih)/(2*ih) + ((vh - ih)/2 % ih > 0 ? 1 : 0);
		if(ny <= 0)
			ny = 1;
		int t = (ny*ih - vh)/2;
		int y = (ny+1)*ih - t;

		itemRect.setY(-y);
		itemRect.setHeight(2*y + ih);

		while(m_yPosition > m_imageRect.height())
			m_yPosition -= m_imageRect.height();
		while(-m_yPosition > m_imageRect.height())
			m_yPosition += m_imageRect.height();
	}
	
	m_imageItem->setRect(itemRect);
	m_displayState |= DisplayStatePositionChanged;
}

void GraphicsImageWidget::updateOffset()
{
	if(!m_xTiled)
	{
		int vw = m_view->width()/2;
		int iw = m_imageRect.width()/2;
		float hMin = 0.0f, hMax = 1.0f;
		if(iw > vw)
		{
			if(iw - m_xPosition < vw)
			{
				m_xPosition = iw - vw;
				hMax = (float) vw / iw;
			}
			else if(iw + m_xPosition < vw)
			{
				m_xPosition = vw - iw;
				hMin = (-1.0f * m_xPosition) / iw;
			}
			else
			{
				hMin = (float) (iw - vw - m_xPosition) / m_imageRect.width();
				hMax = hMin + (float) vw / iw;
			}
		}
		else
		{
			if(iw - m_xPosition > vw)
				m_xPosition = iw - vw;
			else if(iw + m_xPosition > vw)
				m_xPosition = vw - iw;
		}
	}
	else
	{
		while(m_xPosition > m_imageRect.width())
			m_xPosition -= m_imageRect.width();
		while(-m_xPosition > m_imageRect.width())
			m_xPosition += m_imageRect.width();
	}

	if(!m_yTiled)
	{
		int vh = m_view->height()/2;
		int ih = m_imageRect.height()/2;
		float vMin = 0.0f, vMax = 1.0f;
		if(ih > vh)
		{
			if(ih - m_yPosition < vh)
			{
				m_yPosition = ih - vh;
				vMax = (float) vh / ih;
			}
			else if(ih + m_yPosition < vh)
			{
				m_yPosition = vh - ih;
				vMin = (-1.0f * m_yPosition) / ih;
			}
			else
			{
				vMin = (float) (ih - vh - m_yPosition) / m_imageRect.height();
				vMax = vMin + (float) vh / ih;
			}
		}
		else
		{
			if(ih - m_yPosition > vh)
				m_yPosition = ih - vh;
			else if(ih + m_yPosition > vh)
				m_yPosition = vh - ih;
		}
	}
	else
	{
		while(m_yPosition > m_imageRect.height())
			m_yPosition -= m_imageRect.height();
		while(-m_yPosition > m_imageRect.height())
			m_yPosition += m_imageRect.height();
	}

	m_imageRectItem->setPos(m_xPosition/10, m_yPosition/10);
}

/************************************** Slots **************************************/

void GraphicsImageWidget::displayImage(QImage *image) 
{
	if(m_currentImage)
		delete m_currentImage;

	m_currentImage = image;

	// TODO: enum names don't really make sense, need to rename
	// and possibly change update functions.
	m_displayState |= DisplayStateScaleChanged;

	update();
}

void GraphicsImageWidget::panImage(float dx, float dy)
{
	setPosition(m_xPosition + dx, m_yPosition + dy);
}

void GraphicsImageWidget::setPanning(bool panning)
{
	if(m_xTiled || m_yTiled)
		return;

	bool overlapped = (m_view->width() < m_imageItem->rect().width())
						|| (m_view->height() < m_imageItem->rect().height());

	if(overlapped)
	{
		m_viewRectItem->setVisible(panning);
		m_imageRectItem->setVisible(panning);
	}
}

} // namespace emd


