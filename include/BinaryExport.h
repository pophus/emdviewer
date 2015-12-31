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

#pragma once

#include "ExportOperation.h"

#include "BinaryOutputModule.h"
#include "Util.h"

namespace emd
{

class BinaryExport : public ExportOperation
{
    Q_OBJECT

public:
    BinaryExport(QObject *parent = 0);
    ~BinaryExport();

    BinaryOutputModule *outputModule() const;
    void setOutputModule(BinaryOutputModule *module);

    void doFinish() override;

private slots:
    void saveFrameData(Frame *frame);

private:
    BinaryOutputModule *m_outputModule;
    void *m_exportData;
    int64_t m_exportDataLength;
};

} // namespace emd
