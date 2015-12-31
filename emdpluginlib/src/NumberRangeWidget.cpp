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

#include "NumberRangeWidget.h"

#include <limits>

#include <QHBoxLayout>

#include "Util.h"

#include "ImageWindowModule.h"
#include "NumberBox.h"

namespace emd
{

NumberRangeWidget::NumberRangeWidget(QWidget *parent)
	: QWidget(parent),
	m_floatType(true),
	m_lockValues(true)
{
	m_leftBox = new NumberBox(m_floatType, this);
	connect(m_leftBox, SIGNAL(valueChanged(int)),
		this, SLOT(changeLeftLimit(int)));
	connect(m_leftBox, SIGNAL(valueChanged(float)),
		this, SLOT(changeLeftLimit(float)));

	m_rightBox = new NumberBox(m_floatType, this);
	connect(m_rightBox, SIGNAL(valueChanged(int)),
		this, SLOT(changeRightLimit(int)));
	connect(m_rightBox, SIGNAL(valueChanged(float)),
		this, SLOT(changeRightLimit(float)));

	QHBoxLayout *layout = new QHBoxLayout();
	layout->addStretch();
	layout->addWidget(m_leftBox);
	layout->addWidget(m_rightBox);
	layout->addStretch();
	setLayout(layout);

	this->setType(false);
}

NumberRangeWidget::~NumberRangeWidget()
{

}

bool NumberRangeWidget::floatType() const
{
    return m_floatType;
}

void NumberRangeWidget::setType(bool floatType)
{
    if(floatType == m_floatType)
        return;

	m_leftBox->setType(floatType);
	m_rightBox->setType(floatType);

	if(floatType)
	{
		setLimits(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		//setValues(-FLT_MAX, FLT_MAX);
	}
	else
	{
		setLimits(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
		//setValues(INT_MIN, INT_MAX);
	}

	m_floatType = floatType;
}

void NumberRangeWidget::setValues(int left, int right)
{
	if(m_leftBox->intValue() == left && m_rightBox->intValue() == right)
		return;

	m_leftBox->setValue(left);
	m_rightBox->setValue(right);
}

void NumberRangeWidget::setValues(float left, float right)
{
	if(m_leftBox->floatValue() == left && m_rightBox->floatValue() == right)
		return;

	if(left != kInvalidFloatValue)
		m_leftBox->setValue(left);

	if(right != kInvalidFloatValue)
		m_rightBox->setValue(right);
}

bool NumberRangeWidget::getIntValues(int &lower, int &upper) const
{
	if(m_floatType)
		return false;

	lower = m_leftBox->intValue();
	upper = m_rightBox->intValue();

	return true;
}

bool NumberRangeWidget::getFloatValues(float &lower, float &upper) const
{
	if(!m_floatType)
		return false;

	lower = m_leftBox->floatValue();
	upper = m_rightBox->floatValue();

	return true;
}

void NumberRangeWidget::setLimits(int lower, int upper)
{
	m_leftBox->setRange(lower, upper);
	m_rightBox->setRange(lower, upper);
}

void NumberRangeWidget::setLimits(float lower, float upper)
{
	m_leftBox->setRange(lower, upper);
	m_rightBox->setRange(lower, upper);
}

// Slots

void NumberRangeWidget::changeLeftLimit(int limit)
{
	float leftLimit = limit;
	float rightLimit = -std::numeric_limits<float>::max();
	if(limit > m_rightBox->intValue())
	{
		rightLimit = leftLimit;
		m_rightBox->setValue(limit);
	}

	emit(limitsChanged(leftLimit, rightLimit));
}

void NumberRangeWidget::changeLeftLimit(float limit)
{
	float leftLimit = limit;
	float rightLimit = kInvalidFloatValue;
	if(limit > m_rightBox->floatValue())
	{
		rightLimit = leftLimit;
		m_rightBox->setValue(limit);
	}

	emit(limitsChanged(leftLimit, rightLimit));
}

void NumberRangeWidget::changeRightLimit(int limit)
{
	float leftLimit = std::numeric_limits<float>::max();
	float rightLimit = limit;
	if(limit < m_leftBox->intValue())
	{
		leftLimit = rightLimit;
		m_leftBox->setValue(limit);
	}

	emit(limitsChanged(leftLimit, rightLimit));
}

void NumberRangeWidget::changeRightLimit(float limit)
{
	float leftLimit = kInvalidFloatValue;
	float rightLimit = limit;
	if(limit < m_leftBox->floatValue())
	{
		leftLimit = rightLimit;
		m_leftBox->setValue(limit);
	}

	emit(limitsChanged(leftLimit, rightLimit));
}

} // namespace emd
