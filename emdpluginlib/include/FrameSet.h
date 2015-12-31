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

#pragma once

#include "EmdPluginLib.h"

#include "Dataset.h"
#include "DataSpace.h"
#include "Frame.h"

namespace emd
{

class EMDPLUGIN_API FrameSet
{
public:
    enum class DimensionRole
    {
        DisplayHorizontal,
        DisplayVertical,
        Free,
        Selection,
        Special
    };

    static bool isDisplayRole(DimensionRole role);

    typedef struct DimensionInfo {
		unsigned int start;
		unsigned int count;
        DimensionRole role;
        DimensionInfo()
            : start(0), count(0),
            role(DimensionRole::Free)
        {}
        DimensionInfo(int start, int count)
            : start(start), count(count),
            role(DimensionRole::Free)
        {}
        bool operator==(const DimensionInfo &other) const
        {
            return (start == other.start
                && count == other.count
                && role == other.role);
        }
        bool operator!=(const DimensionInfo &other) const
        {
            return !(*this == other);
        }
	} DimensionInfo;
	
	class EMDPLUGIN_API Selection : public std::vector<DimensionInfo>
    {
    public:
        class EMDPLUGIN_API SelectionIterator
        {
        public:
            SelectionIterator(const SelectionIterator &other);
            ~SelectionIterator();
            SelectionIterator& operator++();
            const Selection& operator*() const;
            bool operator==(const SelectionIterator &other) const;
            bool operator!=(const SelectionIterator &other) const;

        private:
            friend class Selection;
            SelectionIterator(const Selection &selection);

            Selection *m_current;
            const Selection *m_selection;
        };

        class EMDPLUGIN_API SliceIterator
        {
        public:
            SliceIterator& operator++();
            const Dataset::Slice& operator*() const;
            bool operator==(const SliceIterator &other) const;
            bool operator!=(const SliceIterator &other) const;

        private:
            friend class Selection;
            SliceIterator(const Selection *frameSet, const Dataset::Slice &slice);

            Dataset::Slice m_current;
            const Selection *m_selection;
        };

    public:
        Selection(int size = 1);
        Selection(const Dataset::Slice &slice);

        int horizontalDimension() const;
        int verticalDimension() const;

        int horizontalSize() const;
        int verticalSize() const;

        int count() const;

        SelectionIterator beginSelection() const;
        SelectionIterator endSelection() const;

        SliceIterator beginSlice() const;
        SliceIterator endSlice() const;

        bool containsSlice(const Dataset::Slice &slice) const;

        Dataset::Slice sliceFromIndex(int index) const;
        int indexFromSlice(const Dataset::Slice &slice) const;
    };

public:
    FrameSet(const Selection &selection);
    ~FrameSet();

    const Selection &selection() const;

    int count() const;

    Frame *frame(int index) const;
    Frame *frame(Dataset::Slice slice) const;

    // The FrameSet takes ownership of its frames.
    void setFrame(Frame *frame, int index);
    void setFrame(Frame *frame, const Dataset::Slice &slice);

    //int horizontalDimension() const;
    //int verticalDimension() const;

    Selection::SliceIterator beginSlice() const;
    Selection::SliceIterator endSlice() const;

    Selection::SelectionIterator beginSelection() const;
    Selection::SelectionIterator endSelection() const;

    bool containsSlice(const Dataset::Slice &slice) const;

    void subtract(const FrameSet &other, emd::FrameList &result) const;

    int sliceIndex(const Dataset::Slice &slice) const;

private:
    friend class Selection;

private:
    Selection m_selection;
    std::vector<Frame *> m_frames;
};

} // namespace emd