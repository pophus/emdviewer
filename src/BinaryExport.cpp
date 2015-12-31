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

#include "BinaryExport.h"

#include "Frame.h"

namespace emd
{

BinaryExport::BinaryExport(QObject *parent)
    : ExportOperation(parent),
    m_outputModule(nullptr),
    m_exportData(nullptr)
{

}

BinaryExport::~BinaryExport()
{
    if(m_outputModule)
        delete m_outputModule;

    if(m_exportData)
        delete[] m_exportData;
}

BinaryOutputModule *BinaryExport::outputModule() const
{
    return m_outputModule;
}

void BinaryExport::setOutputModule(BinaryOutputModule *module)
{
    m_outputModule = module;
    connect(m_outputModule, SIGNAL(frameProcessed(Frame *)),
        this, SLOT(saveFrameData(Frame *)));
}

void BinaryExport::doFinish()
{
    if(m_outputModule->outputMode() == BinaryOutputModule::OutputModeGrouped)
    {
        QFile file(QString(m_outputDirectory % "/" % m_fileStem % "%1").arg(m_fileSuffix));
        file.open(QIODevice::WriteOnly);

        if(!file.isOpen())
        {
            qCritical() << "Failed to open export file.";
        }
        else
        {
            QDataStream stream(&file);

            stream.writeRawData((char *) m_exportData, m_exportDataLength);
        }
    }
}

void BinaryExport::saveFrameData(Frame *frame)
{
    if(!frame)
    {
        qDebug() << "BinaryExport attempted to save empty frame.";
        return;
    }

    if(m_outputModule->outputMode() == BinaryOutputModule::OutputModeInvidual)
    {
        frame->saveRawData(fileNameForFrame(frame));
    }
    else if(m_outputModule->outputMode() == BinaryOutputModule::OutputModeGrouped)
    {
        // On the first frame, initialize the output data vector.
        if(m_exportIndex == 0)
        {
            m_exportDataLength = m_itemCount * emdTypeDepth(m_outputModule->outputType());

            if(frame->isComplex())
                m_exportDataLength *= 2;

            m_exportData = (void*) new char[m_exportDataLength];
        }

        Frame::Data<char> frameData = frame->data<char>();

        int frameSize = frameData.hSize * frameData.vSize * emdTypeDepth(frame->dataType());

        int complexCorrection = 1;
        if(frameData.imaginary)
            complexCorrection = 2;

        // TODO: better way to handle the frame index? e.g. start at zero
        int64_t offset = m_exportIndex * frameSize * complexCorrection;

        memcpy(((char*)m_exportData + offset), frameData.real, frameSize);

        if(frameData.imaginary)
            memcpy(((char*)m_exportData + offset + frameSize), frameData.imaginary, frameSize);

     //   bool descendingData = m_modelManager.currentModel()->currentDataGroup()->data()->dataOrder();
     //   int complexIndex = m_modelManager.currentModel()->currentDataGroup()->data()->complexIndex();

     //   switch(m_fileExporter->binaryDataType())
     //   {
     //   case DataTypeInt8:
		   // this->copyFrameData(*frame, (int8_t *) m_exportData, m_currentExportFrameState, m_centralWidget->dimensionsPane()->dataState(),
     //           descendingData, complexIndex);
		   // break;
	    //case DataTypeInt16:
		   // this->copyFrameData(*frame, (int16_t *) m_exportData, m_currentExportFrameState, m_centralWidget->dimensionsPane()->dataState(),
     //           descendingData, complexIndex);
		   // break;
	    //case DataTypeInt32:
		   // this->copyFrameData(*frame, (int32_t *) m_exportData, m_currentExportFrameState, m_centralWidget->dimensionsPane()->dataState(),
     //           descendingData, complexIndex);
		   // break;
	    //case DataTypeInt64:
		   // this->copyFrameData(*frame, (int64_t *) m_exportData, m_currentExportFrameState, m_centralWidget->dimensionsPane()->dataState(),
     //           descendingData, complexIndex);
		   // break;
	    //case DataTypeUInt8:
		   // this->copyFrameData(*frame, (uint8_t *) m_exportData, m_currentExportFrameState, m_centralWidget->dimensionsPane()->dataState(),
     //           descendingData, complexIndex);
		   // break;
	    //case DataTypeUInt16:
		   // this->copyFrameData(*frame, (uint16_t *) m_exportData, m_currentExportFrameState, m_centralWidget->dimensionsPane()->dataState(),
     //           descendingData, complexIndex);
		   // break;
	    //case DataTypeUInt32:
		   // this->copyFrameData(*frame, (uint32_t *) m_exportData, m_currentExportFrameState, m_centralWidget->dimensionsPane()->dataState(),
     //           descendingData, complexIndex);
		   // break;
	    //case DataTypeUInt64:
		   // this->copyFrameData(*frame, (uint64_t *) m_exportData, m_currentExportFrameState, m_centralWidget->dimensionsPane()->dataState(),
     //           descendingData, complexIndex);
		   // break;
	    //case DataTypeFloat32:
		   // this->copyFrameData(*frame, (float *) m_exportData, m_currentExportFrameState, m_centralWidget->dimensionsPane()->dataState(),
     //           descendingData, complexIndex);
		   // break;
	    //case DataTypeFloat64:
		   // this->copyFrameData(*frame, (double *) m_exportData, m_currentExportFrameState, m_centralWidget->dimensionsPane()->dataState(),
     //           descendingData, complexIndex);
		   // break;
        //}
    }

    ++m_exportIndex;
}

} // namespace emd
