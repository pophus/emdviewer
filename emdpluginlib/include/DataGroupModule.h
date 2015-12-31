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

#ifndef EMD_DATAGROUPMODULE_H
#define EMD_DATAGROUPMODULE_H

#include "EmdPluginLib.h"

#include "WorkflowModule.h"

#include <memory>

#include "Dataset.h"
#include "FrameSet.h"

namespace emd
{

class Dataset;
class Frame;

class EMDPLUGIN_API DataGroupModule : public WorkflowModule
{
    Q_OBJECT

    EMD_MODULE_DECLARATION

public:
	DataGroupModule(const DataGroup *dataGroup);
	~DataGroupModule();

	// WorkflowModule functions
    void doPropertyChanged(const QString &key) override;
	bool validate() const override;
	void preprocess() override;
    void process() override;
    void postprocess() override;

private:
    const DataGroup *dataGroup() const;

public slots:
    void setSlice(const Dataset::Slice &slice);
    void setSelection(const FrameSet::Selection &selection);

protected:
    bool m_processSelectionIndividually;
    FrameSet::Selection m_selection;
    // TODO: find a better way
    FrameSet::Selection::SelectionIterator *m_selectionIterator;
};

} // namespace emd

#endif