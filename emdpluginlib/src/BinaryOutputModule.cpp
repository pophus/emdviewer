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

#include "BinaryOutputModule.h"
#include <qthread.h>
#include "Frame.h"

namespace emd
{

EMD_MODULE_DEFINITION(BinaryOutputModule)

BinaryOutputModule::BinaryOutputModule()
    : WorkflowModule(),
    m_dataType(DataTypeFloat32),
    m_outputMode(OutputModeInvidual),
    m_processingMode(ProcessingModeTruncate),
    m_minScalingLimit(0),
    m_maxScalingLimit(1)
{

}

auto BinaryOutputModule::outputMode() const -> OutputMode
{
    return m_outputMode;
}

void BinaryOutputModule::setOutputMode(OutputMode mode)
{
    m_outputMode = mode;
}

void BinaryOutputModule::setProcessingMode(ProcessingMode mode)
{
    m_processingMode = mode;
}

void BinaryOutputModule::setScalingLimits(const QVariant &min, const QVariant &max)
{
    if(max < min)
    {
        qCritical() << "Max scaling value is less than min!";
        return;
    }

    m_minScalingLimit = min;
    m_maxScalingLimit = max;
}

DataType BinaryOutputModule::outputType() const
{
    return m_dataType;
}

void BinaryOutputModule::setOutputType(DataType type)
{
    m_dataType = type;

 //   switch (m_dataType)
 //   {
 //   case DataTypeInt8:
	//	m_minScalingLimit = m_minScalingLimit.toChar();
 //       m_maxScalingLimit = m_maxScalingLimit.toChar();
	//	break;
	//case DataTypeUInt8:
	//	m_minScalingLimit = m_minScalingLimit.toChar();
 //       m_maxScalingLimit = m_maxScalingLimit.toChar();
	//	break;
	//case DataTypeInt16:
	//	m_minScalingLimit = m_minScalingLimit.toChar();
 //       m_maxScalingLimit = m_maxScalingLimit.toChar();
	//	break;
	//case DataTypeUInt16:
	//	m_minScalingLimit = m_minScalingLimit.toChar();
 //       m_maxScalingLimit = m_maxScalingLimit.toChar();
	//	break;
	//case DataTypeInt32:
	//case DataTypeInt64:
	//	m_minScalingLimit = m_minScalingLimit.toChar();
 //       m_maxScalingLimit = m_maxScalingLimit.toChar();
	//	break;
	//case DataTypeUInt32:
	//case DataTypeUInt64:
	//	m_minScalingLimit = m_minScalingLimit.toChar();
 //       m_maxScalingLimit = m_maxScalingLimit.toChar();
	//	break;
	//case DataTypeFloat32:
	//case DataTypeFloat64:
	//	m_minScalingLimit = m_minScalingLimit.toChar();
 //       m_maxScalingLimit = m_maxScalingLimit.toChar();
	//	break;
 //   default:
 //       break;
 //   }
}

/******************************** Workflow Module **********************************/

void BinaryOutputModule::postprocess()
{
    for(int index = 0; index < m_outputContext.frameCount(); ++index)
    {
        emit(frameProcessed(m_outputContext.frameAtIndex(index)));
    }
}

void BinaryOutputModule::doWork(WorkContext *context)
{
    //if(m_outputMode == OutputModeInvidual)
    {
        WorkflowModule::doWork(context);
    }
}

Frame *BinaryOutputModule::processFrame(Frame *frame, int /*index*/)
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
    QThread::sleep(5);
    return NULL;
}

template <typename T>
Frame *BinaryOutputModule::processData(Frame *frame)
{
    switch(m_dataType)
	{
	case DataTypeInt8:
		return processData<T, int8_t>(frame);
	case DataTypeInt16:
		return processData<T, int16_t>(frame);
	case DataTypeInt32:
		return processData<T, int32_t>(frame);
	case DataTypeInt64:
		return processData<T, int64_t>(frame);
	case DataTypeUInt8:
		return processData<T, uint8_t>(frame);
	case DataTypeUInt16:
		return processData<T, uint16_t>(frame);
	case DataTypeUInt32:
		return processData<T, uint32_t>(frame);
	case DataTypeUInt64:
		return processData<T, uint64_t>(frame);
	case DataTypeFloat32:
		return processData<T, float>(frame);
	case DataTypeFloat64:
		return processData<T, double>(frame);
	default:
		break;
	}

    return NULL;
}

