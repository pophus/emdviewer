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

#ifndef EMD_MAINGRAPHICSIMAGEWIDGET_H
#define EMD_MAINGRAPHICSIMAGEWIDGET_H

#include <QTimer>

#include "MainImageWidget.h"

class QGraphicsTextItem;
class QGraphicsView;
class QImage;

namespace emd
{

class Frame;
class GraphicsImageItem;
class MainImageScene;
class MainImageView;

class MainGraphicsImageWidget : public MainImageWidget
{
	Q_OBJECT
public:
	MainGraphicsImageWidget(QWidget *parent = 0);

	// MainImageWidget
    virtual void reset();

    virtual QRect viewportRect() const;

	virtual void setRegionSelection(const QPolygonF &selection);
	virtual QPolygonF regionSelection() const;

    virtual QImage *windowImage() const;

    // QWidget
	void resizeEvent(QResizeEvent *e);

private:
	// UI
	MainImageView *m_graphicsView;
	MainImageScene *m_imageScene;
	GraphicsImageItem *m_imageItem;
	QRect m_imageRect;

	QGraphicsRectItem *m_viewRectItem;
	QGraphicsRectItem *m_imageRectItem;
    float m_positionOverlayCorrection;

	QTimer *m_updateTimer;

	void rescale();
	void resizeItem();
	void checkOverlap();
	void updateOffset();
	void updateAfterDelay(int delay);	// delay in ms

public slots:
	virtual void displayImage(QImage *);
	void panImage(QPoint delta);
	virtual void setPanning(bool panning);
	virtual void translateHorizontal(int delta);
	virtual void translateVertical(int delta);
	virtual void setHorizontalPos(int);
	virtual void setVerticalPos(int);
	virtual void setHTiling(bool);
	virtual void setVTiling(bool);
	virtual void setHScaling(float);
	virtual void setVScaling(float);
	virtual void setScaling(float, float);

	virtual void setRegionSelectionEnabled(bool enabled);

	virtual void displayPointCloud(PointCloud *);

private slots:
	void update();

signals:
    void mouseWheelUp();
    void mouseWheelDown();
};

} // namespace emd


#endif