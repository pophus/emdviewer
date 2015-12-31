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

#include "IntBox.h"

#include <QKeyEvent>

namespace emd
{

IntBox::IntBox(const int &value, QWidget *parent)
	: QLineEdit(parent)
{
	this->setText(QString("%1").arg(value));
	m_lastValue = this->text();
	m_validator = new QIntValidator(-99, 99, this);
	setValidator(m_validator);
	setMaximumWidth(30);
	connect(this, SIGNAL(editingFinished()), this, SLOT(sendValueChanged()));
}

void IntBox::setValue(const int &val)
{
	this->setText(QString("%1").arg(val));
}

void IntBox::sendValueChanged()
{
	if(m_lastValue.compare(this->text()) != 0)
	{
		m_lastValue = this->text();
		emit(valueChanged());
		emit(valueChanged(value()));
	}
}

void IntBox::keyPressEvent(QKeyEvent *e)
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
