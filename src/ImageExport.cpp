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

#include "ImageExport.h"

#include "FileHelper.h"

namespace emd
{

ImageExport::ImageExport(QObject *parent)
    : ExportOperation(parent)
{

}

void ImageExport::saveExportImage(QImage *image)
{
    QString path = fileNameForFrame(nullptr);

    // Check if the file already exists
    bool abort = false;
    bool fileExists = FileHelper::checkFileOverwrite(path, abort);

    if(abort)
    {
        cancelExport();
        return;
    }

    if(!fileExists)
    {
        bool success = image->save(path);
        if(!success)
        {
            qWarning() << "File write error at path " << path;
        }
    }

    ++m_exportIndex;
}

} // namespace emd
