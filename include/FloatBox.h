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

#ifndef EMD_FLOATBOX_H
#define EMD_FLOATBOX_H

/* FloatBox provides a simple input box for floating point values. A 
validator is used to ensure valid input. The value is accessed via value()
and setValue(). */

#include <QLineEdit>
#include <QDoubleValidator>

class QKeyEvent;

namespace emd
{

class FloatBox : public QLineEdit
{
	Q_OBJECT
public:
	FloatBox(const float &value, QWidget *p = 0);

	void setValue(const float &val);
	float value() {return text().toFloat();}
	void setRange(const float &min, const float &max, const int &dec = 3)
		{m_validator->setRange(min, max, dec);}

protected:
	void keyPressEvent(QKeyEvent* e);

private:
	QDoubleValidator *m_validator;
	float m_lastValue;

private slots:
	void sendValueChanged();

signals:
	void valueChanged();
	void valueChanged(float);
};

} // namespace emd


#endif