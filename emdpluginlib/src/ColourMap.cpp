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

#include "ColourMap.h"

#include <qdebug.h>
#include <QPainter>

namespace emd
{

ColourMap::ColourMap()
	: m_impl(new ColourMapImpl())
{

}

ColourMap::ColourMap(const QLinearGradient &gradient, const QString &name)
	: m_impl(new ColourMapImpl())
{
    m_impl->setGradient(gradient);
	m_impl->setName(name);
}

ColourMap::ColourMap(const ColourMap &other)
{
	this->m_impl = other.m_impl;
}

ColourMap::~ColourMap()
{
	//qDebug() << "~ColourMap() called for: " << (int)this << " with impl: " << (int)m_impl.get(); 
	//fprintf( stdout, "~ColourMap() called for: %x with impl: %x\n", this, m_impl.get() );
}

bool ColourMap::operator==(const ColourMap &other) const
{
    return (m_impl == other.m_impl);
}

const QString ColourMap::name() const
{
	return m_impl->name();
}

QLinearGradient ColourMap::gradient() const
{
	return m_impl->gradient();
}

const QIcon &ColourMap::icon() const
{
	return m_impl->icon();
}

uint const *ColourMap::colourTable() const
{
	return m_impl->colourTable();
}

int ColourMap::colourTableRange() const
{
	return m_impl->colourTableRange();
}

QSize ColourMap::iconSize()
{
    static const int kIconWidth = 256;
    static const int kIconHeight = 40;

    return QSize(kIconWidth, kIconHeight);
}

} // namespace emd

