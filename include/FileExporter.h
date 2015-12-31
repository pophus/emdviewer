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

#ifndef EMD_FILEEXPORTER_H
#define EMD_FILEEXPORTER_H

#include <QtWidgets>

#include "BinaryOutputModule.h"
#include "Dataset.h"
#include "EmdTypeBox.h"
#include "FrameSet.h"

class QLineEdit;

namespace emd
{

class ExportOperation;
class NumberRangeWidget;

class FileExporter : public QWidget
{
	Q_OBJECT

public:
    enum Mode 
    {
        ViewMode,
        ImageMode,
        BinaryMode
    };

public:
	FileExporter(QWidget *parent = 0);

    Mode mode() const;

    const QString fileNameStem() const;
    void setFileNameStem(const QString &stem);

    const QString exportDirectory() const;
    void setExportDirectory(const QString &dir);

    const QString fileSuffix() const;
    
    BinaryOutputModule::OutputMode binaryOutputMode() const;

    BinaryOutputModule::ProcessingMode binaryProcessingMode() const;

    DataType binaryDataType() const;

    void cancelSelection();

    void finishExport();

public slots:
    void setSelection(const FrameSet::Selection &);

private:
    void updateFileName();

private slots:
    void setExportMode(int);
    void nameEditingFinished();
	void selectFolder();
	void updateInfo();
    void setBinaryOutputType(int typeIndex);
    void setDataConversionMode(int);
    void setFileOutputMode(int);
    void beginExport();

signals:
    void exportStarted(ExportOperation *);
    void exportCancelled();

private:
    // All modes
	QLineEdit *m_folderLine;
	QLineEdit *m_nameLine;

    // Image modes
    QGroupBox *m_imageOptionsWidget;
    QComboBox *m_fileTypeBox;

    // Binary mode
    QGroupBox *m_binaryOptionsWidget;
    EmdTypeBox *m_emdTypeBox;
    QButtonGroup *m_binaryFileModeGroup;
    QButtonGroup *m_binaryDataModeGroup;
    NumberRangeWidget *m_binaryDataLimits;

    // Info and export
    QGroupBox *m_infoWidget;
	QLabel *m_infoLabel;
    QPushButton *m_exportButton;

    Mode m_mode;
    QString m_fileNameStem;
    int m_frameCount;
    int m_frameHSize;
    int m_frameVSize;
    bool m_customFileName;

    bool m_exportInProgress;
};

} // namespace emd

#endif
