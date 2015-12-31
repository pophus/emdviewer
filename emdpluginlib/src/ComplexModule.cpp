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

#include "ComplexModule.h"

#include <cmath>
#include <cstdint>

#include <qcombobox.h>
#include <qgroupbox.h>
#include <QVBoxLayout>

#include "Frame.h"

namespace emd
{

EMD_MODULE_DEFINITION(ComplexModule)

ComplexModule::ComplexModule()
	: m_complexType(ComplexTypeAmplitude)
{

}

// WorkflowModule

QWidget *ComplexModule::controlWidget()
{
	QComboBox *complexTypeBox = new QComboBox();
	complexTypeBox->addItem("Real");
	complexTypeBox->addItem("Imaginary");
	complexTypeBox->addItem("Phase");
	complexTypeBox->addItem("Amplitude");
	complexTypeBox->addItem("Intensity");
	complexTypeBox->addItem("Unwrapped Phase");
    complexTypeBox->setCurrentIndex(m_complexType);
	connect(complexTypeBox, SIGNAL(activated(int)),
		this, SLOT(setComplexType(int)));

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(complexTypeBox);

	QGroupBox *controlWidget = new QGroupBox("Complex");
    //controlWidget->setFlat(true);
	controlWidget->setLayout(layout);
	controlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    connect(this, SIGNAL(activityChanged(bool)),
        controlWidget, SLOT(setVisible(bool)));

	return controlWidget;
}

void ComplexModule::setInputContext(ProcessingContext context, WorkflowModule *previous)
{
    bool active = false;

    if(context.frameCount() > 0)
    {
        active = context.frameAtIndex(0)->isComplex();
    }

    setActive(active);

    WorkflowModule::setInputContext(context, previous);
}

Frame *ComplexModule::processFrame(Frame *frame, int /*index*/)
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

template <typename T>
emd::Frame *ComplexModule::processData(Frame *frame)
{
	Frame::Data<T> iData = frame->data<T>();
	Frame::Data<float> oData(iData.attributes,
                                1, iData.hSize,
                                iData.hSize, iData.vSize,
                                new float[iData.size()], NULL);

	if((oData.attributes & Frame::AttributeComplex))
	{
		oData.unsetAttribute(Frame::AttributeComplex);
	}

	int inputPos = 0, outputPos = 0;
	int inputOffset = 0, outputOffset = 0;

	switch (m_complexType)
	{
	case ComplexTypeReal:
		for(int iii = 0; iii < iData.hSize; ++iii)
		{
			for(int jjj = 0; jjj < iData.vSize; ++jjj)
			{
				oData.real[outputPos] = (float) iData.real[inputPos];
				inputPos += iData.vStep;
				outputPos += oData.vStep;
			}
			inputOffset += iData.hStep;
			outputOffset += oData.hStep;
			inputPos = inputOffset;
			outputPos = outputOffset;
		}
		break;
	case ComplexTypeImaginary:
		for(int iii = 0; iii < iData.hSize; ++iii)
		{
			for(int jjj = 0; jjj < iData.vSize; ++jjj)
			{
				oData.real[outputPos] = (float) iData.imaginary[inputPos];
				inputPos += iData.vStep;
				outputPos += oData.vStep;
			}
			inputOffset += iData.hStep;
			outputOffset += oData.hStep;
			inputPos = inputOffset;
			outputPos = outputOffset;
		}
		break;
	case ComplexTypePhase:
		for(int iii = 0; iii < iData.hSize; ++iii)
		{
			for(int jjj = 0; jjj < iData.vSize; ++jjj)
			{
				oData.real[outputPos] 
					= atan2f((float) iData.imaginary[inputPos], 
						(float) iData.real[inputPos]);
				inputPos += iData.vStep;
				outputPos += oData.vStep;
			}
			inputOffset += iData.hStep;
			outputOffset += oData.hStep;
			inputPos = inputOffset;
			outputPos = outputOffset;
		}
		break;
	case ComplexTypeAmplitude:
		for(int iii = 0; iii < iData.hSize; ++iii)
		{
			for(int jjj = 0; jjj < iData.vSize; ++jjj)
			{
				oData.real[outputPos] 
					= sqrtf( (float) iData.real[inputPos] * iData.real[inputPos] 
							+ (float) iData.imaginary[inputPos] 
							* (float) iData.imaginary[inputPos] );
				inputPos += iData.vStep;
				outputPos += oData.vStep;
			}
			inputOffset += iData.hStep;
			outputOffset += oData.hStep;
			inputPos = inputOffset;
			outputPos = outputOffset;
		}
		break;
	case ComplexTypeIntensity:
		for(int iii = 0; iii < iData.hSize; ++iii)
		{
			for(int jjj = 0; jjj < iData.vSize; ++jjj)
			{
				oData.real[outputPos] 
					= (float) iData.real[inputPos] * iData.real[inputPos] 
						+ (float) iData.imaginary[inputPos] 
						* (float) iData.imaginary[inputPos];
				inputPos += iData.vStep;
				outputPos += oData.vStep;
			}
			inputOffset += iData.hStep;
			outputOffset += oData.hStep;
			inputPos = inputOffset;
			outputPos = outputOffset;
		}
		break;
	case ComplexTypeUnwrappedPhase:
		for(int iii = 0; iii < iData.hSize; ++iii)
		{
			for(int jjj = 0; jjj < iData.vSize; ++jjj)
			{
				oData.real[outputPos] = iData.real[inputPos];
				inputPos += iData.vStep;
				outputPos += oData.vStep;
			}
			inputOffset += iData.hStep;
			outputOffset += oData.hStep;
			inputPos = inputOffset;
			outputPos = outputOffset;
		}
		break;
	default:
		break;
	}

    return new Frame(Frame::Data<void>(oData), emd::DataTypeFloat32);
}

// Slots
void ComplexModule::setComplexType(int type)
{
	if(type < 0 || type >= ComplexTypeCount)
		return;

	if(m_complexType == type)
		return;

	m_complexType = (ComplexType) type;

	this->update();
}

} // namespace emd


