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

#ifndef EMD_MAINIMAGEWIDGET_H
#define EMD_MAINIMAGEWIDGET_H

#include <QtGui>
#include <QtWidgets>

#include "PointCloud.h"

class QImage;

namespace emd
{

class Frame;

class MainImageWidget : public QWidget
{
	Q_OBJECT

public:
	MainImageWidget(QWidget *parent = 0);
	virtual ~MainImageWidget();

    // QWidget
	void resizeEvent(QResizeEvent *e);

    // Subclass-optional functions
	virtual void resetOffset();
	virtual void reset();

    virtual QRect viewportRect() const;

	// Subclass-required functions
	virtual void setRegionSelection(const QPolygonF &selection) = 0;
	virtual QPolygonF regionSelection() const = 0;

    virtual QImage *windowImage() const = 0;

protected:
	QImage *m_currentImage;
	QImage *m_scaledImage;

	// Display parameters
	bool m_hTiling;
	bool m_vTiling;
	bool m_overlapped;
	int m_xOffset;
	int m_yOffset;
	float m_hScale;
	float m_vScale;
	int m_displayState;

	enum DisplayState {
		SCALE_CHANGED		= 0x01,
		ITEM_SIZE_CHANGED	= 0x02,
		OVERLAP_CHANGED		= 0x04,
		OFFSET_CHANGED		= 0x08
	};

public slots:
	virtual void displayImage(QImage *) {}
	virtual void setPanning(bool) {}
	virtual void translateHorizontal(int) {}
	virtual void translateVertical(int) {}
	virtual void setHorizontalPos(int) {}
	virtual void setVerticalPos(int) {}
	virtual void setHTiling(bool) {}
	virtual void setVTiling(bool) {}
	virtual void setHScaling(float) {}
	virtual void setVScaling(float) {}
	virtual void setScaling(float, float) {}

	virtual void setRegionSelectionEnabled(bool) {}

	virtual void displayPointCloud(PointCloud *) {}

signals:
	void imageHPos(float min, float max);
	void imageVPos(float min, float max);

    void viewportSizeChanged();
	void cursorPositionChanged(float x, float y);
};

} // namespace emd


#endif