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

#ifndef EMD_IMAGEWINDOWMODULE_H
#define EMD_IMAGEWINDOWMODULE_H

#include "EmdPluginLib.h"

#include <vector>

#include "WorkflowModule.h"

#include "ColourMap.h"
#include "Dataset.h"

class QImage;

namespace emd
{

class EMDPLUGIN_API ImageWindowModule : public WorkflowModule
{
	Q_OBJECT

    EMD_MODULE_DECLARATION

public:
	ImageWindowModule();
	~ImageWindowModule();

	// Inherited from WorkflowModule
    QWidget *controlWidget() override;
    void doPropertyChanged(const QString &key) override;
    void preprocess() override;
    void postprocess() override;
	Frame *processFrame(Frame *frame, int index) override;

protected:
	template <typename T>
	Frame *processData(Frame *frame);

private slots:
    void setColourMap(const QString &mapName);

signals:
	void imageGenerated(QImage *image);

private:
	float m_lowerScalingValue;
	float m_upperScalingValue;
	float m_lowerScalingLimit;
	float m_upperScalingLimit;
	bool m_lockedColourScaling;
    bool m_overrideColourScaling;

    std::vector<QImage *> m_images;
};

} // namespace emd

#endif