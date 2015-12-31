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

#ifndef EMD_INTEGRATIONMODULE_H
#define EMD_INTEGRATIONMODULE_H

#include "WorkflowModule.h"

#include <qlist.h>
#include <qmap.h>

#include "Frame.h"

class QButtonGroup;
class QVBoxLayout;

class IntegrationModule : public emd::WorkflowModule
{
	Q_OBJECT

    EMD_MODULE_DECLARATION

public:
	IntegrationModule();

	void setPhasePlateModule(WorkflowModule *module);
	
	// WorkflowModule
    QWidget *controlWidget() override;
    void doPropertyChanged(const QString &key) override;
    void reset();
    RequiredFeatures requiredFeatures() const override;
	void preprocess() override;
	emd::Frame *processFrame(emd::Frame *frame, int index) override;
    void postprocess() override;

private:
	template <typename T>
	void addFrame(emd::Frame *frame);
	template <typename T>
	void subtractFrame(emd::Frame *frame);

    void addDimensionButtons();

public slots:
    void setSelectionDimensions(int) override;

private slots:
    void dimensionClicked(int);

private:
    emd::Frame *m_resultFrame;
    int m_integratedFrameCount;
    emd::ProcessingContext m_lastInputContext;
    emd::ProcessingContext m_subtractionContext;
    int m_cutoffInputIndex;

    int m_selectionDims;
    int m_availableDims;

    QVBoxLayout *m_dimensionLayout;
    QButtonGroup *m_dimensionGroup;
};

#endif