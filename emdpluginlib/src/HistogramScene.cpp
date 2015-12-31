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

#include "HistogramScene.h"

#include "qgraphicssceneevent.h"

#include "ColourManager.h"
#include "ColourMap.h"
#include "Frame.h"
#include "Histogram.h"
#include "ImageWindowModule.h"

namespace emd
{

const float WHEEL_FACTOR = 20.f;
const int BAR_WIDTH = 10;

HistogramScene::HistogramScene(QObject *parent)
	: QGraphicsScene(parent),
	m_barYPos(0),
	m_leftBar(-BAR_WIDTH/2, 0, BAR_WIDTH, HISTOGRAM_HEIGHT),
	m_topLeft(-BAR_WIDTH/2, -BAR_WIDTH, BAR_WIDTH, BAR_WIDTH),
	m_bottomLeft(-BAR_WIDTH/2, 0, BAR_WIDTH, BAR_WIDTH),
	m_leftLine(-1, BAR_WIDTH, 2, HISTOGRAM_HEIGHT),
	m_rightBar(-BAR_WIDTH/2, 0, BAR_WIDTH, HISTOGRAM_HEIGHT),
	m_topRight(-BAR_WIDTH/2, -BAR_WIDTH, BAR_WIDTH, BAR_WIDTH),
	m_bottomRight(-BAR_WIDTH/2, 0, BAR_WIDTH, BAR_WIDTH),
	m_rightLine(-1, BAR_WIDTH, 2, HISTOGRAM_HEIGHT),
	m_histogramMask(0, 0, HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT),
	m_gradientBox(0, 0, HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT),
	m_imageSize(HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT)
{
	// Init
	m_dragging = false;
	m_sliding = false;
	m_slid = false;

	// Setup
	m_leftBar.setZValue(10);
	m_leftBar.setPen(QPen(Qt::NoPen));
	m_leftBar.setHandlesChildEvents(true);

	m_topLeft.setBrush(QBrush(QColor(255, 50, 50)));
	m_topLeft.setPen(QPen(QBrush(QColor(0, 20, 80)), 1));
	m_topLeft.setZValue(5);
	m_topLeft.setParentItem(&m_leftBar);
	m_topLeft.setPos(0, BAR_WIDTH);

	m_bottomLeft.setBrush(QBrush(QColor(255, 50, 50)));
	m_bottomLeft.setPen(QPen(QBrush(QColor(0, 20, 80)), 1));
	m_bottomLeft.setZValue(5);
	m_bottomLeft.setParentItem(&m_leftBar);
	m_bottomLeft.setPos(0, HISTOGRAM_HEIGHT);

	m_leftLine.setBrush(QBrush(QColor(255, 50, 50)));
	m_leftLine.setPen(QPen(QBrush(QColor(0, 20, 80)), 1));
	m_leftLine.setParentItem(&m_leftBar);
	m_leftLine.setPos(0, 0);

	m_rightBar.setZValue(10);
	m_rightBar.setPen(QPen(Qt::NoPen));
	m_rightBar.setHandlesChildEvents(true);
	m_rightBar.setPos(HISTOGRAM_WIDTH, 0);

	m_topRight.setBrush(QBrush(QColor(255, 50, 50)));
	m_topRight.setPen(QPen(QBrush(QColor(0, 20, 80)), 1));
	m_topRight.setZValue(5);
	m_topRight.setParentItem(&m_rightBar);
	m_topRight.setPos(0, BAR_WIDTH);

	m_bottomRight.setBrush(QBrush(QColor(255, 50, 50)));
	m_bottomRight.setPen(QPen(QBrush(QColor(0, 20, 80)), 1));
	m_bottomRight.setZValue(5);
	m_bottomRight.setParentItem(&m_rightBar);
	m_bottomRight.setPos(0, HISTOGRAM_HEIGHT);

	m_rightLine.setBrush(QBrush(QColor(255, 50, 50)));
	m_rightLine.setPen(QPen(QBrush(QColor(0, 20, 80)), 1));
	m_rightLine.setParentItem(&m_rightBar);
	m_rightLine.setPos(0, 0);

	this->addItem(&m_leftBar);
	this->addItem(&m_rightBar);
	this->addItem(&m_histogramMask);
	this->addItem(&m_gradientBox);

	m_histogramMask.setZValue(5);
	m_histogramMask.setPen(QPen(Qt::NoPen));
	m_gradientBox.setPen(QPen(Qt::NoPen));
}

void HistogramScene::setImage(QImage *image)
{
    if(!image)
    {
        m_leftBar.setVisible(false);
        m_rightBar.setVisible(false);

        QImage bg(":/images/MBlogo04_textured_04_cropped_small.png");
        bg = bg.scaledToHeight(m_imageSize.height());
	    QBrush brush(bg);
	    m_histogramMask.setBrush(brush);
    }
    else
    {
	    float oldLeftPos = m_leftBar.scenePos().x() / m_imageSize.width();
	    float oldRightPos = m_rightBar.scenePos().x() / m_imageSize.width();

	    m_imageSize = image->size();
	    m_histogramMask.setRect(0, 0, m_imageSize.width(), m_imageSize.height());
	    m_gradientBox.setRect(0, 0, m_imageSize.width(), m_imageSize.height());

	    m_bottomLeft.setPos(0, m_imageSize.height() + BAR_WIDTH - 1);
	    m_bottomRight.setPos(0, m_imageSize.height() + BAR_WIDTH - 1);

        QRectF oldLeftRect = m_leftLine.rect();
        oldLeftRect.setHeight(m_imageSize.height());
        m_leftLine.setRect(oldLeftRect);

        QRectF oldRightRect = m_rightLine.rect();
        oldRightRect.setHeight(m_imageSize.height());
        m_rightLine.setRect(oldRightRect);

	    QBrush brush(*image);
	    m_histogramMask.setBrush(brush);
	    delete image;

	    QSizeF sceneSize = this->sceneRect().size();
	    m_barYPos = -(sceneSize.height() - m_imageSize.height()) / 2;

	    m_leftBar.setPos(oldLeftPos * m_imageSize.width(), m_barYPos);
	    m_rightBar.setPos(oldRightPos * m_imageSize.width(), m_barYPos);

        m_leftBar.setVisible(true);
        m_rightBar.setVisible(true);

	    this->updateGradient();
    }
}

void HistogramScene::setScalingLimits(const float &lower, const float &upper)
{
	m_lowerLimit = lower;
    m_upperLimit = upper;
}

void HistogramScene::setScalingValues(float lower, float upper)
{
    float range = m_upperLimit - m_lowerLimit;
    if(range == 0)
        return;

    if(lower != kInvalidFloatValue)
    {
        float relativeLower = (lower - m_lowerLimit) / range;
        m_leftBar.setPos(relativeLower * m_imageSize.width(), m_barYPos);
    }

    if(upper != kInvalidFloatValue)
    {
        float relativeUpper = (upper - m_lowerLimit) / range;
	    m_rightBar.setPos(relativeUpper * m_imageSize.width(), m_barYPos);
    }

	this->updateGradient();
	this->update();
}

void HistogramScene::setGradient(const QLinearGradient &gradient)
{
	m_gradient = gradient;

    updateGradient();
}

void HistogramScene::updateGradient()
{
	m_gradient.setStart(m_leftBar.scenePos().x(), 0);
	m_gradient.setFinalStop(m_rightBar.scenePos().x(), 0);

	m_gradientBox.setBrush(QBrush(m_gradient));
}

float HistogramScene::limitForPosition(float pos) const
{
	if(m_imageSize.width() == 0)
		return 0;

	return pos / m_imageSize.width();
}

float HistogramScene::positionForLimit(float limit) const
{
	return limit * m_imageSize.width();
}

float  HistogramScene::relativeToAbsolute(float relative) const
{
    if(relative == kInvalidFloatValue)
        return kInvalidFloatValue;

    float range = m_upperLimit - m_lowerLimit;

    return m_lowerLimit + relative * range;
}

/********************** Mouse Events **************************/
void HistogramScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
	QPointF pos = e->scenePos();
	QGraphicsItem *item = this->itemAt(pos, QTransform());

