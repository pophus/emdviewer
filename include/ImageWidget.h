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

#ifndef EMD_IMAGEWIDGET_H
#define EMD_IMAGEWIDGET_H

#include <QtGui>
#include <QtWidgets>

namespace emd
{

extern const float ImageWidgetInvalidValue;

class ImageWidget : public QWidget
{
	Q_OBJECT

public:
	ImageWidget(QWidget *parent = 0);
	virtual ~ImageWidget();

	// Subclass-required functions
	virtual void update() = 0;

	// Subclass-optional functions
	virtual void reset();

protected:
	QImage *m_currentImage;

	// Display parameters
	int m_xPosition;
	int m_yPosition;

	bool m_xTiled;
	bool m_yTiled;

	float m_xScale;
	float m_yScale;

	int m_displayState;

	enum DisplayState {
		DisplayStateScaleChanged		= 0x01,
		DisplayStateTilingChanged		= 0x02,
		DisplayStatePositionChanged		= 0x04,
		DisplayStateImageChanged		= 0x08
	};

public slots:
	// Subclass-required slots ---------------------------------------
	virtual void displayImage(QImage *) = 0;

	// Subclass-optional slots ---------------------------------------
	// The default implementation updates the appropriate parameters
	// and then calls update().

	// Pass ImageWidgetInvalidValue to leave the current value.
	virtual void setPosition(float xPosition, float yPosition);

	virtual void setTiling(bool xTiled, bool yTiled);

	// Pass ImageWidgetInvalidValue to leave the current value.
	virtual void setScale(float xScale, float yScale);
};

} // namespace emd

#endif