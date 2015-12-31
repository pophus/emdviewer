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

#ifndef EMD_GRAPHICSIMAGEWIDGET_H
#define EMD_GRAPHICSIMAGEWIDGET_H

#include <QtWidgets>

#include "ImageWidget.h"

class QGraphicsView;
class QImage;

namespace emd
{

class ImageScene;

class GraphicsImageWidget : public ImageWidget
{
	Q_OBJECT
public:
	GraphicsImageWidget(QWidget *parent = 0);

	void resizeEvent(QResizeEvent *e);

private:
	// UI
	QGraphicsView *m_view;
	ImageScene *m_scene;
	QGraphicsRectItem *m_imageItem;
	QRect m_imageRect;

	QGraphicsRectItem *m_viewRectItem;
	QGraphicsRectItem *m_imageRectItem;

	void rescale();
	void resizeItem();
	void checkOverlap();
	void updateOffset();
	void updateAfterDelay(int delay);	// delay in ms

public slots:
	// ImageWidget
	virtual void displayImage(QImage *);
	//virtual void setPosition(float xPosition, float yPosition);
	//virtual void setTiling(bool xTiled, bool yTiled);
	//virtual void setScale(float xScale, float yScale);

private slots:
	void update();
	void setPanning(bool);
	void panImage(float, float);

signals:

};

} // namespace emd

#endif