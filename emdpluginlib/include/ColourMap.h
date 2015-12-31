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

#ifndef EMD_COLOURMAP_H
#define EMD_COLOURMAP_H

#include "EmdPluginLib.h"

#include <memory>

#include "ColourMapImpl.h"

namespace emd
{

// The ColourMap class represents a table or gradient of colour values. These
// values are used to colour data images and their corresponding histograms.
class EMDPLUGIN_API ColourMap
{
public:
	ColourMap();
	ColourMap(const QLinearGradient &gradient, const QString &name);
	ColourMap(const ColourMap &other);
	~ColourMap();

    bool operator==(const ColourMap &other) const;

	//---------------------------- Accessors ----------------------------------
	const QString name() const;

	QLinearGradient gradient() const;

	// Gets a rectangular icon which can be used as a visual representation of 
	// the ColourMap.
	const QIcon &icon() const;

	// The colour table provides access to the individual colour values which
	//	comprise the gradient.
	uint const *colourTable() const;

	// All colour maps have the same (fixed) number of colour values.
	int colourTableRange() const;

    // Static
    static QSize iconSize();

private:
	std::shared_ptr<ColourMapImpl> m_impl;
};

} // namespace emd

#endif