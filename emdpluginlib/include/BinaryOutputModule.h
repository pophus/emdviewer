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

#ifndef EMD_BINARYOUTPUTMODULE_H
#define EMD_BINARYOUTPUTMODULE_H

#include "EmdPluginLib.h"

#include "WorkflowModule.h"

#include "Dataset.h"

namespace emd
{

class EMDPLUGIN_API BinaryOutputModule : public WorkflowModule
{
    Q_OBJECT

    EMD_MODULE_DECLARATION

public:
    enum OutputMode
    {
        OutputModeInvidual,
        OutputModeGrouped
    };

    enum ProcessingMode 
    {
        ProcessingModeTruncate,
        ProcessingModeScale
    };

public:
    BinaryOutputModule();

    OutputMode outputMode() const;
    void setOutputMode(OutputMode mode);

    void setProcessingMode(ProcessingMode mode);

    void setScalingLimits(const QVariant &min, const QVariant &max);

    DataType outputType() const;
    void setOutputType(DataType type);

    // WorkflowModule
    virtual void postprocess();
	virtual Frame *processFrame(Frame *frame, int index);

	virtual void doWork(WorkContext *context);

	template <typename T>
	Frame *processData(Frame *frame);

    template <typename T, typename U>
	Frame *processData(Frame *frame);

signals:
    void frameProcessed(Frame *frame);

private:
    OutputMode m_outputMode;
    ProcessingMode m_processingMode;
    DataType m_dataType;
    QVariant m_minScalingLimit;
    QVariant m_maxScalingLimit;
};

}

#endif