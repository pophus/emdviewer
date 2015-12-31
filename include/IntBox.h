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

#ifndef EMD_INTBOX_H
#define EMD_INTBOX_H
/* IntBox provides a simple text input box for integer values. A 
QValidator is used to ensure valid input. */

#include <QLineEdit>
#include <QIntValidator>

class QKeyEvent;

namespace emd
{

class IntBox : public QLineEdit
{
	Q_OBJECT
public:
	IntBox(const int &value, QWidget *p = 0);

	void setValue(const int &val);
	int value() {return text().toInt();};
	void setRange(const int &min, const int &max)
		{m_validator->setRange(min, max);};

protected:
	void keyPressEvent(QKeyEvent* e);

private:
	QIntValidator *m_validator;
	QString m_lastValue;

private slots:
	void sendValueChanged();

signals:
	void valueChanged();
	void valueChanged(int);
};

} // namespace emd


#endif