	if(item == &m_topLeft || item == &m_bottomLeft || item == &m_leftLine)
		item = &m_leftBar;
	else if(item == &m_topRight|| item == &m_bottomRight || item == &m_rightLine)
		item = &m_rightBar;

	if(item == &m_rightBar || item == &m_leftBar)
	{
		m_dragging = true;
		m_draggedItem = item;
		// A hack to prevent auto-scaling on middle-click drag
		m_slid = true;
	}
	else
	{
		m_sliding = true;
	}

	e->accept();
}
void HistogramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
	m_dragging = false;
	m_sliding = false;

	if(!m_slid)
	{
		QPointF pos = e->scenePos();
		if(e->button() == Qt::LeftButton)
		{
			float newPos = pos.x();
			m_leftBar.setPos(newPos, m_barYPos);
			float lowerLimit = limitForPosition(newPos);

			float upperLimit = kInvalidFloatValue;
			if(newPos > m_rightBar.scenePos().x())
			{
				m_rightBar.setPos(newPos, m_barYPos);
				upperLimit = lowerLimit;
			}
			
			this->updateGradient();
			this->update();
            
			emit(scalingLimitsChanged(relativeToAbsolute(lowerLimit), relativeToAbsolute(upperLimit)));
		}
		else if(e->button() == Qt::MiddleButton)
		{
			m_leftBar.setPos(0, m_barYPos);
			m_rightBar.setPos(m_imageSize.width(), m_barYPos);

			this->updateGradient();
			this->update();

			emit(scalingLimitsChanged(m_lowerLimit, m_upperLimit));
		}
		else if(e->button() == Qt::RightButton)
		{
			float newPos = pos.x();
			m_rightBar.setPos(newPos, m_barYPos);
			float upperLimit = limitForPosition(newPos);

			float lowerLimit = kInvalidFloatValue;
			if(newPos < m_leftBar.scenePos().x())
			{
				m_leftBar.setPos(newPos, m_barYPos);
				lowerLimit = upperLimit;
			}
			
			this->updateGradient();
			this->update();

			emit(scalingLimitsChanged(relativeToAbsolute(lowerLimit), relativeToAbsolute(upperLimit)));
		}
	}

	m_slid = false;
	
	e->accept();
}
void HistogramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
	if(m_dragging)
	{
		float delta = e->screenPos().x() - e->lastScreenPos().x();
		float newPos = m_draggedItem->scenePos().x() + delta;
		m_draggedItem->moveBy(delta, 0);
		float lowerLimit = kInvalidFloatValue;
		float upperLimit = kInvalidFloatValue;
		if(m_draggedItem == &m_leftBar)
		{
			lowerLimit = limitForPosition(newPos);
			if(newPos > m_rightBar.scenePos().x())
			{
				m_rightBar.setPos(newPos, m_barYPos);
				upperLimit = lowerLimit;
			}
		}
		else
		{
			upperLimit = limitForPosition(newPos);
			if(newPos < m_leftBar.scenePos().x())
			{
				m_leftBar.setPos(newPos, m_barYPos);
				lowerLimit = upperLimit;
			}
		}

		this->updateGradient();
		this->update();

		emit(scalingLimitsChanged(relativeToAbsolute(lowerLimit), relativeToAbsolute(upperLimit)));
	}
	else if(m_sliding)
	{
		m_slid = true;
		int delta = e->screenPos().x() - e->lastScreenPos().x();
		float increment = delta * 0.1f;
		
		float lowerLimit = kInvalidFloatValue;
		float upperLimit = kInvalidFloatValue;

		if(e->buttons() & Qt::LeftButton)
		{
			float newPos = m_leftBar.scenePos().x() + increment;
			lowerLimit = limitForPosition(newPos);
			m_leftBar.setPos(newPos, m_barYPos);
		}
		else if(e->buttons() & Qt::RightButton)
		{
			float newPos = m_rightBar.scenePos().x() + increment;
			upperLimit = limitForPosition(newPos);
			m_rightBar.setPos(newPos, m_barYPos);
		}
		else if(e->buttons() & Qt::MiddleButton)
		{
			float newPos = m_leftBar.scenePos().x() + increment;
			lowerLimit = limitForPosition(newPos);
			m_leftBar.setPos(newPos, m_barYPos);
			
			newPos = m_rightBar.scenePos().x() + increment;
			upperLimit = limitForPosition(newPos);
			m_rightBar.setPos(newPos, m_barYPos);
		}

		this->updateGradient();
		this->update();

		emit(scalingLimitsChanged(relativeToAbsolute(lowerLimit), relativeToAbsolute(upperLimit)));
	}
	e->accept();
}
void HistogramScene::wheelEvent(QGraphicsSceneWheelEvent *e)
{
	float delta = e->delta() / WHEEL_FACTOR;

	m_leftBar.moveBy(delta, 0);
	m_rightBar.moveBy(delta, 0);

	float lowerLimit = limitForPosition(m_leftBar.scenePos().x());
	float upperLimit = limitForPosition(m_rightBar.scenePos().x());

	this->updateGradient();
	this->update();

	emit(scalingLimitsChanged(relativeToAbsolute(lowerLimit), relativeToAbsolute(upperLimit)));

	e->accept();
}

} // namespace emd
