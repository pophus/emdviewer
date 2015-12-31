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

#include "FloatBox.h"

#include <QKeyEvent>

namespace emd
{

FloatBox::FloatBox(const float &value, QWidget *parent)
	: QLineEdit(parent)
{
	setText(QString("%1").arg(value, 5, 'f', 3, '0'));
	m_validator = new QDoubleValidator(0, 1, 3, this);
	m_validator->setNotation(QDoubleValidator::StandardNotation);
	setValidator(m_validator);

	setMaximumWidth(50);
	setAlignment(Qt::AlignCenter);

	m_lastValue = this->value();

	connect(this, SIGNAL(editingFinished()), this, SLOT(sendValueChanged()));
}
void FloatBox::setValue(const float &val)
{
	this->setText(QString("%1").arg(val, 5, 'f', 3, '0'));
}

void FloatBox::sendValueChanged()
{
	if(m_lastValue != value())
	{
		m_lastValue = value();
		emit(valueChanged());
		emit(valueChanged(value()));
	}
}

void FloatBox::keyPressEvent(QKeyEvent *e)
{
	// let base class handle the event
	QLineEdit::keyPressEvent(e);
     
	if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
	{
		// Accept enter/return events so they won't
		// be ever propagated to the parent dialog.
		e->accept();
	}
}

} // namespace emd
