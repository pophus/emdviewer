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

#include "MainGraphicsImageWidget.h"

#include <qgraphicsitem.h>
#include <qgraphicsview.h>

#include "Frame.h"
#include "GraphicsImageItem.h"
#include "MainImageScene.h"
#include "MainImageView.h"

#include "PointCloud.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static const float kSmallValue = 1.E-5f;

namespace emd
{

static const int UPDATE_DELAY = 50;

MainGraphicsImageWidget::MainGraphicsImageWidget(QWidget *parent)
	: MainImageWidget(parent)
{
	// Set up the scene
	m_imageScene = new MainImageScene(this);
	m_imageScene->setSceneRect(-960, -540, 1920, 1080);
	connect(m_imageScene, SIGNAL(panningChanged(bool)), this, SLOT(setPanning(bool)));
	connect(m_imageScene, SIGNAL(imagePanned(QPoint)), this, SLOT(panImage(QPoint)));
	connect(m_imageScene, SIGNAL(cursorPositionChanged(float, float)),
		this, SIGNAL(cursorPositionChanged(float, float)));

	m_imageItem = new GraphicsImageItem();
	m_imageItem->setPen(Qt::NoPen);
	m_imageScene->setImageItem(m_imageItem);
	m_imageScene->addItem(m_imageItem);
	connect(m_imageItem, SIGNAL(cursorPositionChanged(float, float)),
		this, SIGNAL(cursorPositionChanged(float, float)));

	// Image positioning display
	m_viewRectItem = m_imageScene->addRect(0, 0, 0, 0);
	m_viewRectItem->setOpacity(0.7);
	m_viewRectItem->setZValue(1.0f);
	m_viewRectItem->setBrush(QBrush(QColor(220, 220, 220)));
	QPen vrPen(QColor(180, 180, 180));
	vrPen.setJoinStyle(Qt::RoundJoin);
	vrPen.setWidth(3);
	m_viewRectItem->setPen(vrPen);
	m_viewRectItem->setVisible(false);

	m_imageRectItem = new QGraphicsRectItem(m_viewRectItem);
	m_imageRectItem->setOpacity(0.7);
	m_imageRectItem->setZValue(0.9f);
	m_imageRectItem->setBrush(QBrush(QColor(200, 220, 255)));
	QPen irPen(QColor(110, 140, 190));
	irPen.setWidth(3);
	irPen.setJoinStyle(Qt::RoundJoin);
	m_imageRectItem->setPen(irPen);
	m_imageRectItem->setVisible(false);

	// Set up the view
	m_graphicsView = new MainImageView(this);
	m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//view->setMouseTracking(true);
	m_graphicsView->setScene(m_imageScene);
    connect(m_graphicsView, SIGNAL(mouseWheelDown()),
        this, SIGNAL(mouseWheelDown()));
    connect(m_graphicsView, SIGNAL(mouseWheelUp()),
        this, SIGNAL(mouseWheelUp()));

	QHBoxLayout *mainLayout = new QHBoxLayout();
	mainLayout->addWidget(m_graphicsView);
	setLayout(mainLayout);

	m_imageRect = QRect(0, 0, 0, 0);

	m_updateTimer = new QTimer(this);
	m_updateTimer->setSingleShot(true);
	connect(m_updateTimer, SIGNAL(timeout()),
		this, SLOT(update()));

    this->setContentsMargins(0, 0, 0, 0);
}

/*** ImageWindow Methods ***/

void MainGraphicsImageWidget::update()
{
	if(!m_currentImage)
		return;

	m_updateTimer->stop();

	if(m_displayState & SCALE_CHANGED)
		rescale();

	if(m_displayState & ITEM_SIZE_CHANGED)
		resizeItem();

	if(m_displayState & OVERLAP_CHANGED)
		checkOverlap();

	if(m_displayState & OFFSET_CHANGED)
		updateOffset();
	
	m_displayState = 0;
	m_imageItem->setPos(-m_imageRect.width()/2 + m_xOffset, -m_imageRect.height()/2 + m_yOffset);
	m_imageScene->update();
}

void MainGraphicsImageWidget::updateAfterDelay(int delay)
{
	// Delays an update by delay ms in case there are other changes coming
	m_updateTimer->start(delay);
}

void MainGraphicsImageWidget::reset()
{
    MainImageWidget::reset();

    if(m_currentImage)
        delete m_currentImage;

    m_currentImage = NULL;

    m_imageItem->setBrush(QBrush());
}

QRect MainGraphicsImageWidget::viewportRect() const
{
    return m_graphicsView->viewport()->rect();
}

void MainGraphicsImageWidget::setRegionSelectionEnabled(bool enabled)
{
	m_imageScene->setRegionSelectionEnabled(enabled);
}

void MainGraphicsImageWidget::setRegionSelection(const QPolygonF &selection)
{
	m_imageScene->setRegionSelection(selection);
}

QPolygonF MainGraphicsImageWidget::regionSelection() const
{
	return m_imageScene->regionSelection();
}

QImage *MainGraphicsImageWidget::windowImage() const
{
    QRectF itemRect = m_imageItem->mapRectToScene(m_imageItem->rect());
    QRectF windowRect = m_graphicsView->mapToScene(m_graphicsView->viewport()->rect()).boundingRect();

    QRectF clippedRect;
    clippedRect.setLeft(MAX(itemRect.left(), windowRect.left()));
    clippedRect.setRight(MIN(itemRect.right(), windowRect.right()));
    clippedRect.setTop(MAX(itemRect.top(), windowRect.top()));
    clippedRect.setBottom(MIN(itemRect.bottom(), windowRect.bottom()));

    QImage *image = new QImage(clippedRect.width(), clippedRect.height(), QImage::Format_RGB32);

    QPainter painter(image);

    m_imageScene->render(&painter, QRectF(), clippedRect);

    return image;
}

/******************************* Image update methods ******************************/

void MainGraphicsImageWidget::resizeEvent(QResizeEvent *e)
{
	MainImageWidget::resizeEvent(e);

	m_graphicsView->centerOn(0, 0);
}

void MainGraphicsImageWidget::rescale()
{
	if(!m_currentImage)
		return;

	if(m_scaledImage)
	{
		delete m_scaledImage;
		m_scaledImage = 0;
	}
	if(abs(m_hScale - 1.0f) < kSmallValue && abs(m_vScale - 1.0f) < kSmallValue)
	{
		m_imageItem->setBrush(*m_currentImage);
		m_imageRect = m_currentImage->rect();
	}
	else
	{
		QImage scaledImage = m_currentImage->scaled((int)(m_hScale * m_currentImage->width()),
											(int) (m_vScale * m_currentImage->height()),
											Qt::IgnoreAspectRatio,
											Qt::SmoothTransformation);
		m_imageItem->setBrush(scaledImage);
		m_imageRect = scaledImage.rect();
	}

	m_imageScene->setScale(m_hScale, m_vScale);
	
	m_displayState |= ITEM_SIZE_CHANGED;
}

void MainGraphicsImageWidget::resizeItem()
{
	QRect itemRect = m_imageRect;

	if(m_hTiling)
	{
		int vw = m_graphicsView->rect().width();
		int iw = m_imageRect.width();
		int nx = (vw - iw)/(2*iw) + ((vw - iw)/2 % iw > 0 ? 1 : 0);
		if(nx <= 0)
			nx = 1;
		int t = (nx*iw - vw)/2;
		int x = (nx+1)*iw - t;

		itemRect.setX(-x);
		itemRect.setWidth(2*x + iw);

		while(m_xOffset > m_imageRect.width())
			m_xOffset -= m_imageRect.width();
		while(-m_xOffset > m_imageRect.width())
			m_xOffset += m_imageRect.width();
	}

	if(m_vTiling)
	{
		int vh = m_graphicsView->rect().height();
		int ih = m_imageRect.height();
		int ny = (vh - ih)/(2*ih) + ((vh - ih)/2 % ih > 0 ? 1 : 0);
		if(ny <= 0)
			ny = 1;
		int t = (ny*ih - vh)/2;
		int y = (ny+1)*ih - t;

		itemRect.setY(-y);
		itemRect.setHeight(2*y + ih);

		while(m_yOffset > m_imageRect.height())
			m_yOffset -= m_imageRect.height();
		while(-m_yOffset > m_imageRect.height())
			m_yOffset += m_imageRect.height();
	}
	
	m_imageItem->setRect(itemRect);
	m_displayState |= OVERLAP_CHANGED;
}

void MainGraphicsImageWidget::checkOverlap()
{
	m_overlapped = false;
	if(m_imageRect.width() > m_graphicsView->width() && !m_hTiling)
	{
		m_overlapped = true;
	}

	if(m_imageRect.height() > m_graphicsView->height() && !m_vTiling)
	{
		m_overlapped = true;
	}

	if(m_overlapped)
	{
		float imRecWidth = m_imageRect.width();
		float imRecHeight = m_imageRect.height();

		float vRecWidth = m_graphicsView->width();
		float vRecHeight = m_graphicsView->height();

        // The larger of the two rects should not take up more than 25% of the image window.
        float maxWidth = MAX(imRecWidth, vRecWidth);
        float maxHeight = MAX(imRecHeight, vRecHeight);
        float maxDim = MAX(maxWidth, maxHeight);
        float maxViewDim = MAX(m_graphicsView->rect().width(), m_graphicsView->rect().height());

        m_positionOverlayCorrection = 0.25 * maxViewDim / maxDim;

        imRecWidth *= m_positionOverlayCorrection;
        imRecHeight *= m_positionOverlayCorrection;
        vRecWidth *= m_positionOverlayCorrection;
        vRecHeight *= m_positionOverlayCorrection;

        // The view rect is set to a fixed position  as close to the corner of the view
        // as possible while leaving enough room to move the image rect item all the way
        // to the edge (with a small fixed margin).
		m_viewRectItem->setRect(-0.5 * vRecWidth, -0.5 * vRecHeight, 
                                vRecWidth, vRecHeight);
		m_viewRectItem->setPos(-0.5 * m_graphicsView->rect().width() + 10
                                + m_positionOverlayCorrection * maxWidth - 0.5 * vRecWidth,
                             -0.5 * m_graphicsView->rect().height() + 10
                                + m_positionOverlayCorrection *maxHeight - 0.5 * vRecHeight);

        // The origin of the image rect item is set in the middle so it can just be positioned
        // relative to the view rect item the same as the actual image is to the view.
		m_imageRectItem->setRect(0, 0, imRecWidth, imRecHeight);
		m_imageRectItem->setPos(m_positionOverlayCorrection * m_xOffset - imRecWidth/2, 
                                m_positionOverlayCorrection * m_yOffset - imRecHeight/2);

        QImage scaledImage = m_currentImage->scaled(imRecWidth, imRecHeight);
        QBrush bgBrush(scaledImage);
        m_imageRectItem->setBrush(bgBrush);
	}

    m_displayState |= OFFSET_CHANGED;
}

void MainGraphicsImageWidget::updateOffset()
{
	if(!m_hTiling)
	{
		int vw = m_graphicsView->width()/2;
		int iw = m_imageRect.width()/2;
		float hMin = 0.0f, hMax = 1.0f;
		if(iw > vw)
		{
			if(iw - m_xOffset < vw)
			{
				m_xOffset = iw - vw;
				hMax = (float) vw / iw;
			}
			else if(iw + m_xOffset < vw)
			{
				m_xOffset = vw - iw;
				hMin = (-1.0f * m_xOffset) / iw;
			}
			else
			{
				hMin = (float) (iw - vw - m_xOffset) / m_imageRect.width();
				hMax = hMin + (float) vw / iw;
			}

			emit(imageHPos(hMin, hMax));
		}
		else
		{
			if(iw - m_xOffset > vw)
				m_xOffset = iw - vw;
			else if(iw + m_xOffset > vw)
				m_xOffset = vw - iw;

			emit(imageHPos(0, 1));
		}
	}
	else
	{
		while(m_xOffset > m_imageRect.width())
			m_xOffset -= m_imageRect.width();
		while(-m_xOffset > m_imageRect.width())
			m_xOffset += m_imageRect.width();
	}
	if(!m_vTiling)
	{
		int vh = m_graphicsView->height()/2;
		int ih = m_imageRect.height()/2;
		float vMin = 0.0f, vMax = 1.0f;
		if(ih > vh)
		{
			if(ih - m_yOffset < vh)
			{
				m_yOffset = ih - vh;
				vMax = (float) vh / ih;
			}
			else if(ih + m_yOffset < vh)
			{
				m_yOffset = vh - ih;
				vMin = (-1.0f * m_yOffset) / ih;
			}
			else
			{
				vMin = (float) (ih - vh - m_yOffset) / m_imageRect.height();
				vMax = vMin + (float) vh / ih;
			}

			emit(imageVPos(vMin, vMax));
		}
		else
		{
			if(ih - m_yOffset > vh)
				m_yOffset = ih - vh;
			else if(ih + m_yOffset > vh)
				m_yOffset = vh - ih;

			emit(imageVPos(0, 1));
		}
	}
	else
	{
		while(m_yOffset > m_imageRect.height())
			m_yOffset -= m_imageRect.height();
		while(-m_yOffset > m_imageRect.height())
			m_yOffset += m_imageRect.height();
	}

	if(m_overlapped)
    {
        QRectF size = m_imageRectItem->rect();
		m_imageRectItem->setPos(m_positionOverlayCorrection * m_xOffset - 0.5 * size.width(), 
                                m_positionOverlayCorrection * m_yOffset - 0.5 * size.height());
    }
}

/********************** Parameter Functions *****************************/

void MainGraphicsImageWidget::setHTiling(bool tiling)
{
	m_hTiling = (tiling);
	m_displayState |= ITEM_SIZE_CHANGED;
	updateAfterDelay(UPDATE_DELAY);
}
void MainGraphicsImageWidget::setVTiling(bool tiling)
{
	m_vTiling = (tiling);
	m_displayState |= ITEM_SIZE_CHANGED;
	updateAfterDelay(UPDATE_DELAY);
}

void MainGraphicsImageWidget::setHScaling(float scale)
{
	this->setScaling(scale, -1);
}
void MainGraphicsImageWidget::setVScaling(float scale)
{
	this->setScaling(-1, scale);
}

void MainGraphicsImageWidget::setScaling(float hs, float vs)
{
	if(hs > 0)
		m_hScale = hs;

	if(vs > 0)
		m_vScale = vs;
	
	m_imageItem->setScale(hs, vs);

	m_displayState |= SCALE_CHANGED;
	updateAfterDelay(UPDATE_DELAY);
}

/************************************** Slots **************************************/

void MainGraphicsImageWidget::displayImage(QImage *image) 
{
	if(m_currentImage)
		delete m_currentImage;

	m_currentImage = image;

	m_imageScene->setImageRect(image->rect());

	m_displayState |= SCALE_CHANGED;

	update();
}

void MainGraphicsImageWidget::displayPointCloud(PointCloud *pointCloud)
{
	m_imageItem->displayPointCloud(pointCloud);
}

void MainGraphicsImageWidget::panImage(QPoint delta)
{
	translateHorizontal(delta.x());
	translateVertical(delta.y());
	update();
}

void MainGraphicsImageWidget::setPanning(bool panning)
{
	if(m_currentImage && m_overlapped)
	{
		m_viewRectItem->setVisible(panning);
		m_imageRectItem->setVisible(panning);
	}
}

void MainGraphicsImageWidget::translateHorizontal(int delta)
{
	if(0 == delta)
		return;
	m_xOffset += delta;
	m_displayState |= OFFSET_CHANGED;
	update();
}

void MainGraphicsImageWidget::translateVertical(int delta)
{
	if(0 == delta)
		return;
	m_yOffset += delta;
	m_displayState |= OFFSET_CHANGED;
	update();
}

void MainGraphicsImageWidget::setHorizontalPos(int pos)
{
	int offset = (m_imageRect.width() - m_graphicsView->width())/2 - (int) (m_hScale * pos);

    if(m_xOffset != offset)
    {
        m_xOffset = offset;

	    m_displayState |= OFFSET_CHANGED;
	    update();
    }
}

void MainGraphicsImageWidget::setVerticalPos(int pos)
{
    int offset = (m_imageRect.height() - m_graphicsView->height())/2 - (int) (m_vScale * pos);

    if(m_yOffset != offset)
    {
        m_yOffset = offset;

	    m_displayState |= OFFSET_CHANGED;
	    update();
    }
}

} // namespace emd
