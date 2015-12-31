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

#include "ProcessingContextImpl.h"

#include "Frame.h"

namespace emd
{

ProcessingContextImpl::ProcessingContextImpl(const FrameSet::Selection &selection)
    : m_frameSet(new FrameSet(selection))
{
    
}

ProcessingContextImpl::~ProcessingContextImpl()
{
    
}

FrameSet *ProcessingContextImpl::frameSet() const
{
    return m_frameSet.get();
}

bool ProcessingContextImpl::axesFlipped() const
{
    return m_frameSet->selection().horizontalDimension() > m_frameSet->selection().verticalDimension();
}

int ProcessingContextImpl::frameCount() const
{
    return m_frameSet->count();
}

Frame *ProcessingContextImpl::frameAtIndex(int index) const
{
    return m_frameSet->frame(index);
}

void ProcessingContextImpl::setFrameAtIndex(Frame *frame, int index)
{
    m_frameSet->setFrame(frame, index);
}

}