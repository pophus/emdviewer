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

#include "MainImageWidget.h"

namespace emd
{

MainImageWidget::MainImageWidget(QWidget *parent)
	: QWidget(parent),
	m_displayState(0)
{
	m_scaledImage = 0;
	m_currentImage = 0;
	// Init variables
	m_hTiling = false;
	m_vTiling = false;
	m_xOffset = 0;
	m_yOffset = 0;
	m_hScale = 1.0f;
	m_vScale = 1.0f;
}

MainImageWidget::~MainImageWidget()
{
	if(m_scaledImage)
		delete m_scaledImage;
	if(m_currentImage)
		delete m_currentImage;
}

void MainImageWidget::resizeEvent(QResizeEvent *)
{
	m_displayState |= (OVERLAP_CHANGED | OFFSET_CHANGED);

	update();

    emit(viewportSizeChanged());
}

void MainImageWidget::resetOffset()
{
	m_xOffset = 0;
	m_yOffset = 0;
	m_displayState |= OFFSET_CHANGED;
}

void MainImageWidget::reset()
{
	m_xOffset = 0;
	m_yOffset = 0;
	m_hTiling = 0;
	m_vTiling = 0;
	m_hScale = 1.0f;
	m_vScale = 1.0f;
	m_displayState = SCALE_CHANGED | ITEM_SIZE_CHANGED 
					| OVERLAP_CHANGED | OFFSET_CHANGED;
}

QRect MainImageWidget::viewportRect() const
{
    return this->rect();
}

} // namespace emd