template <typename T, typename U>
Frame *BinaryOutputModule::processData(Frame *frame)
{
	Frame::Data<T> data = frame->data<T>();

    U *realOutput = new U[data.hSize * data.vSize];
    
    U *imaginaryOutput = NULL;
    if(data.imaginary)
        imaginaryOutput = new U[data.hSize * data.vSize];

    if(m_processingMode == ProcessingModeTruncate)
    {
	    int kkk = 0;
        int qqq = 0;
	    int offset = 0;

        if(imaginaryOutput)
        {
		    for(int jjj = 0; jjj < data.vSize; ++jjj)
		    {
			    for(int iii = 0; iii < data.hSize; ++iii)
			    {
				    realOutput[qqq] = (U) data.real[kkk];
                    imaginaryOutput[qqq] = (U) data.imaginary[kkk];
				    kkk += data.hStep;
                    ++qqq;
			    }
			    offset += data.vStep;
			    kkk = offset;
		    }
        }
        else
        {
            for(int jjj = 0; jjj < data.vSize; ++jjj)
		    {
			    for(int iii = 0; iii < data.hSize; ++iii)
			    {
				    realOutput[qqq] = (U) data.real[kkk];
				    kkk += data.hStep;
                    ++qqq;
			    }
			    offset += data.vStep;
			    kkk = offset;
		    }
        }
    }
    else if(m_processingMode == ProcessingModeScale)
    {
        T iMin, iMax;

	    frame->getDataRange(iMin, iMax);

        T iRange = iMax - iMin;
        if(iRange == 0)
            iRange = 1;

        U oMin = m_minScalingLimit.value<U>();
        U oMax = m_maxScalingLimit.value<U>();

        float oRange = oMax - oMin;
        if(oRange == 0)
            oRange = 1;

        int kkk = 0;
        int qqq = 0;
	    int offset = 0;

        if(imaginaryOutput)
        {
		    for(int jjj = 0; jjj < data.vSize; ++jjj)
		    {
			    for(int iii = 0; iii < data.hSize; ++iii)
			    {
				    realOutput[qqq] = (U) (oMin + (oRange * (data.real[kkk] - iMin)) / iRange);

                    // This might cause integer underflow/overflow depending on the
                    // types and values.
                    if(realOutput[qqq] < oMin)
                        realOutput[qqq] = oMin;
                    else if(realOutput[qqq] > oMax)
                        realOutput[qqq] = oMax;

                    imaginaryOutput[qqq] = (U) (oMin + (oRange * (data.imaginary[kkk] - iMin)) / iRange);

                    if(imaginaryOutput[qqq] < oMin)
                        imaginaryOutput[qqq] = oMin;
                    else if(imaginaryOutput[qqq] > oMax)
                        imaginaryOutput[qqq] = oMax;

				    kkk += data.hStep;
                    ++qqq;
			    }
			    offset += data.vStep;
			    kkk = offset;
		    }
        }
        else
        {
            for(int jjj = 0; jjj < data.vSize; ++jjj)
		    {
			    for(int iii = 0; iii < data.hSize; ++iii)
			    {
				    realOutput[qqq] = (U) (oMin + (oRange * (data.real[kkk] - iMin)) / iRange);

                    if(realOutput[qqq] < oMin)
                        realOutput[qqq] = oMin;
                    else if(realOutput[qqq] > oMax)
                        realOutput[qqq] = oMax;

				    kkk += data.hStep;
                    ++qqq;
			    }
			    offset += data.vStep;
			    kkk = offset;
		    }
        }
    }

    Frame *outputFrame = new Frame(realOutput, imaginaryOutput, 1, data.hSize, data.hSize, data.vSize, m_dataType);

    return outputFrame;
}

} //namespace emd