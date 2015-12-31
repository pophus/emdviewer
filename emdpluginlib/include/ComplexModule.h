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

#ifndef EMD_COMPLEXMODULE_H
#define EMD_COMPLEXMODULE_H

#include "EmdPluginLib.h"

#include "WorkflowModule.h"

namespace emd
{

class Frame;

enum ComplexType {
	ComplexTypeReal,
	ComplexTypeImaginary,
	ComplexTypePhase,
	ComplexTypeAmplitude,
	ComplexTypeIntensity,
	ComplexTypeUnwrappedPhase,
	ComplexTypeCount
};

class EMDPLUGIN_API ComplexModule : public WorkflowModule
{
	Q_OBJECT

    EMD_MODULE_DECLARATION

public:
	ComplexModule();
	
	// WorkflowModule
	QWidget *controlWidget() override;
    void setInputContext(ProcessingContext context, WorkflowModule *previous) override;
	Frame *processFrame(Frame *frame, int index) override;

public slots:
	void setComplexType(int);

private:
	template <typename T>
	Frame *processData(Frame *frame);

    ComplexType m_complexType;
};

}

#endif