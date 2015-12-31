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

#include "IntegrationModule.h"

#include <stdint.h>

#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <QVBoxLayout>

#include "Frame.h"
#include "DataGroup.h"

EMD_MODULE_DEFINITION(IntegrationModule)

IntegrationModule::IntegrationModule()
    : m_resultFrame(nullptr),
    m_integratedFrameCount(0),
    m_dimensionLayout(nullptr),
    m_dimensionGroup(nullptr),
    m_selectionDims(0)
{

}

QWidget *IntegrationModule::controlWidget()
{
    m_dimensionLayout = new QVBoxLayout();
    m_dimensionLayout->setSpacing(0);

    addDimensionButtons();

    QGroupBox *control = new QGroupBox("Integration");
    control->setFlat(true);
    control->setLayout(m_dimensionLayout);

    return control;
}

void IntegrationModule::doPropertyChanged(const QString &key)
{
    if(key.compare("DataGroup") == 0)
    {
        addDimensionButtons();
    } 
    else if(key.compare("DataState") == 0)
    {
        
    }
}

void IntegrationModule::addDimensionButtons()
{
    if(!m_dimensionLayout)
            return;

    const emd::DataGroup *dataGroup = property("DataGroup").value<const emd::DataGroup *>();
    if(!dataGroup)
        return;

    m_dimensionGroup = new QButtonGroup();
    m_dimensionGroup->setExclusive(false);
    connect(m_dimensionGroup, SIGNAL(buttonClicked(int)),
        this, SLOT(dimensionClicked(int)));

    for(int index = 0; index < dataGroup->dimCount(); ++index)
    {
        const emd::Dataset *dim = dataGroup->dimData(index);
        QRadioButton *button = new QRadioButton(dim->dataName());
        button->setCheckable(true);
        m_dimensionGroup->addButton(button, index);
        m_dimensionLayout->addWidget(button);

        if(dim->isComplexDim())
            button->hide();

        if(((1 << index) & m_availableDims) == 0)
            button->setEnabled(false);
    }
}

void IntegrationModule::setSelectionDimensions(int dims)
{
    m_availableDims = dims;

    if(m_dimensionGroup)
    {
        int opt = dims | m_selectionDims;

        for(int index = 0; index < m_dimensionGroup->buttons().count(); ++index)
        {
            m_dimensionGroup->button(index)->setEnabled(((1 << index) & opt) != 0); 
        }
    }
}

void IntegrationModule::dimensionClicked(int index)
{
    bool checked = m_dimensionGroup->button(index)->isChecked();

    if(checked)
    {
        m_selectionDims |= (1 << index);
    }
    else
    {
        m_selectionDims &= ~(1 << index);
    }

    emit(dimensionSelectionRequested(index, checked));
}

void IntegrationModule::reset()
{
    WorkflowModule::reset();

    m_lastInputContext.reset();

    if(m_resultFrame)
    {
        delete m_resultFrame;
        m_resultFrame = NULL;
    }

    m_integratedFrameCount = 0;
}

emd::WorkflowModule::RequiredFeatures IntegrationModule::requiredFeatures() const
{
    return RangeSelectionFeature;
}

