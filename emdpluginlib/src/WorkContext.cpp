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

#include "WorkContext.h"

#include "WorkflowModule.h"

namespace emd
{

WorkContext::WorkContext(WorkflowModule *module)
    : m_module(module)
{
}

WorkflowModule *WorkContext::module() const
{
    return m_module;
}

int WorkContext::start() const
{
    return m_start;
}

void WorkContext::setStart(int start)
{
    m_start = start;
}

int WorkContext::count() const
{
    return m_count;
}

void WorkContext::setCount(int count)
{
    m_count = count;
}

}