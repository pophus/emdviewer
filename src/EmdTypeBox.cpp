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

#include "EmdTypeBox.h"

namespace emd
{

static const int s_supportedTypeCount = 10;
static const DataType s_types[s_supportedTypeCount] =
{
    DataTypeUInt8,
    DataTypeUInt16,
    DataTypeUInt32,
    DataTypeUInt64,
    DataTypeInt8,
    DataTypeInt16,
    DataTypeInt32,
    DataTypeInt64,
    DataTypeFloat32,
    DataTypeFloat64
};

EmdTypeBox::EmdTypeBox(QWidget *parent)
    : QComboBox(parent)
{
    for(int iii = 0; iii < s_supportedTypeCount; ++iii)
        this->addItem(emdTypeString(s_types[iii]));
}

DataType EmdTypeBox::currentType() const
{
    return s_types[this->currentIndex()];
}

} // namespace emd