void IntegrationModule::preprocess()
{
    emd::FrameList framesToAdd;
    emd::FrameList framesToSubtract;

    if(m_lastInputContext.isValid())
    {
        m_inputContext.frameSet()->subtract(*m_lastInputContext.frameSet(), framesToAdd);

        m_lastInputContext.frameSet()->subtract(*m_inputContext.frameSet(), framesToSubtract);
        
        if(framesToSubtract.size() > 0)
        {
            // If we're subtracting frames, preserve the context which contains
            // them so that they live long enough for the work to be done.
            m_subtractionContext = m_lastInputContext;
        }
    
        m_lastInputContext = m_inputContext;

        m_inputContext.reset();
        m_inputContext.init((int)(framesToAdd.size() + framesToSubtract.size()));

        // Copy the frames before setting them in the new input context.
        // This avoids having the same frames referenced by two different
        // processing contexts which would both try to delete them.
        // The copied frames don't own the data, so the frame data are
        // not deleted until m_lastInputContext is reset.
        int index = 0;
        while(index < framesToAdd.size())
        {
            m_inputContext.setFrameAtIndex(new emd::Frame(framesToAdd.at(index)), index);

            ++index;
        }

        while(index < m_inputContext.frameCount())
        {
            m_inputContext.setFrameAtIndex(new emd::Frame(framesToSubtract.at(index - framesToAdd.size())), index);

            ++index;
        }

        m_cutoffInputIndex = (int) framesToAdd.size();
    }
    else
    {
        m_cutoffInputIndex = (int) m_inputContext.frameCount();
        
        m_lastInputContext = m_inputContext;
    }

    if(!m_resultFrame && m_inputContext.frameCount() > 0)
    {
        emd::Frame *frame = m_inputContext.frameAtIndex(0);
        emd::Frame::Data<void> data = frame->data<void>();
        int size = data.size();

        float *real = new float[size];
        memset(real, 0, 4 * size);

        float *imag = NULL;
        if(frame->isComplex())
        {
            imag = new float[size];
            memset(imag, 0, 4 * size);
        }

        m_resultFrame = new emd::Frame(real, imag, 1, data.hSize, data.hSize, data.vSize, emd::DataTypeFloat32);

        m_resultFrame->setIndex(0);
    }
}

emd::Frame *IntegrationModule::processFrame(emd::Frame *frame, int index)
{
    if(index < m_cutoffInputIndex)
    {
	    switch(frame->dataType())
	    {
	    case emd::DataTypeInt8:
		    addFrame<int8_t>(frame);
		    break;
	    case emd::DataTypeInt16:
		    addFrame<int16_t>(frame);
		    break;
	    case emd::DataTypeInt32:
		    addFrame<int32_t>(frame);
		    break;
	    case emd::DataTypeInt64:
		    addFrame<int64_t>(frame);
		    break;
	    case emd::DataTypeUInt8:
		    addFrame<uint8_t>(frame);
		    break;
	    case emd::DataTypeUInt16:
		    addFrame<uint16_t>(frame);
		    break;
	    case emd::DataTypeUInt32:
		    addFrame<uint32_t>(frame);
		    break;
	    case emd::DataTypeUInt64:
		    addFrame<uint64_t>(frame);
		    break;
	    case emd::DataTypeFloat32:
		    addFrame<float>(frame);
		    break;
	    case emd::DataTypeFloat64:
		    addFrame<double>(frame);
		    break;
	    default:
		    break;
	    }
    }
    else
    {
        switch(frame->dataType())
	    {
	    case emd::DataTypeInt8:
		    subtractFrame<int8_t>(frame);
		    break;
	    case emd::DataTypeInt16:
		    subtractFrame<int16_t>(frame);
		    break;
	    case emd::DataTypeInt32:
		    subtractFrame<int32_t>(frame);
		    break;
	    case emd::DataTypeInt64:
		    subtractFrame<int64_t>(frame);
		    break;
	    case emd::DataTypeUInt8:
		    subtractFrame<uint8_t>(frame);
		    break;
	    case emd::DataTypeUInt16:
		    subtractFrame<uint16_t>(frame);
		    break;
	    case emd::DataTypeUInt32:
		    subtractFrame<uint32_t>(frame);
		    break;
	    case emd::DataTypeUInt64:
		    subtractFrame<uint64_t>(frame);
		    break;
	    case emd::DataTypeFloat32:
		    subtractFrame<float>(frame);
		    break;
	    case emd::DataTypeFloat64:
		    subtractFrame<double>(frame);
		    break;
	    default:
		    break;
	    }
    }

    return NULL;
}

