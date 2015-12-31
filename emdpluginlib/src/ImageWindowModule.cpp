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

#include "ImageWindowModule.h"

#include <limits>
#include <cstdlib>

#include <qboxlayout.h>
#include <qgroupbox.h>
#include <QImage>
#include <qwidget.h>

#include "ColourManager.h"
#include "ColourMapSelector.h"
#include "Frame.h"

namespace emd
{

EMD_MODULE_DEFINITION(ImageWindowModule)

const float kSmallFloat = 1E-5f;

ImageWindowModule::ImageWindowModule()
	: m_lowerScalingValue(kInvalidFloatValue),
	m_upperScalingValue(kInvalidFloatValue),
	m_lowerScalingLimit(kInvalidFloatValue),
	m_upperScalingLimit(kInvalidFloatValue),
	m_lockedColourScaling(false),
    m_overrideColourScaling(false)
{
    setProperty("ColourMap", ColourManager::instance().defaultMap().name());
    setProperty("Output", "MainWindow");
    setProperty("ColourScalingValues", QPointF(0, 1));
    setProperty("ColourScalingLocked", true);
}

ImageWindowModule::~ImageWindowModule()
{

}

void ImageWindowModule::setColourMap(const QString &mapName)
{
    if(mapName.compare(property("ColourMap").toString()) == 0)
        return;

    setProperty("ColourMap", mapName);
}

// WorkflowModule functions

QWidget *ImageWindowModule::controlWidget()
{
    ColourMapSelector *colourMapSelector = new ColourMapSelector();
    colourMapSelector->setCurrent(property("ColourMap").toString());
    connect(colourMapSelector, SIGNAL(currentChanged(const QString &)),
        this, SLOT(setColourMap(const QString &)));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(colourMapSelector);
    //layout->setContentsMargins(0, 0, 0, 0);

	QGroupBox *colourMapGroupBox = new QGroupBox("Image Output");
	//colourMapGroupBox->setFlat(true);
	colourMapGroupBox->setLayout(layout);
	colourMapGroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    return colourMapGroupBox;
}

void ImageWindowModule::doPropertyChanged(const QString &key)
{
    if(key.compare("ColourScalingValues") == 0)
    {
        QPointF vals = property(key).toPointF();

        if(vals.x() != kInvalidFloatValue)
		    m_lowerScalingValue = vals.x();

	    if(vals.y() != kInvalidFloatValue)
		    m_upperScalingValue = vals.y();

        m_overrideColourScaling = true;

	    this->update();
    }
    //else if(key.compare("ColourMap") == 0)
    //{

    //}
    else
    {
        WorkflowModule::doPropertyChanged(key);
    }
}

void ImageWindowModule::preprocess()
{
    
}

void ImageWindowModule::postprocess()
{
    for(int index = 0; index < m_images.size(); ++index)
    {
        emit(imageGenerated(m_images.at(index)));
    }

    m_images.clear();
}

Frame *ImageWindowModule::processFrame(Frame *frame, int /*index*/)
{
	switch(frame->dataType())
	{
	case DataTypeInt8:
		return processData<int8_t>(frame);
	case DataTypeInt16:
		return processData<int16_t>(frame);
	case DataTypeInt32:
		return processData<int32_t>(frame);
	case DataTypeInt64:
		return processData<int64_t>(frame);
	case DataTypeUInt8:
		return processData<uint8_t>(frame);
	case DataTypeUInt16:
		return processData<uint16_t>(frame);
	case DataTypeUInt32:
		return processData<uint32_t>(frame);
	case DataTypeUInt64:
		return processData<uint64_t>(frame);
	case DataTypeFloat32:
		return processData<float>(frame);
	case DataTypeFloat64:
		return processData<double>(frame);
	default:
		break;
	}

    return NULL;
}

// Private

template <typename T>
Frame *ImageWindowModule::processData(Frame *frame)
{
	T min, max;

	frame->getDataRange(min, max);

	m_lowerScalingLimit = (float) min;
	m_upperScalingLimit = (float) max;

	if(m_lowerScalingValue == kInvalidFloatValue)
		m_lowerScalingValue = (float) min;

	if(m_upperScalingValue == kInvalidFloatValue)
		m_upperScalingValue = (float) max;
	
	if(m_lockedColourScaling || m_overrideColourScaling)
	{
		min = (T) m_lowerScalingValue;
		max = (T) m_upperScalingValue;

        m_overrideColourScaling = false;
	}
    else
    {
        m_lowerScalingValue = (float) min;
        m_upperScalingValue = (float) max;
    }

	float range = (float) (max - min);

	Frame::Data<T> data = frame->data<T>();

	int xStep, yStep;
	int xSize, ySize;
	if(!m_inputContext.axesFlipped())
	{
		xStep = data.hStep;
		yStep = data.vStep;
		xSize = data.hSize;
		ySize = data.vSize;
	}
	else
	{
		xStep = data.vStep;
		yStep = data.hStep;
		xSize = data.vSize;
		ySize = data.hSize;
	}
    
	QImage *image = new QImage(xSize, ySize, QImage::Format_RGB32);

    ColourMap map = ColourManager::instance().colourMap(property("ColourMap").toString());
	const QRgb *colourTable = map.colourTable();
	int colourRange = map.colourTableRange() - 1;

	int kkk = 0;
	int offset = 0;
	// If we have a non-zero image, fill in the pixels normally
	if(range > kSmallFloat)
	{
		float rangeMult = (float) colourRange / range;
		int val;

		for(int iii = 0; iii < xSize; ++iii)
		{
			for(int jjj = 0; jjj < ySize; ++jjj)
			{
				val = (int) (rangeMult * (data.real[kkk] - min));

				if(val > colourRange)
					val = colourRange;
				else if(val < 0)
					val = 0;

				image->setPixel(iii, jjj, colourTable[val]);
				kkk += yStep;
			}
			offset += xStep;
			kkk = offset;
		}
	}
	// If the range is effectively zero, gate the pixels
	else
	{
		uint minColour = colourTable[0];
		uint maxColour = colourTable[colourRange];
		uint colour;
		for(int iii = 0; iii < xSize; ++iii)
		{
			for(int jjj = 0; jjj < ySize; ++jjj)
			{
				if(data.real[kkk] < min)
					colour = minColour;
				else
					colour = maxColour;
				image->setPixel(iii, jjj, colour);
				kkk += yStep;
			}
			offset += xStep;
			kkk = offset;
		}
	}

    m_images.push_back(image);

    return new Frame(frame);
}

} // namespace emd



