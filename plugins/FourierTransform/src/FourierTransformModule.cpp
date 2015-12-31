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

#include "FourierTransformModule.h"

#include <stdint.h>

#include <kiss_fftnd.h>

#include <qbuttongroup.h>
#include <QDebug>
#include <qgroupbox.h>
#include <QPushButton>
#include <QVBoxLayout>

#include "Frame.h"

namespace emd
{

EMD_MODULE_DEFINITION(FourierTransformModule)

FourierTransformModule::FourierTransformModule()
{
    setProperty("TransformType", "None");
    setProperty("DataShift", "false");
}

// WorkflowModule

QWidget *FourierTransformModule::controlWidget()
{
    QPushButton *noneButton = new QPushButton("None");
    noneButton->setCheckable(true);
    noneButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

	QPushButton *forwardButton = new QPushButton("Forward");
    forwardButton->setCheckable(true);
    forwardButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

	QPushButton *reverseButton = new QPushButton("Reverse");
    reverseButton->setCheckable(true);
    reverseButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    if(property("TransformType").toString().compare("None") == 0)
        noneButton->setChecked(true);
    else if(property("TransformType").toString().compare("Forward") == 0)
        forwardButton->setChecked(true);
    else if(property("TransformType").toString().compare("Reverse") == 0)
        reverseButton->setChecked(true);

    QButtonGroup *buttonGroup = new QButtonGroup();
    buttonGroup->addButton(noneButton);
    buttonGroup->addButton(forwardButton);
    buttonGroup->addButton(reverseButton);
    buttonGroup->setId(noneButton, TransformTypeNone);
    buttonGroup->setId(forwardButton, TransformTypeForward);
    buttonGroup->setId(reverseButton, TransformTypeReverse);
    connect(buttonGroup, SIGNAL(buttonClicked(int)),
        this, SLOT(setTransformType(int)));

	QHBoxLayout *layout = new QHBoxLayout();
	//layout->addStretch();
	layout->addWidget(noneButton);
	layout->addWidget(forwardButton);
	layout->addWidget(reverseButton);
	//layout->addStretch();

	QGroupBox *controlWidget = new QGroupBox("Fourier Transform");
    controlWidget->setFlat(true);
	controlWidget->setLayout(layout);
    controlWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    return controlWidget;
}

Frame *FourierTransformModule::processFrame(Frame *frame, int /*index*/)
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
Frame *FourierTransformModule::processData(Frame *frame)
{
	Frame::Data<T> iData = frame->data<T>();
	Frame::Data<float> oData(iData.attributes,
                                1, iData.hSize,
                                iData.hSize, iData.vSize,
                                new float[iData.size()], 
                                new float[iData.size()]);

    if(property("TransformType").toString().compare("Forward") == 0)
	{
		if(property("DataShift").toBool())
			oData.setAttribute(Frame::AttributeFourierTransformed);
		else
			oData.setAttribute(Frame::AttributeFourierTransformedNoShift);

		int inputPos = 0, outputPos = 0;
		int inputOffset = 0, outputOffset = 0;

		kiss_fft_cpx* freqData = new kiss_fft_cpx[iData.hSize * iData.vSize];
		kiss_fft_cpx* timeData = new kiss_fft_cpx[iData.hSize * iData.vSize];

        // TODO: better way?
        if(iData.imaginary)
        {
		    for(int iii = 0; iii < iData.hSize; ++iii)
		    {
			    for(int jjj = 0; jjj < iData.vSize; ++jjj)
			    {
				    timeData[outputPos].r = (float) iData.real[inputPos];
				    timeData[outputPos].i = (float) iData.imaginary[inputPos];
				    inputPos += iData.vStep;
				    outputPos += oData.vStep;
			    }
			    inputOffset += iData.hStep;
			    outputOffset += oData.hStep;
			    inputPos = inputOffset;
			    outputPos = outputOffset;
		    }
        }
        else
        {
            for(int iii = 0; iii < iData.hSize; ++iii)
		    {
			    for(int jjj = 0; jjj < iData.vSize; ++jjj)
			    {
				    timeData[outputPos].r = (float) iData.real[inputPos];
				    timeData[outputPos].i = 0.f;
				    inputPos += iData.vStep;
				    outputPos += oData.vStep;
			    }
			    inputOffset += iData.hStep;
			    outputOffset += oData.hStep;
			    inputPos = inputOffset;
			    outputPos = outputOffset;
		    }
        }

		int* dims = new int[2];
		dims[1] = iData.hSize;
		dims[0] = iData.vSize;
		kiss_fftnd_cfg cfg = kiss_fftnd_alloc(dims, 2, 0, NULL, NULL);

		kiss_fftnd(cfg, timeData, freqData);

		if(property("DataShift").toBool())
		{
			int k = 0;
			int vHalf = oData.vSize / 2;
			int hHalf = oData.hSize / 2;
			for(int jjj = 0; jjj < oData.vSize; jjj++)
			{
				int j = jjj;
				if(j > vHalf - 1)
					j -= vHalf;
				else
					j += vHalf;
				for(int iii = 0; iii < oData.hSize; iii++)
				{
					int i = iii;
					if(i > hHalf - 1)
						i -= hHalf;
					else
						i += hHalf;
					int ij = j*oData.hSize + i;

					oData.real[ij] = freqData[k].r;
					oData.imaginary[ij] = freqData[k].i;

					k++;
				}
			}
		}
		else
		{
			int k = 0;
			for(int jjj = 0; jjj < oData.vSize; jjj++)
			{
				for(int iii = 0; iii < oData.hSize; iii++)
				{
					int ij = jjj*oData.hSize + iii;

					oData.real[ij] = freqData[k].r;
					oData.imaginary[ij] = freqData[k].i;

					k++;
				}
			}
		}

		delete[] freqData;
		delete[] timeData;
		delete[] dims;
	}
	else if(property("TransformType").toString().compare("Reverse") == 0)
	{
		oData.unsetAttribute(Frame::AttributeFourierTransformed);
		oData.unsetAttribute(Frame::AttributeFourierTransformedNoShift);

		int inputPos = 0, outputPos = 0;
		int inputOffset = 0, outputOffset = 0;

		kiss_fft_cpx* freqData = new kiss_fft_cpx[iData.hSize * iData.vSize];
		kiss_fft_cpx* timeData = new kiss_fft_cpx[iData.hSize * iData.vSize];

        if(iData.imaginary)
        {
		    for(int iii = 0; iii < iData.hSize; ++iii)
		    {
			    for(int jjj = 0; jjj < iData.vSize; ++jjj)
			    {
				    freqData[outputPos].r = (float) iData.real[inputPos];
				    freqData[outputPos].i = (float) iData.imaginary[inputPos];
				    inputPos += iData.vStep;
				    outputPos += oData.vStep;
			    }
			    inputOffset += iData.hStep;
			    outputOffset += oData.hStep;
			    inputPos = inputOffset;
			    outputPos = outputOffset;
		    }
        }
        else
        {
            for(int iii = 0; iii < iData.hSize; ++iii)
		    {
			    for(int jjj = 0; jjj < iData.vSize; ++jjj)
			    {
				    freqData[outputPos].r = (float) iData.real[inputPos];
				    freqData[outputPos].i = 0.f;
				    inputPos += iData.vStep;
				    outputPos += oData.vStep;
			    }
			    inputOffset += iData.hStep;
			    outputOffset += oData.hStep;
			    inputPos = inputOffset;
			    outputPos = outputOffset;
		    }
        }

		int* dims = new int[2];
		dims[1] = iData.hSize;
		dims[0] = iData.vSize;
		kiss_fftnd_cfg cfg = kiss_fftnd_alloc(dims, 2, 1, NULL, NULL);

		kiss_fftnd(cfg, freqData, timeData);

		float magnitudeCorrection = 1.f / (oData.hSize * oData.vSize);

		if(property("DataShift").toBool())
		{
			int k = 0;
			int vHalf = oData.vSize / 2;
			int hHalf = oData.hSize / 2;
			for(int jjj = 0; jjj < oData.vSize; jjj++)
			{
				int j = jjj;
				if(j > vHalf - 1)
					j -= vHalf;
				else
					j += vHalf;
				for(int iii = 0; iii < oData.hSize; iii++)
				{
					int i = iii;
					if(i > hHalf - 1)
						i -= hHalf;
					else
						i += hHalf;
					int ij = j*oData.hSize + i;

					oData.real[ij] = timeData[k].r;
					oData.imaginary[ij] = timeData[k].i;

					k++;
				}
			}
		}
		else
		{
			int k = 0;
			for(int jjj = 0; jjj < oData.vSize; jjj++)
			{
				for(int iii = 0; iii < oData.hSize; iii++)
				{
					oData.real[k] = timeData[k].r * magnitudeCorrection;
					oData.imaginary[k] = timeData[k].i * magnitudeCorrection;

					k++;
				}
			}
		}

		delete[] freqData;
		delete[] timeData;
		delete[] dims;
	}
	//else if(m_shift)
	//{
	//	int k = 0;
	//	int vHalf = oData.vSize / 2;
	//	int hHalf = oData.hSize / 2;

	//	// TODO: replace duplicate code with templated function
    //  // TODO: input data indexes are wrong if this is the first processing module
	//	if(iData.imaginary)
	//	{
	//		for(int jjj = 0; jjj < oData.vSize; jjj++)
	//		{
	//			int j = jjj;
	//			if(j > vHalf - 1)
	//				j -= vHalf;
	//			else
	//				j += vHalf;
	//			for(int iii = 0; iii < oData.hSize; iii++)
	//			{
	//				int i = iii;
	//				if(i > hHalf - 1)
	//					i -= hHalf;
	//				else
	//					i += hHalf;
	//				int ij = j*oData.hSize + i;

	//				oData.real[ij] = iData.real[k];
	//				oData.imaginary[ij] = iData.imaginary[k];

	//				k++;
	//			}
	//		}
	//	}
	//	else
	//	{
	//		for(int jjj = 0; jjj < oData.vSize; jjj++)
	//		{
	//			int j = jjj;
	//			if(j > vHalf - 1)
	//				j -= vHalf;
	//			else
	//				j += vHalf;
	//			for(int iii = 0; iii < oData.hSize; iii++)
	//			{
	//				int i = iii;
	//				if(i > hHalf - 1)
	//					i -= hHalf;
	//				else
	//					i += hHalf;
	//				int ij = j*oData.hSize + i;

	//				oData.real[ij] = iData.real[k];

	//				k++;
	//			}
	//		}
	//	}
	//}
    else
    {
        // Just copy the data
        delete[] oData.real;
        delete[] oData.imaginary;

        return new Frame(iData, DataTypeFloat32, false);
    }

    return new Frame(Frame::Data<void>(oData), emd::DataTypeFloat32);
}

/************************ Slots ****************************/

void FourierTransformModule::setTransformType(int type)
{
    switch (type)
    {
    case TransformTypeForward:
        setProperty("TransformType", "Forward");
        break;
    case TransformTypeReverse:
        setProperty("TransformType", "Reverse");
        break;
    case TransformTypeNone:
        setProperty("TransformType", "None");
    default:
        break;
    }
}

} // namespace emd

