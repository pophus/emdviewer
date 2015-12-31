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

#include "DimensionsPane.h"

#include "DataGroup.h"
#include "DimensionWidget.h"

namespace emd
{

DimensionsPane::DimensionsPane(const DataGroup * const dataGroup, QWidget *parent)
	: QWidget(parent),
    m_dimCount(0),
    m_selectionModeEnabled(false)
{
    QVBoxLayout *layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	setLayout(layout);

    this->reset(dataGroup);
}

DimensionsPane::~DimensionsPane()
{
    
}

void DimensionsPane::reset(const DataGroup * const dataGroup)
{
    // Clear out old dimensions
    for(int iii = 0; iii < m_dimCount; ++iii)
    {
        if(m_dimensions[iii])
        {
            this->layout()->removeWidget(m_dimensions[iii]);
            delete m_dimensions[iii];
        }
    }

    m_dimensions.clear();
    m_dimCount = 0;

    if(!dataGroup)
        return;

	m_dimCount = dataGroup->dimCount();
	
	//QLabel *integrateLabel = new QLabel(QChar(0x222B));

	for(int iii = 0; iii < m_dimCount; ++iii)
	{
		Dataset *dimData = dataGroup->dimData(iii);

		// Do not create a complex dim
		if(dimData->isComplexDim())
		{
			m_dimensions.append(0);
			continue;
		}

		DimensionWidget *dim = new DimensionWidget(dimData, iii);
		this->layout()->addWidget(dim);
		m_dimensions.append(dim);

		connect(dim, SIGNAL(hButtonChanged(int, bool)),
			this, SLOT(changeHButton(int, bool)));
		connect(dim, SIGNAL(vButtonChanged(int, bool)),
			this, SLOT(changeVButton(int, bool)));
		connect(dim, SIGNAL(rangeChanged(int)),
			this, SLOT(updateDimensionRange(int)));
		connect(dim, SIGNAL(sliderPositionChanged(int, int)),
			this, SLOT(changeSliderPosition(int, int)));
		connect(dim, SIGNAL(panningChanged(int, bool)),
			this, SLOT(changeSliderPanning(int, bool)));
	}

	this->setSelection(FrameSet::Selection(dataGroup->data()->defaultSlice()));

    updateColumnWidths();
}

void DimensionsPane::setSelection(const FrameSet::Selection &selection)
{
    if(selection.size() != m_dimCount)
        return;

	m_selection = selection;

    for(int index = 0; index < m_dimCount; ++index)
    {
        if(selection[index].role == FrameSet::DimensionRole::DisplayHorizontal)
        {
            m_dimensions[index]->setHButtonChecked(true);
            m_dimensions[index]->setSliderPosition(0, 1);
            //m_dimensions[index]->setSliderSize(1.f);
        }
        else if(selection[index].role == FrameSet::DimensionRole::DisplayVertical)
        {
            m_dimensions[index]->setVButtonChecked(true);
            m_dimensions[index]->setSliderPosition(0, 1);
            //m_dimensions[index]->setSliderSize(1.f);
        }
        else if(m_dimensions[index])
        {
            m_dimensions[index]->setPrimaryIndex(selection[index].start);
            m_dimensions[index]->setSecondaryIndex(selection[index].start + selection[index].count - 1);
        }
    }
}

const FrameSet::Selection &DimensionsPane::selection() const
{
    return m_selection;
}

Dataset::Slice DimensionsPane::slice() const
{
    return *(m_selection.beginSlice());
}

void DimensionsPane::setDimensionSelection(int index, bool select)
{
    // TODO: better way?
    if(index == -1)
    {
        for(index = 0; index < m_dimCount; ++index)
        {
            if(!m_dimensions[index])
                continue;

            m_dimensions[index]->setSelectionEnabled(select);

            if(!FrameSet::isDisplayRole(m_selection[index].role))
                m_selection[index].role = FrameSet::DimensionRole::Free;
        }
        return;
    }

    if(index < 0 || index >= m_dimensions.count())
        return;

    if(FrameSet::isDisplayRole(m_selection[index].role))
        return;

    m_dimensions[index]->setSelectionEnabled(select);

    m_selection[index].role = FrameSet::DimensionRole::Special;
}

void DimensionsPane::setSelectionModeEnabled(bool enabled)
{
    m_selectionModeEnabled = enabled;

    for(int index = 0; index < m_dimCount; ++index)
    {
        if(FrameSet::isDisplayRole(m_selection[index].role))
            continue;

        if(m_dimensions[index])
        {
            m_dimensions[index]->setSelectionEnabled(enabled);

            m_selection[index].role = FrameSet::DimensionRole::Selection;
        }
    } 
}

int DimensionsPane::selectionDimensions() const
{
    int dims = 0;

    for(int index = 0; index < m_dimCount; ++index)
    {
        if(FrameSet::isDisplayRole(m_selection[index].role))
            continue;

        if(m_dimensions[index])
        {
            if(!m_dimensions[index]->selectionEnabled())
                dims |= (1 << index);
        }
    } 

    return dims;
}

int DimensionsPane::dimensionSize(const int index) const
{
    if(index < 0 || index >= m_dimCount)
        return -1;

    return m_dimensions.at(index)->dimLength();
}

void DimensionsPane::updateColumnWidths()
{
    int nameWidth = 0;
    int unitsWidth = 0;

    for(int index = 0; index < m_dimensions.size(); ++index)
    {
        if(!m_dimensions.at(index))
            continue;

        if(m_dimensions.at(index)->nameWidth() > nameWidth)
            nameWidth = m_dimensions.at(index)->nameWidth();

        if(m_dimensions.at(index)->unitsWidth() > unitsWidth)
            unitsWidth = m_dimensions.at(index)->unitsWidth();
    }

    // Add a bit of padding
    nameWidth += 10;
    unitsWidth += 10;

    for(int index = 0; index < m_dimensions.size(); ++index)
    {
        if(!m_dimensions.at(index))
            continue;

        m_dimensions.at(index)->setNameWidth(nameWidth);

        m_dimensions.at(index)->setUnitsWidth(unitsWidth);
    }
}


/******************************** Slots ************************************/

void DimensionsPane::setHSliderPos(float min, float max)
{
	m_dimensions.at(m_selection.horizontalDimension())->setSliderPosition(min, max);
}

void DimensionsPane::setVSliderPos(float min, float max)
{
	m_dimensions.at(m_selection.verticalDimension())->setSliderPosition(min, max);
}

void DimensionsPane::changeHButton(int index, bool checked)
{
	if(checked)
	{
		int currentHIndex = m_selection.horizontalDimension();
		int currentVIndex = m_selection.verticalDimension();

		// If the new H is the old V, swap them
		if(index == currentVIndex)
		{
			m_dimensions.at(index)->setVButtonChecked(false);
			m_dimensions.at(currentHIndex)->setVButtonChecked(true);

			m_selection[currentHIndex].role = FrameSet::DimensionRole::DisplayVertical;
		}
        else
        {
            if(m_selectionModeEnabled)
                m_dimensions.at(currentHIndex)->setSelectionEnabled(true);

			m_selection[currentHIndex].role = FrameSet::DimensionRole::Free;
            m_selection[currentHIndex].count = 1;
        }

		m_dimensions.at(currentHIndex)->setHButtonChecked(false);

		m_selection[index].role = FrameSet::DimensionRole::DisplayHorizontal;

		emit(selectionChanged(m_selection));
	}
	// Don't allow direct un-checking
	else
    {
		m_dimensions.at(index)->setHButtonChecked(true);
    }

}

void DimensionsPane::changeVButton(int index, bool checked)
{
	if(checked)
	{
		int currentHIndex = m_selection.horizontalDimension();
		int currentVIndex = m_selection.verticalDimension();

		// If the new V is the old H, swap them
		if(index == currentHIndex)
		{
			m_dimensions.at(index)->setHButtonChecked(false);
			m_dimensions.at(currentVIndex)->setHButtonChecked(true);

            m_selection[currentVIndex].role = FrameSet::DimensionRole::DisplayHorizontal;
		}
        else
        {
            if(m_selectionModeEnabled)
                m_dimensions.at(currentVIndex)->setSelectionEnabled(true);

            m_selection[currentVIndex].role = FrameSet::DimensionRole::Free;
            m_selection[currentVIndex].count = 1;
        }

		m_dimensions.at(currentVIndex)->setVButtonChecked(false);

        m_selection[index].role = FrameSet::DimensionRole::DisplayVertical;

		emit(selectionChanged(m_selection));
	}
	// Don't allow direct un-checking
	else
		m_dimensions.at(index)->setVButtonChecked(true);
}

// slider
void DimensionsPane::updateDimensionRange(int dimIndex)
{
    if(!FrameSet::isDisplayRole(m_selection[dimIndex].role))
	{
        m_dimensions[dimIndex]->getRange(m_selection[dimIndex].start, 
            m_selection[dimIndex].count);
		
		emit(selectionChanged(m_selection));
	}
}

void DimensionsPane::changeSliderPosition(int dimIndex, int position)
{
	if(dimIndex == m_selection.horizontalDimension())
	{
		emit(hPositionChanged(position));
	}
	else if(dimIndex == m_selection.verticalDimension())
	{
		emit(vPositionChanged(position));
	}
}

void DimensionsPane::changeSliderPanning(int dimIndex, bool panning)
{
	if(FrameSet::isDisplayRole(m_selection[dimIndex].role))
	{
		emit(panningChanged(panning));
	}
}

} // namespace emd
