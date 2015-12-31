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

#include "ColourMapImpl.h"

#include <qdebug.h>
#include <QPainter>

#include "ColourMap.h"

namespace emd
{

ColourMapImpl::ColourMapImpl()
{
    setGradient(QLinearGradient(0, 0, 1, 1));
    m_name = "Default";
}

ColourMapImpl::~ColourMapImpl()
{
	//qDebug() << "~ColourMapImpl() called for: " << (int)this; 
	//fprintf( stdout, "~ColourMapImpl() called for: %x\n", this );
}

const QString ColourMapImpl::name() const
{
	return m_name;
}

void ColourMapImpl::setName(const QString &name)
{
	m_name = name;
}

QLinearGradient ColourMapImpl::gradient() const 
{
	return m_gradient;
}

void ColourMapImpl::setGradient(const QLinearGradient &gradient)
{
	m_gradient = gradient;

	// Changing the gradient of a ColourMap requires the regeneration of
	// its icon and colour table.
	makeIcon();
	generateColourTable();
}

const QIcon &ColourMapImpl::icon() const
{
	return m_icon;
}

uint const *ColourMapImpl::colourTable() const
{
	return m_colourTable;
}

int ColourMapImpl::colourTableRange() const
{
	return GRADIENT_RANGE;
}

void ColourMapImpl::makeIcon()
{
    QSize iconSize = ColourMap::iconSize();

	m_gradient.setStart(0, 0);
	m_gradient.setFinalStop(iconSize.width(), 0);

	QBrush brush(m_gradient);
	
	// For now, only a single fixed size of icon is supported. In the future,
	// arbitrary icon sizes may be supported.
	QPixmap pixmap(iconSize.width(), iconSize.height());

	QPainter p(&pixmap);
	p.fillRect(0, 0, iconSize.width(), iconSize.height(), brush);

	m_icon = QIcon(pixmap);
}

void ColourMapImpl::generateColourTable()
{
	// A gradient has n-1 segments, where n is the number of stops of the gradient.
	// To fill in the colour table, we apportion to each segment a subset of the
	// colour table values based on the separation distance of the segment's
	// endpoint stops. For example, if the first stop is at 0.0 (always the case),
	// the second stop is at 0.2, and the colour table has 100 values, then the
	// first segment will get 0.2 * 100 = 20 values in the colour table. Note that
	// the stop locations will always range from 0.0 to 1.0.
	//
	// For each segment of the gradient, we assign its colour table values using
	// simple linear interpolation between its endpoint colours.

	// We must have at least two stops to proceed.
	QGradientStops stops = m_gradient.stops();
	if(stops.count() < 2)
		return;

	QGradientStop start = stops.at(0), end;
	QColor startColour, endColour;
	int steps, count = 0;
	float r, g, b;
	float dr, dg, db;
	for(int iii = 1; iii < stops.count(); ++iii)
	{
		end = stops.at(iii);
		steps = (int)(end.first * GRADIENT_RANGE) - (int)(start.first * GRADIENT_RANGE);   // number of colour steps in the segment
		if(steps <= 0)
			steps = 1;

		startColour = start.second;
		endColour = end.second;

		r = startColour.red();
		g = startColour.green();
		b = startColour.blue();

		dr = (float)(endColour.red() - startColour.red()) / steps;
		dg = (float)(endColour.green() - startColour.green()) / steps;
		db = (float)(endColour.blue() - startColour.blue()) / steps;

		for(int jjj = count; jjj <= count + steps; ++jjj)
		{
			if(jjj == GRADIENT_RANGE)
				break;

			m_colourTable[jjj] = qRgb((int)r, (int)g, (int)b);
			r += dr;
			g += dg;
			b += db;
		}

		start = end;
		count += steps;
	}
}

} // namespace emd
