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

#ifndef EMD_NUMBERBOX_H
#define EMD_NUMBERBOX_H

/* IntBox provides a simple text input box for integer values. A 
QValidator is used to ensure valid input. */

#include <QLineEdit>

class QKeyEvent;
class QValidator;

namespace emd
{

class NumberBox : public QLineEdit
{
	Q_OBJECT
public:
	NumberBox(bool floatType, QWidget *p = 0);

	void setType(bool floatType);
	void setValue(const int &val);
	void setValue(const float &val);
	int intValue();
	float floatValue();
	void setRange(const int &min, const int &max);
	void setRange(const float &min, const float &max);

protected:
	void keyPressEvent(QKeyEvent* e);

private:
	QValidator *m_validator;
	bool m_floatType;

private slots:
	void sendValueChanged();

signals:
	void valueChanged(int);
	void valueChanged(float);
};

} // namespace emd


#endif