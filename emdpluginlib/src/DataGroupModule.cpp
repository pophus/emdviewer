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

#include "DataGroupModule.h"

#include <qdebug.h>

#include "DataGroup.h"
#include "Dataset.h"
#include "FrameSet.h"

namespace emd
{

EMD_MODULE_DEFINITION(DataGroupModule)

DataGroupModule::DataGroupModule(const DataGroup *dataGroup)
	: m_processSelectionIndividually(true),
    m_selectionIterator(nullptr)
{
    m_properties["Source"] = "Automatic";

    QVariant var;
    var.setValue(dataGroup);
    setProperty("DataGroup", var);
}

DataGroupModule::~DataGroupModule()
{
    if(m_selectionIterator)
        delete m_selectionIterator;
}

// WorkflowModule functions

bool DataGroupModule::validate() const
{
	const DataGroup *dataGroup = property("DataGroup").value<const DataGroup *>();

	if(!dataGroup)
		return false;

    if(!dataGroup->data()->isLoaded())
        return false;

	return true;
}

void DataGroupModule::doPropertyChanged(const QString &key)
{
    if(key.compare("DataGroup") == 0)
    {
        const DataGroup *dataGroup = this->dataGroup();

        if(dataGroup)
        {
            setSlice(dataGroup->data()->defaultSlice());
        } 
    }
    else
    {
        WorkflowModule::doPropertyChanged(key);
    }
}

void DataGroupModule::preprocess()
{
    m_outputContext.reset();
    m_outputContext.init(**m_selectionIterator);

    const Dataset *dataset = this->dataGroup()->data();

    FrameSet *frameSet = m_outputContext.frameSet();

    // TODO: make the iterator code work for 2D slices.
    // Currently, beginSlice will be equal to endSlice in the 2D case
    // and thus we won't add the frame to the frameSet.
    if(frameSet->count() > 1)
    {
        auto it = frameSet->beginSlice();
        auto end = frameSet->endSlice();

        while(it != end)
        {
            const Dataset::Slice &slice = *it;

            frameSet->setFrame(dataset->frame(slice), slice);

            ++it;
        }
    }
    else
    {
        auto it = frameSet->beginSlice();
        const Dataset::Slice &slice = *it;

        frameSet->setFrame(dataset->frame(slice), slice);
    }
}

void DataGroupModule::process()
{
    // Do nothing. The default WorkflowModule implementation resets the output context.
    emit(workFinished(this));
}

void DataGroupModule::postprocess()
{
    if(m_processSelectionIndividually)
    {
        if(++*m_selectionIterator != m_selection.endSelection())
        {
            //FrameSet::Selection::SelectionIterator end = m_selection.endSelection();
            //for(int index = 0; index < (**m_selectionIterator).size(); ++index)
            //{
            //    qDebug() << (**m_selectionIterator)[index].start
            //         << (**m_selectionIterator)[index].count
            //          << (*end)[index].start
            //         << (*end)[index].count;
            //}
            // Trigger another processing iteration after this one
            this->update();
        }
        else
        {
            // Reset the iterator
            delete m_selectionIterator;
            m_selectionIterator = new FrameSet::Selection::SelectionIterator(m_selection.beginSelection());
        }
    }
}

// DataGroupModule functions

void DataGroupModule::setSlice(const Dataset::Slice &slice)
{
    setSelection(FrameSet::Selection(slice));
}

void DataGroupModule::setSelection(const FrameSet::Selection &selection)
{
    if(!dataGroup() /*|| !state.isValid()*/)
		    return;

    m_selection = selection;

    if(m_processSelectionIndividually)
    {
        m_selectionIterator = new FrameSet::Selection::SelectionIterator(m_selection.beginSelection());
    }

    if(enabled())
        update();
}

// Private
const DataGroup *DataGroupModule::dataGroup() const
{
    return property("DataGroup").value<const DataGroup *>();
}

} // namespace emd


