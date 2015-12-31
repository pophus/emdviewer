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

#include "ImageWidget.h"

#include "Util.h"

namespace emd
{

ImageWidget::ImageWidget(QWidget *parent)
	: QWidget(parent),
	m_currentImage(NULL),
	m_xPosition(0), m_yPosition(0),
	m_xTiled(false), m_yTiled(false),
	m_xScale(1.0f), m_yScale(1.0f),
	m_displayState(DisplayStateScaleChanged)
{

}

ImageWidget::~ImageWidget()
{
	if(m_currentImage)
		delete m_currentImage;
}

void ImageWidget::reset()
{
	m_xPosition = 0;
	m_yPosition = 0;

	m_xTiled = false;
	m_yTiled = false;

	m_xScale = 1.0f;
	m_yScale = 1.0f;

	m_displayState = DisplayStateScaleChanged | DisplayStateTilingChanged 
					| DisplayStatePositionChanged | DisplayStateImageChanged;
}

void ImageWidget::setPosition(float xPosition, float yPosition)
{
	if(xPosition != kInvalidFloatValue)
		m_xPosition = xPosition;

	if(yPosition != kInvalidFloatValue)
		m_yPosition = yPosition;

	m_displayState |= DisplayStatePositionChanged;

	update();
}
	
void ImageWidget::setTiling(bool xTiled, bool yTiled)
{
	m_xTiled = xTiled;
	m_yTiled = yTiled;

	m_displayState |= DisplayStateTilingChanged;

	update();
}
	
void ImageWidget::setScale(float xScale, float yScale)
{
	if(xScale != kInvalidFloatValue)
		m_xScale = xScale;

	if(yScale !=  kInvalidFloatValue)
		m_yScale = yScale;

	m_displayState |= DisplayStateScaleChanged;

	update();
}

} // namespace emd


