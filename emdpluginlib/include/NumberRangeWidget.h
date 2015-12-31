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

#ifndef EMD_NUMBERRANGEWIDGET_H
#define EMD_NUMBERRANGEWIDGET_H

#include "EmdPluginLib.h"

#include <QWidget>

namespace emd
{

class NumberBox;

class EMDPLUGIN_API NumberRangeWidget : public QWidget
{
	Q_OBJECT

public:
	NumberRangeWidget(QWidget *parent = 0);
	~NumberRangeWidget();

    bool floatType() const;
	void setType(bool floatType);

	void setValues(int left, int right);
	void setValues(float left, float right);

	void setLimits(int lower, int upper);
	void setLimits(float lower, float upper);

    bool getIntValues(int &lower, int &upper) const;
	bool getFloatValues(float &lower, float &upper) const;

private:
	NumberBox *m_leftBox;
	NumberBox *m_rightBox;
	bool m_floatType;
	bool m_lockValues;

private slots:
	void changeLeftLimit(int);
	void changeLeftLimit(float);
	void changeRightLimit(int);
	void changeRightLimit(float);

signals:
	void limitsChanged(float, float);
};

} // namespace emd


#endif