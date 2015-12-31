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

#include "DimensionWidget.h"

#include "Dataset.h"

#include "LabeledSlider.h"

namespace emd
{

DimensionWidget::DimensionWidget(Dataset *data, int index, QWidget *parent)
	: QWidget(parent)
    , m_data(data)
	, m_index(index)
    , m_selectionEnabled(false)
{
	m_nameLabel.setText(data->dataName());
	m_nameLabel.setAlignment(Qt::AlignCenter);

	m_unitLabel.setText(data->units());
	m_unitLabel.setAlignment(Qt::AlignCenter);

	m_hButton.setCheckable(true);
    m_hButton.setText("H");
	connect(&m_hButton, SIGNAL(clicked(bool)),
		this, SLOT(changeHButton(bool)));

	m_vButton.setCheckable(true);
    m_vButton.setText("V");
	connect(&m_vButton, SIGNAL(clicked(bool)),
		this, SLOT(changeVButton(bool)));

    QHBoxLayout *hvLayout = new QHBoxLayout();
    hvLayout->addWidget(&m_hButton);
    hvLayout->addWidget(&m_vButton);
    hvLayout->setSpacing(0);
    hvLayout->setContentsMargins(0, 0, 0, 0);

    m_slider = new LabeledSlider(0, m_data->dimLength(0) - 1);
    //m_slider->enableFlattening(true);
    m_slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    QString firstString = m_data->valueString(0);
    m_slider->setLeftText(firstString);
    m_slider->setPrimaryText(firstString);
    m_slider->setSecondaryText(firstString);

    m_slider->setRightText(m_data->valueString(m_data->dimLength(0) - 1));

    connect(m_slider, SIGNAL(indexesChanged()),
        this, SLOT(updateRange()));
	connect(m_slider, SIGNAL(panningChanged(bool)),
		this, SLOT(changeSliderPanning(bool)));

    QSize sliderHint = m_slider->sizeHint();
    m_hButton.setFixedSize(sliderHint.height(), sliderHint.height());
    m_vButton.setFixedSize(sliderHint.height(), sliderHint.height());

	QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget(&m_nameLabel);
	layout->addWidget(&m_unitLabel);
    layout->addLayout(hvLayout);
    layout->addWidget(m_slider);
	//layout->addWidget(&m_integrateButton);
	layout->setSpacing(20);
	layout->setContentsMargins(1, 1, 1, 1);

	setLayout(layout);
	
//	this->setStyleSheet(
//		"DimensionWidget {background-color: white}"
//		);
}

DimensionWidget::~DimensionWidget()
{

}

/********************* Accessors ******************************/

int DimensionWidget::dimLength() const
{
    return m_data->dimLength(0);
}

void DimensionWidget::setHButtonChecked(bool checked)
{
	m_hButton.setChecked(checked);

	if(checked)
		m_slider->setSelectionMode(LabeledSlider::SelectionModeFixedRange);
	else if(!m_vButton.isChecked())
        m_slider->setSelectionMode(m_selectionEnabled ? LabeledSlider::SelectionModeVariableRange : LabeledSlider::SelectionModeSingle);
}

void DimensionWidget::setVButtonChecked(bool checked)
{
	m_vButton.setChecked(checked);

	if(checked)
		m_slider->setSelectionMode(LabeledSlider::SelectionModeFixedRange);
	else if(!m_hButton.isChecked())
        m_slider->setSelectionMode(m_selectionEnabled ? LabeledSlider::SelectionModeVariableRange : LabeledSlider::SelectionModeSingle);
}

void DimensionWidget::setPrimaryIndex(int index)
{
    if(index < m_slider->minimum() || index > m_slider->maximum())
        return;

    m_slider->setPrimaryIndex(index);
}

void DimensionWidget::setSecondaryIndex(int index)
{
    if(index < m_slider->minimum() || index > m_slider->maximum())
        return;

    m_slider->setSecondaryIndex(index);
}

void DimensionWidget::setSliderPosition(float min, float max)
{
    int length = m_slider->maximum() - m_slider->minimum() + 1;
	int primary = (int) (min * length);
    if(primary < 0)
        primary = 0;

    int secondary = (int) (max * length);
    if(secondary > m_slider->maximum())
        secondary = m_slider->maximum();

    m_slider->setIndexes(primary, secondary);

    updateSliderLabels();
}

bool DimensionWidget::selectionEnabled() const
{
    return m_selectionEnabled;
}

void DimensionWidget::setSelectionEnabled(bool enabled)
{
    m_selectionEnabled = enabled;

    if(!m_hButton.isChecked() && !m_vButton.isChecked())
    {
        if(enabled)
            m_slider->setSelectionMode(LabeledSlider::SelectionModeVariableRange);
        else
            m_slider->setSelectionMode(LabeledSlider::SelectionModeSingle);
    }
}

int DimensionWidget::nameWidth() const
{
    return m_nameLabel.minimumSizeHint().width();
}

void DimensionWidget::setNameWidth(int width)
{
    m_nameLabel.setMinimumWidth(width);
}

int DimensionWidget::unitsWidth() const
{
    return m_unitLabel.minimumSizeHint().width();
}

void DimensionWidget::setUnitsWidth(int width)
{
    m_unitLabel.setMinimumWidth(width);
}

void DimensionWidget::getRange(unsigned int &start, unsigned int &count) const
{
    start = m_slider->primaryIndex();
    count = m_slider->secondaryIndex() - m_slider->primaryIndex() + 1;
}

/************************************ Private *************************************/

void DimensionWidget::updateSliderLabels()
{
    m_slider->setPrimaryText(m_data->valueString(m_slider->primaryIndex()));

    if(m_slider->selectionMode() != LabeledSlider::SelectionModeSingle)
        m_slider->setSecondaryText(m_data->valueString(m_slider->secondaryIndex()));
}

/*************************** QWidget ***************************/
//void DimensionWidget::paintEvent(QPaintEvent *)
//{
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
//}

/*************************** Slots *********************************/
void DimensionWidget::changeHButton(bool checked)
{
	setHButtonChecked(checked);
	emit(hButtonChanged(m_index, checked));
}

void DimensionWidget::changeVButton(bool checked)
{
	setVButtonChecked(checked);
	emit(vButtonChanged(m_index, checked));
}

void DimensionWidget::updateRange()
{
    updateSliderLabels();

    if(m_slider->selectionMode() == LabeledSlider::SelectionModeFixedRange)
    {
        emit(sliderPositionChanged(m_index, m_slider->primaryIndex()));
    }
    else
    {
        emit(rangeChanged(m_index));
    }
}

void DimensionWidget::changeSliderPanning(bool panning)
{
	emit(panningChanged(m_index, panning));
}

} // namespace emd

