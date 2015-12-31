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

#ifndef EMD_PROCESSINGCONTEXT_H
#define EMD_PROCESSINGCONTEXT_H

#include "EmdPluginLib.h"

#include <memory>

#include <qobject.h> // for Q_DECL_EXPORT

#include "FrameSet.h"

namespace emd
{

class Frame;
class ProcessingContextImpl;

class EMDPLUGIN_API ProcessingContext
{
public:
    ProcessingContext();

    void init(const FrameSet::Selection &selection);
    void init(int length);
    void reset();

    bool isValid() const;

    FrameSet *frameSet() const;

    bool axesFlipped() const;

    int frameCount() const;

    Frame *frameAtIndex(int index) const;
    void setFrameAtIndex(Frame *frame, int index);

private:
    std::shared_ptr<ProcessingContextImpl> m_impl;
};

}

#endif