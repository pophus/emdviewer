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

#ifndef EMD_COLOURMAPIMPL_H
#define EMD_COLOURMAPIMPL_H

#include <memory>

#include <QGradient>
#include <QIcon>
#include <QString>

namespace emd
{

#define GRADIENT_RANGE 256

// The ColourMap class represents a table or gradient of colour values. These
// values are used to colour data images and their corresponding histograms.
class ColourMapImpl
{
public:
	ColourMapImpl();
	~ColourMapImpl();

	//---------------------------- Accessors ----------------------------------
	const QString name() const;
	void setName(const QString &name);

	QLinearGradient gradient() const;
	void setGradient(const QLinearGradient &gradient);

	// Gets a rectangular icon which can be used as a visual representation of 
	// the ColourMap.
	const QIcon &icon() const;

	// The colour table provides access to the individual colour values which
	//	comprise the gradient.
	uint const *colourTable() const;

	// All colour maps have the same (fixed) number of colour values.
	int colourTableRange() const;

private:
	QLinearGradient m_gradient;
	QString m_name;
	QIcon m_icon;
	uint m_colourTable[GRADIENT_RANGE];


	// Creates the colour table from the gradient.
	void generateColourTable();

	// Generates the display icon. Must be repeated if the gradient is modified.
	void makeIcon();
};

} // namespace emd

#endif