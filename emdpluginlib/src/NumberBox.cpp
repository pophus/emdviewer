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

#include "NumberBox.h"

#include <limits>

#include <QKeyEvent>
#include <QIntValidator>
#include <QDoubleValidator>

#include <qdebug.h>

namespace emd
{

NumberBox::NumberBox(bool floatType, QWidget *parent)
	: QLineEdit(parent),
	m_validator(0),
	m_floatType(floatType)
{
	this->setType(m_floatType);
	//setMaximumWidth(30);
	connect(this, SIGNAL(editingFinished()), this, SLOT(sendValueChanged()));
}

void NumberBox::setType(bool floatType)
{
    if(m_validator && m_floatType == floatType)
        return;

	QValidator *oldValidator = m_validator;
	if(floatType)
        m_validator = new QDoubleValidator(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 99, this);
	else
		m_validator = new QIntValidator(-99, 99, this);

	if(oldValidator)
		delete oldValidator;

	m_floatType = floatType;

    this->setValidator(m_validator);
}

int NumberBox::intValue()
{
	return text().toInt();
}

float NumberBox::floatValue()
{
	return text().toFloat();
}

void NumberBox::setValue(const int &val)
{
	setText(QString("%1").arg(val));
}

void NumberBox::setValue(const float &val)
{
	setText(QString("%1").arg(val));
}


void NumberBox::setRange(const int &min, const int &max)
{
    if(m_floatType)
        return;

	((QIntValidator*)m_validator)->setRange(min, max);

    if(this->intValue() < min)
        this->setValue(min);
    else if(this->intValue() > max)
        this->setValue(max);
}

void NumberBox::setRange(const float &min, const float &max)
{
    if(!m_floatType)
        return;

	((QDoubleValidator*)m_validator)->setRange(min, max, 99);

    if(this->floatValue() < min)
        this->setValue(min);
    else if(this->floatValue() > max)
        this->setValue(max);
}

void NumberBox::sendValueChanged()
{
	if(m_floatType)
		emit(valueChanged(floatValue()));
	else
		emit(valueChanged(intValue()));
}
void NumberBox::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
	{
		// Accept enter/return events so they won't
		// ever be propagated to the parent dialog.
		e->accept();

        emit(editingFinished());
	}
    else
    {
	    // let base class handle the event
	    QLineEdit::keyPressEvent(e);
    }
}

} // namespace emd
