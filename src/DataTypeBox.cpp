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

#include "DataTypeBox.h"

#include "Util.h"

namespace emd
{

DataTypeBox::DataTypeBox(QWidget *parent)
	: QComboBox(parent)
{
	this->addItem("int8", QVariant::fromValue<emd::DataType>(DataTypeInt8));
	this->addItem("int16", QVariant::fromValue<emd::DataType>(DataTypeInt16));
	this->addItem("int32", QVariant::fromValue<emd::DataType>(DataTypeInt32));
	this->addItem("int64", QVariant::fromValue<emd::DataType>(DataTypeInt64));
	this->addItem("uint8", QVariant::fromValue<emd::DataType>(DataTypeUInt8));
	this->addItem("uint16", QVariant::fromValue<emd::DataType>(DataTypeUInt16));
	this->addItem("uint32", QVariant::fromValue<emd::DataType>(DataTypeUInt32));
	this->addItem("uint64", QVariant::fromValue<emd::DataType>(DataTypeUInt64));
	this->addItem("float", QVariant::fromValue<emd::DataType>(DataTypeFloat32));
	this->addItem("double", QVariant::fromValue<emd::DataType>(DataTypeFloat64));
	this->addItem("string", QVariant::fromValue<emd::DataType>(DataTypeString));
}

DataTypeBox::~DataTypeBox()
{

}

DataType DataTypeBox::currentType() const
{
	QVariant value = this->itemData(this->currentIndex());
	return value.value<emd::DataType>();
}

} // namespace emd