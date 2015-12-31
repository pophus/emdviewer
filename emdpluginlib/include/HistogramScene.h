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

#ifndef EMD_HISTOGRAMSCENE_H
#define EMD_HISTOGRAMSCENE_H

#include <qgraphicsscene.h>
#include <QGraphicsRectItem>

class QImage;

namespace emd
{

class HistogramScene : public QGraphicsScene
{
	Q_OBJECT
public:
	HistogramScene(QObject *parent = 0);
	void setImage(QImage *image);
	void setScalingLimits(const float &lower, const float &upper);
    void setScalingValues(float lower, float upper);
	void setGradient(const QLinearGradient &gradient);

	// Mouse functions
	void mousePressEvent(QGraphicsSceneMouseEvent *e);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
	void wheelEvent(QGraphicsSceneWheelEvent *e);
	
private:
	bool m_dragging;
	bool m_sliding;
	bool m_slid;
	QSize m_imageSize;
	float m_barYPos;
	QPoint m_mousePos;
	QGraphicsRectItem m_leftBar;
	QGraphicsRectItem m_topLeft;
	QGraphicsRectItem m_bottomLeft;
	QGraphicsRectItem m_leftLine;
	QGraphicsRectItem m_rightBar;
	QGraphicsRectItem m_topRight;
	QGraphicsRectItem m_bottomRight;
	QGraphicsRectItem m_rightLine;
	QGraphicsRectItem m_histogramMask;
	QGraphicsRectItem m_gradientBox;
	QGraphicsItem *m_draggedItem;
	QLinearGradient m_gradient;
    float m_lowerLimit;
    float m_upperLimit;

	void updateGradient();
	float limitForPosition(float pos) const;
	float positionForLimit(float limit) const;
    float relativeToAbsolute(float relative) const;

signals:
	void scalingLimitsChanged(float lower, float upper);
};

} // namespace emd


#endif