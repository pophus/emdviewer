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

#include "ProcessingContext.h"

#include <qdebug.h>

#include "ProcessingContextImpl.h"

namespace emd
{

ProcessingContext::ProcessingContext()
{

}

void ProcessingContext::init(const FrameSet::Selection &selection)
{
    if(m_impl.get())
    {
        qWarning() << "Processing context already initialized!";
        return;
    }

    m_impl.reset(new ProcessingContextImpl(selection));
}

void ProcessingContext::init(int length)
{
    if(m_impl.get())
    {
        qWarning() << "Processing context already initialized!";
        return;
    }

    m_impl.reset(new ProcessingContextImpl(FrameSet::Selection(length)));
}

void ProcessingContext::reset()
{
    m_impl.reset();
}

bool ProcessingContext::isValid() const
{
    return (m_impl.get() != nullptr);
}

FrameSet *ProcessingContext::frameSet() const
{
    if(m_impl.get())
    {
        return m_impl->frameSet();
    }

    return nullptr;
}

bool ProcessingContext::axesFlipped() const
{
    if(m_impl.get())
    {
        return m_impl->axesFlipped();
    }

    return false;
}

int ProcessingContext::frameCount() const
{
    if(m_impl.get())
    {
        return m_impl->frameCount();
    }

    return 0;
}

Frame *ProcessingContext::frameAtIndex(int index) const
{
    if(m_impl.get())
    {
        return m_impl->frameAtIndex(index);
    }

    return NULL;
}

void ProcessingContext::setFrameAtIndex(Frame *frame, int index)
{
    if(m_impl.get())
    {
        m_impl->setFrameAtIndex(frame, index);
    }
}

}