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

#include "Histogram.h"

#include <qpushbutton.h>

#include "ColourManager.h"
#include "ColourMap.h"
#include "DataGroup.h"
#include "HistogramView.h"
#include "NumberRangeWidget.h"
#include "Util.h"

namespace emd
{

Histogram::Histogram(QWidget *parent)
	: QWidget(parent),
	m_scene(this),
    m_hasFirstImage(false)
{
	this->setContentsMargins(0, 0, 0, 0);

	m_scene.setBackgroundBrush(QBrush(QColor(220, 220, 220)));
	connect(&m_scene, SIGNAL(scalingLimitsChanged(float, float)), 
		this, SLOT(changeScalingValuesFromHistogram(float, float)));

    m_view = new HistogramView();
	m_view->setScene(&m_scene);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setFrameShape(QFrame::NoFrame);
    //m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_view, SIGNAL(sizeChanged(int, int)),
        this, SIGNAL(sizeChanged(int, int)));

	m_scalingBoxes = new NumberRangeWidget();
	m_scalingBoxes->setValues(0, 1);
    // TODO: fix for int
    m_scalingBoxes->setType(true);
	connect(m_scalingBoxes, SIGNAL(limitsChanged(float, float)),
		this, SLOT(changeScalingValuesFromBoxes(float, float)));

	m_autoScalingButton = new QPushButton("Auto");
	m_autoScalingButton->setCheckable(true);
	connect(m_autoScalingButton, SIGNAL(toggled(bool)),
		this, SLOT(setAutoScaling(bool)));

	QHBoxLayout *scalingBoxLayout = new QHBoxLayout();
	scalingBoxLayout->addStretch();
	scalingBoxLayout->addWidget(m_autoScalingButton);
	scalingBoxLayout->addWidget(m_scalingBoxes);
	scalingBoxLayout->addStretch();

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_view);
    layout->addLayout(scalingBoxLayout);
    this->setLayout(layout);

    this->setImage(NULL, 0, 1);
}

Histogram::~Histogram()
{
    
}

void Histogram::reset(const DataGroup *dataGroup)
{
    if(dataGroup)
    {
        if(isFloatType(dataGroup->type()))
            m_scalingBoxes->setType(true);
        else
            m_scalingBoxes->setType(false);
    }
    else
    {
        m_scalingBoxes->setValues(0, 1);

        m_scene.setScalingLimits(0, 1);
        m_scene.setScalingValues(0, 1);

        this->setImage(NULL, 0, 1);
    }
}

void Histogram::setLimits(const float &lower, const float &upper)
{
	m_scene.setScalingLimits(lower, upper);

    if(m_autoScalingButton->isChecked())
    {
        m_scene.setScalingValues(lower, upper);
        m_scalingBoxes->setValues(lower, upper);
    }
}

void Histogram::setValues(float lower, float upper)
{
    m_scene.setScalingValues(lower, upper);

    m_scalingBoxes->setValues(lower, upper);
}

void Histogram::setFloatType(bool floatType)
{
    m_scalingBoxes->setType(floatType);
}

/********************************** Slots ************************************/

void Histogram::setImage(QImage *image, float min, float max)
{
    if(!image)
    {
	    m_scene.setBackgroundBrush(QBrush(QColor(220, 220, 220)));

        if(m_hasFirstImage)
            m_autoScalingButton->setChecked(true);

        this->setEnabled(false);
    }
    else
    {
        if(!m_hasFirstImage)
        {
            m_hasFirstImage = true;

            m_autoScalingButton->setChecked(true);
        }

        this->setEnabled(true);

	    QSize size = image->size();
	    QSize sceneSize = m_view->size();
	    float dx = (sceneSize.width() - size.width()) / 2;
	    float dy = (sceneSize.height() - size.height()) / 2;
	    m_scene.setSceneRect(-dx, -dy - 1, sceneSize.width(), sceneSize.height());
    }
    
    this->setLimits(min, max);

	m_scene.setImage(image);
}

void Histogram::setColourMap(const QString &mapName)
{
    m_scene.setGradient(ColourManager::instance().colourMap(mapName).gradient());
}

void Histogram::setHistogramSize(int width, int height)
{
    if(!this->isEnabled())
    {
        m_scene.setImage(NULL);
    }
    else
    {
        emit(sizeChanged(width, height));
    }
}

void Histogram::setAutoScaling(bool autoScaling)
{
    emit(scalingLocked(!autoScaling));
}

void Histogram::changeScalingValuesFromBoxes(float lower, float upper)
{
    if(m_autoScalingButton->isChecked())
    {
        m_autoScalingButton->setChecked(false);
    }

	m_scene.setScalingValues(lower, upper);

	emit(scalingValuesChanged(lower, upper));
}

void Histogram::changeScalingValuesFromHistogram(float lower, float upper)
{
    if(m_autoScalingButton->isChecked())
    {
        m_autoScalingButton->setChecked(false);
    }

	m_scalingBoxes->setValues(lower, upper);

	emit(scalingValuesChanged(lower, upper));
}

} // namespace emd