void IntegrationModule::postprocess()
{
    m_subtractionContext.reset();

    if(m_lastInputContext.frameCount() != m_integratedFrameCount)
    {
        qCritical() << "Integrated frame vector size mismatch!";
        return;
    }

    m_outputContext.reset();

    m_outputContext.init(1);

    m_outputContext.setFrameAtIndex(new emd::Frame(m_resultFrame), 0);
}

template <typename T>
void IntegrationModule::addFrame(emd::Frame *frame)
{
    emd::Frame::Data<T> iData = frame->data<T>();
	emd::Frame::Data<T> oData(iData.attributes,
                                1, iData.hSize,
                                iData.hSize, iData.vSize,
                                new T[iData.size()], 
                                iData.imaginary ? new T[iData.size()] : NULL);

    emd::Frame::Data<float> rData = m_resultFrame->data<float>();
    
	int inputPos = 0, outputPos = 0;
	int inputOffset = 0, outputOffset = 0;

    if(frame->isComplex())
    {
	    for(int iii = 0; iii < iData.hSize; ++iii)
	    {
		    for(int jjj = 0; jjj < iData.vSize; ++jjj)
		    {
                rData.real[outputPos] = (float)(((double)m_integratedFrameCount * rData.real[outputPos] 
                                         + iData.real[inputPos])
                                             / (m_integratedFrameCount + 1));

                rData.imaginary[outputPos] = (float)(((double)m_integratedFrameCount * rData.imaginary[outputPos] 
                                         + iData.imaginary[inputPos])
                                             / (m_integratedFrameCount + 1));

			    oData.real[outputPos] = iData.real[inputPos];
                oData.imaginary[outputPos] = iData.imaginary[inputPos];

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
                rData.real[outputPos] = (float)(((double)m_integratedFrameCount * rData.real[outputPos] 
                                         + iData.real[inputPos])
                                             / (m_integratedFrameCount + 1));

			    oData.real[outputPos] = iData.real[inputPos];

			    inputPos += iData.vStep;
			    outputPos += oData.vStep;
		    }

		    inputOffset += iData.hStep;
		    outputOffset += oData.hStep;

		    inputPos = inputOffset;
		    outputPos = outputOffset;
	    }
    }

    ++m_integratedFrameCount;
}

template <typename T>
void IntegrationModule::subtractFrame(emd::Frame *frame)
{
    emd::Frame::Data<T> iData = frame->data<T>();
    
    emd::Frame::Data<float> rData = m_resultFrame->data<float>();

	int inputPos = 0, outputPos = 0;
	int inputOffset = 0, outputOffset = 0;

    if(frame->isComplex())
    {
	    for(int iii = 0; iii < iData.hSize; ++iii)
	    {
		    for(int jjj = 0; jjj < iData.vSize; ++jjj)
		    {
                rData.real[outputPos] = (float)(((double)m_integratedFrameCount * rData.real[outputPos] 
                                         - iData.real[inputPos])
                                             / (m_integratedFrameCount - 1));

                rData.imaginary[outputPos] = (float)(((double)m_integratedFrameCount * rData.imaginary[outputPos] 
                                             - iData.imaginary[inputPos])
                                                / (m_integratedFrameCount - 1));

			    inputPos += iData.vStep;
			    outputPos += rData.vStep;
		    }

		    inputOffset += iData.hStep;
		    outputOffset += rData.hStep;

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
                rData.real[outputPos] = (float)(((double)m_integratedFrameCount * rData.real[outputPos] 
                                         - iData.real[inputPos])
                                             / (m_integratedFrameCount - 1));

			    inputPos += iData.vStep;
			    outputPos += rData.vStep;
		    }

		    inputOffset += iData.hStep;
		    outputOffset += rData.hStep;

		    inputPos = inputOffset;
		    outputPos = outputOffset;
	    }
    }
        
    --m_integratedFrameCount;
}
