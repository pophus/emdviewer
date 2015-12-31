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

#ifndef EMD_DIMENSIONSPANE_H
#define EMD_DIMENSIONSPANE_H

#include <QtGui>
#include <QtWidgets>

#include "Dataset.h"
#include "FrameSet.h"

namespace emd
{

class DimensionWidget;
class DataGroup;

class DimensionsPane : public QWidget
{
	Q_OBJECT

public:
	DimensionsPane(const DataGroup * const dataGroup, QWidget *parent = 0);
	virtual ~DimensionsPane();

	void reset(const DataGroup * const dataGroup);
    const FrameSet::Selection &selection() const;
    Dataset::Slice slice() const;
    void setSelection(const FrameSet::Selection &selection);
    void setDimensionSelection(int index, bool select);
    void setSelectionModeEnabled(bool enabled);
    int selectionDimensions() const;
    int dimensionSize(const int index) const;

public slots:
	void setHSliderPos(float, float);
	void setVSliderPos(float, float);

private:
    void updateColumnWidths();

private slots:
	void changeHButton(int, bool);
	void changeVButton(int, bool);
	// slider
    void updateDimensionRange(int dimIndex);
	void changeSliderPosition(int, int);
	void changeSliderPanning(int, bool);

signals:
	void hPositionChanged(int);
	void vPositionChanged(int);
	void panningChanged(bool);
	void selectionChanged(const FrameSet::Selection &state);

private:
	int m_dimCount;
	FrameSet::Selection m_selection;
	QList<DimensionWidget*> m_dimensions;
    bool m_selectionModeEnabled;
};

} // namespace emd


#endif