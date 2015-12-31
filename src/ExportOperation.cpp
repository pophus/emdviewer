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

#include "ExportOperation.h"

namespace emd
{

ExportOperation::ExportOperation(QObject *parent)
    : QObject(parent),
    m_exportIndex(0)
{
}

/******************************** Export Parameters ****************************/

void ExportOperation::setOutputDirectory(const QString &dir)
{
    m_outputDirectory = dir;
}

void ExportOperation::setFileStem(const QString &stem)
{
    m_fileStem = stem;
}

QString ExportOperation::fileSuffix() const
{
    return m_fileSuffix;
}

void ExportOperation::setFileSuffix(const QString &suffix)
{
    m_fileSuffix = suffix;
}

QString ExportOperation::fullPath() const
{
    return m_outputDirectory + "/" + m_fileStem + m_fileSuffix;
}

int ExportOperation::itemCount() const
{
    return m_itemCount;
}

void ExportOperation::setItemCount(int count)
{
    m_itemCount = count;
}

void ExportOperation::finish()
{
    this->doFinish();
}

void ExportOperation::cancel()
{
    this->doCancel();
}

/********************************** Utility **************************************/

QString ExportOperation::fileNameForFrame(Frame *frame)
{
    return m_outputDirectory + "/" + m_fileStem + QString::number(m_exportIndex + 1) + m_fileSuffix;
}

void ExportOperation::cancelExport()
{

}

} // namespace emd