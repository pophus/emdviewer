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

#include "HistogramModule.h"

#include "ColourManager.h"
#include "Frame.h"
#include "Histogram.h"

namespace emd
{

EMD_MODULE_DEFINITION(HistogramModule)

static const float kSmallFloat = 1E-5f;

HistogramModule::HistogramModule()
    : m_histogram(nullptr)
{
    setProperty("ColourMap", "Default");
    setProperty("ScalingValues", QPointF(0.f, 1.f));
    setProperty("ScalingLocked", true);
}

// WorkflowModule functions

QWidget *HistogramModule::controlWidget() 
{
    // Histogram
    m_histogram = new Histogram();
    m_histogram->setColourMap(property("ColourMap").toString());
	connect(this, SIGNAL(histogramGenerated(QImage*, float, float)),
		m_histogram, SLOT(setImage(QImage*, float, float)));
    connect(m_histogram, SIGNAL(sizeChanged(int, int)),
        this, SLOT(setHistogramSize(int, int)));
    connect(m_histogram, SIGNAL(scalingLocked(bool)),
        this, SLOT(setScalingLocked(bool)));
    connect(m_histogram, SIGNAL(scalingValuesChanged(float, float)),
        this, SLOT(setScalingValues(float, float)));
	
	QVBoxLayout *histogramGroupLayout = new QVBoxLayout();
	histogramGroupLayout->addWidget(m_histogram);
	histogramGroupLayout->setContentsMargins(0, 0, 0, 0);
    
	QGroupBox *histogramGroupBox = new QGroupBox("Histogram");
	//histogramGroupBox->setFlat(true);
	histogramGroupBox->setLayout(histogramGroupLayout);
	histogramGroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    return histogramGroupBox;
}

void HistogramModule::doPropertyChanged(const QString &key)
{
    if(key.compare("ColourMap") == 0)
    {
        if(m_histogram)
            m_histogram->setColourMap(property(key).toString());
    }
    else
    {
        WorkflowModule::doPropertyChanged(key);
    }
}

bool HistogramModule::validate() const
{
    if(!m_histogramSize.isValid())
        return false;

    return true;
}

void HistogramModule::reset(const DataGroup *dataGroup)
{
    m_histogram->reset(dataGroup);
}

void HistogramModule::preprocess()
{
    if(m_inputContext.frameCount() > 0)
    {
        Frame *frame = m_inputContext.frameAtIndex(0);
        m_histogram->setFloatType(isFloatType(frame->dataType()));
    }
}

Frame *HistogramModule::processFrame(Frame *frame, int /*index*/)
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

void HistogramModule::postprocess()
{
    //m_histogram->setLimits(m_lowerScalingLimit, m_upperScalingLimit);

    //m_histogram->setValues(m_lowerScalingValue, m_upperScalingValue);
}

// Private:

template <typename T>
Frame *HistogramModule::processData(Frame *frame)
{
	Frame::Data<T> data = frame->data<T>();

    T dataMin, dataMax;

	frame->getDataRange(dataMin, dataMax);

	const int width = m_histogramSize.width();
	const int height = m_histogramSize.height();

	QImage *histogram = new QImage(width, height, QImage::Format_ARGB32);

	int kkk = 0;
	int offset = 0;

	if((dataMax - dataMin) < kSmallFloat)
		emit(histogramGenerated(histogram, dataMin, dataMax));
	else 
	{
		float *bins = new float[width+1];
		memset(bins, 0, width * sizeof(float));
		float rangeMult = (float) width / (dataMax - dataMin);
		float val;
		int floor;
		float delta;
		for(int iii = 0; iii < data.hSize; ++iii)
		{
			for(int jjj = 0; jjj < data.vSize; ++jjj)
			{
				val = rangeMult * (data.real[kkk] - dataMin);

				if(val >= width)
					continue;
				else if(val < 0)
					continue;
				floor = (int) val;
				delta = val - floor;
				bins[floor] += 1 - delta;
				bins[floor+1] += delta;
				kkk += data.vStep;
			}
			offset += data.hStep;
			kkk = offset;
		}

		// Find the max and mean.
		float max = 0;
        float total = 0;
		for(int iii = 0; iii < width; ++iii)
		{
			if(bins[iii] > max)
				max = bins[iii];

            total += bins[iii];
		}

        // Calculate the mean without the max so it doesn't skew the result.
        float adjustedMean = (total - max) / width;

        bool twoPhase = false;
        int mainPhaseHeight = height;
        int secondPhaseHeight = 0;
        float correctedMax = max;

        // If the max is too much larger than the adjusted mean, use a two-
        // phase histogram.
        if(max > 20 * adjustedMean)
        {
            twoPhase = true;
            mainPhaseHeight = 1 * height;
            secondPhaseHeight = height - mainPhaseHeight;
            correctedMax = 4 * adjustedMean;
        }

		histogram->fill(0x00FFFFFF);
		int colHeight, count;
		for(int iii = 0; iii < width; ++iii)
		{
			count = 0;
			colHeight = mainPhaseHeight - (int) ( bins[iii] * mainPhaseHeight / correctedMax );
            if(colHeight < 0)
                colHeight = 0;

			while(count < colHeight)
			{
				histogram->setPixel(iii, secondPhaseHeight + count, 0xFF8F8F8F);
				++count;
			}
		}

        if(twoPhase)
        {
            for(int iii = 0; iii < width; ++iii)
			{
				count = 0;
				colHeight = secondPhaseHeight - (int) ( bins[iii] * secondPhaseHeight / max );
				while(count < colHeight)
				{
					histogram->setPixel(iii, count, 0xFF4F4F4F);
					++count;
				}
			}
        }
	
		delete[] bins;

		emit(histogramGenerated(histogram, dataMin, dataMax));
	}

    return new Frame(frame->data<void>(), frame->dataType(), false);
}

/************************************* Slots ***********************************/

void HistogramModule::setHistogramSize(int width, int height)
{
    m_histogramSize = QSize(width, height);

    this->update();
}

void HistogramModule::setScalingValues(float min, float max)
{
    setProperty("ScalingValues", QPointF(min, max));
}

void HistogramModule::setScalingLocked(bool locked)
{
    setProperty("ScalingLocked", locked);
}

};
