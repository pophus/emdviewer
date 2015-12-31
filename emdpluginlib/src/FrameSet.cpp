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

#include "FrameSet.h"

#include "Frame.h"

#include <qdebug.h>

namespace emd
{

bool FrameSet::isDisplayRole(DimensionRole role)
{
    return (role == DimensionRole::DisplayHorizontal
        || role == DimensionRole::DisplayVertical);
}

// ----------------------------------------------------------------------------------

FrameSet::Selection::SliceIterator::SliceIterator(const Selection *selection, const Dataset::Slice &slice)
    : m_selection(selection), m_current(slice)
{
}

FrameSet::Selection::SliceIterator &FrameSet::Selection::SliceIterator::operator++()
{
    for(int findex = 0; findex < m_current.size(); ++findex)
    {
        if(m_current[findex] < 0)
            continue;

        ++m_current[findex];

        if((unsigned)m_current[findex] < 
            (*m_selection)[findex].start + (*m_selection)[findex].count)
        {
            for(int rindex = findex - 1; rindex >= 0; --rindex)
            {
                if(m_current[rindex] < 0)
                    continue;

                m_current[rindex] = (*m_selection)[rindex].start;
            }

            break;
        }
    }

    return *this;
}

const Dataset::Slice &FrameSet::Selection::SliceIterator::operator*() const
{
    return m_current;
}

bool FrameSet::Selection::SliceIterator::operator==(const SliceIterator &other) const
{
    if(m_current.size() != other.m_current.size())
        return false;

    for(int index = 0; index < m_current.size(); ++index)
    {
        if(m_current[index] != other.m_current[index])
            return false;
    }

    return true;
}

bool FrameSet::Selection::SliceIterator::operator!=(const SliceIterator &other) const
{
    return !(*this == other);
}

// ----------------------------------------------------------------------------------

FrameSet::Selection::SelectionIterator::SelectionIterator(const SelectionIterator &other)
    : m_current(new Selection(*other.m_current)),
    m_selection(other.m_selection)
{
}

FrameSet::Selection::SelectionIterator::SelectionIterator(const Selection &selection)
    : m_selection(&selection)
{
    m_current = new Selection(selection);
}

FrameSet::Selection::SelectionIterator::~SelectionIterator()
{
    if(m_current)
        delete m_current;
}

FrameSet::Selection::SelectionIterator &FrameSet::Selection::SelectionIterator::operator++()
{
    for(int findex = 0; findex < m_current->size(); ++findex)
    {
        if((*m_current)[findex].role == DimensionRole::Selection)
        {
            ++((*m_current)[findex].start);

            if((*m_current)[findex].start < 
                (*m_selection)[findex].start + (*m_selection)[findex].count)
            {
                for(int rindex = findex - 1; rindex >= 0; --rindex)
                {
                    if((*m_current)[rindex].role != DimensionRole::Selection)
                        continue;

                    (*m_current)[rindex].start = (*m_selection)[rindex].start;
                }

                break;
            }
        }
    }

    return *this;
}

const FrameSet::Selection &FrameSet::Selection::SelectionIterator::operator*() const
{
    return *m_current;
}

bool FrameSet::Selection::SelectionIterator::operator==(const SelectionIterator &other) const
{
    if(m_selection != other.m_selection)
        return false;

    if(m_current->size() != other.m_current->size())
        return false;

    for(int index = 0; index < m_current->size(); ++index)
    {
        if((*m_current)[index] != (*other.m_current)[index])
            return false;
    }

    return true;
}

bool FrameSet::Selection::SelectionIterator::operator!=(const SelectionIterator &other) const
{
    return !(*this == other);
}

FrameSet::Selection::Selection(int size)
    : std::vector<DimensionInfo>(1)
{
    (*this)[0].start = 0;
    (*this)[0].count = size;
    (*this)[0].role = DimensionRole::Free;
}

FrameSet::Selection::Selection(const Dataset::Slice &slice)
    : std::vector<DimensionInfo>(slice.size())
{
    for(int index = 0; index < slice.size(); ++index)
    {
        if(slice[index] == Dataset::HorizontalDimension)
        {
            (*this)[index].role = DimensionRole::DisplayHorizontal;
        }
        else if(slice[index] == Dataset::VerticalDimension)
        {
            (*this)[index].role = DimensionRole::DisplayVertical;
        }
        else
        {
            (*this)[index].start = slice[index];
            (*this)[index].count = 1;
            (*this)[index].role = DimensionRole::Free;
        }
    }
}

int FrameSet::Selection::horizontalDimension() const
{
    for(int index = 0; index < size(); ++index)
    {
        if((*this)[index].role == FrameSet::DimensionRole::DisplayHorizontal)
            return index;
    }

    return -1;
}

int FrameSet::Selection::verticalDimension() const
{
    for(int index = 0; index < size(); ++index)
    {
        if((*this)[index].role == FrameSet::DimensionRole::DisplayVertical)
            return index;
    }

    return -1;
}

int FrameSet::Selection::horizontalSize() const
{
    int index = horizontalDimension();

    if(index >= 0)
        return (*this)[index].count;

    return 0;
}

int FrameSet::Selection::verticalSize() const
{
    int index = verticalDimension();

    if(index >= 0)
        return (*this)[index].count;

    return 0;
}

int FrameSet::Selection::count() const
{
    if(size() == 0)
        return 0;

    int count = 1;

    for(int dimIndex = 0; dimIndex < size(); ++dimIndex)
    {
        if(FrameSet::isDisplayRole((*this)[dimIndex].role))
            continue;

        count *= (*this)[dimIndex].count;
    }

    return count;
}

FrameSet::Selection::SelectionIterator FrameSet::Selection::beginSelection() const
{
    SelectionIterator it(*this);

    for(int index = 0; index < size(); ++index)
    {
        if(at(index).role == DimensionRole::Selection)
            (*(it.m_current))[index].count = 1;
    }

    return it;
}

FrameSet::Selection::SelectionIterator FrameSet::Selection::endSelection() const
{
    SelectionIterator it(*this);

    for(int index = 0; index < size(); ++index)
    {
        if(at(index).role == DimensionRole::Selection)
        {
            (*(it.m_current))[index].start += (*(it.m_current))[index].count;
            (*(it.m_current))[index].count = 1;
        }
    }

    return it;
}

FrameSet::Selection::SliceIterator FrameSet::Selection::beginSlice() const
{
    if(size() == 0)
        return Selection::SliceIterator(this, Dataset::Slice());

    return Selection::SliceIterator(this, sliceFromIndex(0));
}

FrameSet::Selection::SliceIterator FrameSet::Selection::endSlice() const
{
    if(size() == 0)
        return SliceIterator(this, Dataset::Slice());

    Dataset::Slice slice(size());
    for(int index = 0; index < size(); ++index)
    {
        if((*this)[index].role == DimensionRole::DisplayHorizontal)
            slice[index] = Dataset::HorizontalDimension;
        else if((*this)[index].role == DimensionRole::DisplayVertical)
            slice[index] = Dataset::VerticalDimension;
        else
            slice[index] = (*this)[index].start + (*this)[index].count;
    }

    return SliceIterator(this, slice);
}

bool FrameSet::Selection::containsSlice(const Dataset::Slice &slice) const
{
    if(slice.size() != this->size())
        return false;

    for(int index = 0; index < slice.size(); ++index)
    {
        if(slice.at(index) >= 0
            && (slice.at(index) < this->at(index).start
            || slice.at(index) >= this->at(index).start + this->at(index).count))
            return false;
    }

    return true;
}

Dataset::Slice FrameSet::Selection::sliceFromIndex(int index) const
{
    Dataset::Slice slice(size(), -1);

    int count = this->count();

    for(int dimIndex = (int)size() - 1; dimIndex >= 0; --dimIndex)
    {
        if((*this)[dimIndex].role == DimensionRole::DisplayHorizontal)
        {
            slice[dimIndex] = Dataset::HorizontalDimension;
        }
        else if((*this)[dimIndex].role == DimensionRole::DisplayVertical)
        {
            slice[dimIndex] = Dataset::VerticalDimension;
        }
        else
        {
            count /= (*this)[dimIndex].count;

            slice[dimIndex] = (*this)[dimIndex].start + index / count;

            index = index % count;
        }
    }

    return slice;
}

int FrameSet::Selection::indexFromSlice(const Dataset::Slice &slice) const
{
    int index = 0;
    int mult = 1;

    for(int dimIndex = 0; dimIndex < (int)size(); ++dimIndex)
    {
        if(isDisplayRole((*this)[dimIndex].role))
            continue;

        index += mult * (slice[dimIndex] - (*this)[dimIndex].start);

        mult *= (*this)[dimIndex].count;
    }

    return index;
}

// ----------------------------------------------------------------------------------

FrameSet::FrameSet(const Selection &selection)
    : m_selection(selection)
{
    m_frames.resize(count(), nullptr);
}

FrameSet::~FrameSet()
{
    for(Frame *frame : m_frames)
    {
        delete frame;
    }
}

const FrameSet::Selection &FrameSet::selection() const
{
    return m_selection;
}

int FrameSet::count() const
{
    int count = 1;

    for(int dimIndex = 0; dimIndex < m_selection.size(); ++dimIndex)
    {
        if(isDisplayRole(m_selection[dimIndex].role))
            continue;

        count *= m_selection[dimIndex].count;
    }

    return count;
}

Frame *FrameSet::frame(int index) const
{
    if(index < 0 || index >= m_frames.size())
        return nullptr;

    return m_frames[index];
}

Frame *FrameSet::frame(Dataset::Slice slice) const
{
    return frame(m_selection.indexFromSlice(slice));
}

void FrameSet::setFrame(Frame *frame, int index)
{
    if(index < 0 || index >= m_frames.size())
    {
        return;
    }

    m_frames[index] = frame;
}

void FrameSet::setFrame(Frame *frame, const Dataset::Slice &slice)
{
    setFrame(frame, m_selection.indexFromSlice(slice));
}

FrameSet::Selection::SelectionIterator FrameSet::beginSelection() const
{
    return m_selection.beginSelection();
}

FrameSet::Selection::SelectionIterator FrameSet::endSelection() const
{
    return m_selection.endSelection();
}

FrameSet::Selection::SliceIterator FrameSet::beginSlice() const
{
    return m_selection.beginSlice();
}

FrameSet::Selection::SliceIterator FrameSet::endSlice() const
{
    return m_selection.endSlice();
}

bool FrameSet::containsSlice(const Dataset::Slice &slice) const
{
    return m_selection.containsSlice(slice);
}

void FrameSet::subtract(const FrameSet &other, emd::FrameList &result) const
{
    Selection::SliceIterator it = this->beginSlice();
    Selection::SliceIterator end = this->endSlice();

    while(it != end)
    {
        if(!other.containsSlice(*it))
            result.push_back(this->frame(*it));

        ++it;
    }
}

int FrameSet::sliceIndex(const Dataset::Slice &slice) const
{
    return m_selection.indexFromSlice(slice);
}

// -------------------------------------------------------------------------------

} // namespace emd