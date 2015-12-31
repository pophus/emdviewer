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

#include "FileExporter.h"

#include <stdlib.h>

#include "BinaryExport.h"
#include "ImageExport.h"
#include "NumberRangeWidget.h"
#include "ViewExport.h"

namespace emd
{

FileExporter::FileExporter(QWidget *parent)
    : QWidget(parent),
    m_customFileName(false),
    m_exportInProgress(false)
{
    // Export mode
    QPushButton *viewTypeButton = new QPushButton("View");
    viewTypeButton->setCheckable(true);
    viewTypeButton->setChecked(true);

    QPushButton *imageTypeButton = new QPushButton("Image");
    imageTypeButton->setCheckable(true);

    QPushButton *binaryTypeButton = new QPushButton("Binary");
    binaryTypeButton->setCheckable(true);

    QButtonGroup *typeGroup = new QButtonGroup(this);
    typeGroup->addButton(viewTypeButton, 0);
    typeGroup->addButton(imageTypeButton, 1);
    typeGroup->addButton(binaryTypeButton, 2);
    typeGroup->setExclusive(true);
    connect(typeGroup, SIGNAL(buttonClicked(int)),
        this, SLOT(setExportMode(int)));

    QHBoxLayout *typeButtonLayout = new QHBoxLayout();
    typeButtonLayout->addWidget(viewTypeButton);
    typeButtonLayout->addWidget(imageTypeButton);
    typeButtonLayout->addWidget(binaryTypeButton);

    // Folder selection
	m_folderLine = new QLineEdit();

	QPushButton *folderSelectButton = new QPushButton("Folder:");
    folderSelectButton->setMinimumWidth(0);
	connect(folderSelectButton, SIGNAL(clicked()), 
        this, SLOT(selectFolder()));

    QHBoxLayout *folderLayout = new QHBoxLayout();
    folderLayout->addWidget(folderSelectButton);
    folderLayout->addWidget(m_folderLine);


	// File name
	QLabel *nameLabel = new QLabel("Name:");

	m_nameLine = new QLineEdit();
    connect(m_nameLine, SIGNAL(editingFinished()),
        this, SLOT(nameEditingFinished()));

	QHBoxLayout *nameLayout = new QHBoxLayout();
	nameLayout->addWidget(nameLabel);
	nameLayout->addWidget(m_nameLine);

    // Path layout
    QVBoxLayout *pathLayout = new QVBoxLayout();
    pathLayout->addLayout(folderLayout);
    pathLayout->addLayout(nameLayout);

    // Path group box
    QGroupBox *pathGroup = new QGroupBox("");
    pathGroup->setFlat(true);
    pathGroup->setLayout(pathLayout);


    // Image file type
    QLabel *fileTypeLabel = new QLabel("File Type:");
    fileTypeLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

	m_fileTypeBox = new QComboBox();
	m_fileTypeBox->addItem(".png", "PNG");
	m_fileTypeBox->addItem(".jpg", "JPG");
	m_fileTypeBox->addItem(".tiff", "TIFF");
	m_fileTypeBox->addItem(".bmp", "BMP");
    m_fileTypeBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Image Options layout
    QHBoxLayout *imageOptionsLayout = new QHBoxLayout();
    imageOptionsLayout->addWidget(fileTypeLabel);
    imageOptionsLayout->addWidget(m_fileTypeBox, 0, Qt::AlignLeft);

    // Image options widget
    m_imageOptionsWidget = new QGroupBox("Image Options");
    m_imageOptionsWidget->setFlat(true);
    m_imageOptionsWidget->setLayout(imageOptionsLayout);


    // Binary options

    // Type label
    QLabel *typeLabel = new QLabel("Type:");
    typeLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    // Type box
    m_emdTypeBox = new EmdTypeBox();
    m_emdTypeBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    connect(m_emdTypeBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setBinaryOutputType(int)));

    QHBoxLayout *typeBoxLayout = new QHBoxLayout();
    typeBoxLayout->addWidget(typeLabel);
    typeBoxLayout->addWidget(m_emdTypeBox, 0, Qt::AlignLeft);

    // Data conversion
    QLabel *conversionLabel = new QLabel("Values:");

    QPushButton *truncateButton = new QPushButton("Truncate");
    truncateButton->setCheckable(true);
    truncateButton->setChecked(true);
    truncateButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    QPushButton *scaleButton = new QPushButton("Scale");
    scaleButton->setCheckable(true);
    scaleButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    m_binaryDataModeGroup = new QButtonGroup(this);
    m_binaryDataModeGroup->addButton(truncateButton, BinaryOutputModule::ProcessingModeTruncate);
    m_binaryDataModeGroup->addButton(scaleButton, BinaryOutputModule::ProcessingModeScale);
    connect(m_binaryDataModeGroup, SIGNAL(buttonClicked(int)),
        this, SLOT(setDataConversionMode(int)));

    QHBoxLayout *dataConversionLayout = new QHBoxLayout();
    dataConversionLayout->addWidget(conversionLabel);
    dataConversionLayout->addWidget(truncateButton);
    dataConversionLayout->addWidget(scaleButton);

    // Data limits
    QLabel *limitsLabel = new QLabel("Limits:");

    m_binaryDataLimits = new NumberRangeWidget();
    m_binaryDataLimits->setType(false);
    m_binaryDataLimits->setEnabled(false);

    QHBoxLayout *limitsLayout = new QHBoxLayout();
    limitsLayout->addWidget(limitsLabel);
    limitsLayout->addWidget(m_binaryDataLimits);

    // File Output
    QLabel *outputLabel = new QLabel("Output:");

    QPushButton *individualButton = new QPushButton("Individual");
    individualButton->setCheckable(true);
    individualButton->setChecked(true);
    individualButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    QPushButton *combinedButton = new QPushButton("Combined");
    combinedButton->setCheckable(true);
    combinedButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    m_binaryFileModeGroup = new QButtonGroup(this);
    m_binaryFileModeGroup->addButton(individualButton, BinaryOutputModule::OutputModeInvidual);
    m_binaryFileModeGroup->addButton(combinedButton, BinaryOutputModule::OutputModeGrouped);
    connect(m_binaryFileModeGroup, SIGNAL(buttonClicked(int)),
        this, SLOT(setFileOutputMode(int)));

    QHBoxLayout *fileOutputLayout = new QHBoxLayout();
    fileOutputLayout->addWidget(outputLabel);
    fileOutputLayout->addWidget(individualButton);
    fileOutputLayout->addWidget(combinedButton);

    QVBoxLayout *binaryOptionsLayout = new QVBoxLayout();
    binaryOptionsLayout->addLayout(typeBoxLayout);
    binaryOptionsLayout->addLayout(dataConversionLayout);
    binaryOptionsLayout->addLayout(limitsLayout);
    binaryOptionsLayout->addLayout(fileOutputLayout);

    m_binaryOptionsWidget = new QGroupBox("Binary Data Options");
    m_binaryOptionsWidget->setFlat(true);
    m_binaryOptionsWidget->setLayout(binaryOptionsLayout);


    // Info and export
	m_infoLabel = new QLabel("");

	m_exportButton = new QPushButton("Export");
    // TODO: device independence!
    m_exportButton->setMaximumWidth(70);
	connect(m_exportButton, SIGNAL(clicked()), 
        this, SLOT(beginExport()));

    QHBoxLayout *infoLayout = new QHBoxLayout();
    infoLayout->addWidget(m_infoLabel);
    infoLayout->addWidget(m_exportButton);

    m_infoWidget = new QGroupBox("");
    m_infoWidget->setFlat(true);
    m_infoWidget->setLayout(infoLayout);
    
    
	// Main Layout
	QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(typeButtonLayout);
    mainLayout->addWidget(pathGroup);
    mainLayout->addWidget(m_imageOptionsWidget);
    mainLayout->addWidget(m_binaryOptionsWidget);
    mainLayout->addWidget(m_infoWidget);
	mainLayout->addStretch();
	setLayout(mainLayout);

	// Setup
	folderSelectButton->setDefault(false);

    setExportMode(FileExporter::ViewMode);

    // Calls updateInfo
    setBinaryOutputType(0);
}

void FileExporter::setSelection(const FrameSet::Selection &selection)
{
    m_frameCount = selection.count();
    m_frameHSize = selection.horizontalSize();
    m_frameVSize = selection.verticalSize();

    if(m_frameCount > 0)
        updateInfo();
}

FileExporter::Mode FileExporter::mode() const
{
    return m_mode;
}

const QString FileExporter::fileNameStem() const
{
    return m_nameLine->text();
}

void FileExporter::setFileNameStem(const QString &stem)
{
    m_fileNameStem = stem;

    m_customFileName = false;
}

const QString FileExporter::exportDirectory() const
{
    return m_folderLine->text();
}

void FileExporter::setExportDirectory(const QString &dir)
{
    m_folderLine->setText(dir);
}

const QString FileExporter::fileSuffix() const
{
    if(m_mode == ImageMode || m_mode == ViewMode)
        return m_fileTypeBox->currentText();
    else if(m_mode == BinaryMode)
        return ".dat";

    return QString();
}

BinaryOutputModule::OutputMode FileExporter::binaryOutputMode() const
{
    return (BinaryOutputModule::OutputMode) m_binaryFileModeGroup->checkedId();
}

BinaryOutputModule::ProcessingMode FileExporter::binaryProcessingMode() const
{
    return (BinaryOutputModule::ProcessingMode) m_binaryDataModeGroup->checkedId();
}

DataType FileExporter::binaryDataType() const
{
    return m_emdTypeBox->currentType();
}

void FileExporter::finishExport()
{
    m_exportButton->setText("Export");

    m_exportInProgress = false;
}

void FileExporter::updateFileName()
{
    if(m_customFileName)
        return;
    
    QString fileName = m_fileNameStem;

    switch (m_mode)
    {
    case emd::FileExporter::BinaryMode:
        fileName += QString("-%1x%2").arg(m_frameHSize).arg(m_frameVSize);

        if(m_binaryFileModeGroup->checkedId() == BinaryOutputModule::OutputModeInvidual)
        {
            // Nothing else to add
        }
        else if(m_binaryFileModeGroup->checkedId() == BinaryOutputModule::OutputModeGrouped)
        {
            fileName += QString("x%1").arg(m_frameCount);
        }
        break;
    case emd::FileExporter::ViewMode:
    case emd::FileExporter::ImageMode:
    default:
        break;
    }

    m_nameLine->setText(fileName);
}

/******************************** Slots *********************************/

void FileExporter::setExportMode(int mode)
{
    switch(mode)
    {
    case ViewMode:
        m_imageOptionsWidget->setHidden(false);
        m_binaryOptionsWidget->setHidden(true);
        break;

    case ImageMode:
        m_imageOptionsWidget->setHidden(false);
        m_binaryOptionsWidget->setHidden(true);
        break;

    case BinaryMode:
        m_imageOptionsWidget->setHidden(true);
        m_binaryOptionsWidget->setHidden(false);
        break;
    }

    m_mode = (Mode) mode;

    updateInfo();
}

void FileExporter::nameEditingFinished()
{
    m_customFileName = true;
}

void FileExporter::selectFolder()
{
	QFileDialog *dialog = new QFileDialog(this, "Select Folder", 
		m_folderLine->text(), "Folders");
	dialog->setFileMode(QFileDialog::Directory);
	if(dialog->exec())
		m_folderLine->setText(dialog->selectedFiles().first());
}

void FileExporter::updateInfo()
{
    if(m_mode == ViewMode)
    {
        m_infoLabel->setText("");
        return;
    }

    float displaySize;
    QString sizeUnits;
    if(m_mode == BinaryMode)
    {
        int typeSize = emdTypeDepth(m_emdTypeBox->currentType());
        int frameSize = typeSize * m_frameHSize * m_frameVSize;

        displaySize = frameSize;

        if(m_binaryFileModeGroup->checkedId() == BinaryOutputModule::OutputModeGrouped)
            displaySize *= m_frameCount;

        int steps = 0;
        while(displaySize > 1024)
        {
            displaySize /= 1024;
            ++steps;
        }

        switch(steps)
        {
        case 0:
            sizeUnits = "B";
            break;
        case 1:
            sizeUnits = "KB";
            break;
        case 2:
            sizeUnits = "MB";
            break;
        case 3:
            sizeUnits = "GB";
            break;
        default:
            sizeUnits = "??";
            break;
        }
    }

    QString itemString;
    QString itemSizeString;
    QString countString;

    switch (m_mode)
    {
    case emd::FileExporter::ImageMode:
        itemString = m_frameCount > 1 ? "images" : "image";
        countString = QString("%1").arg(m_frameCount);
        break;
    case emd::FileExporter::BinaryMode:
        if(m_binaryFileModeGroup->checkedId() == BinaryOutputModule::OutputModeInvidual)
        {
            itemString = m_frameCount > 1 ? "frames" : "frame";

            itemSizeString = QString(" (%1 %2 each)").arg(displaySize, 0, 'f', 2).arg(sizeUnits);

            countString = QString("%1").arg(m_frameCount);
        }
        else if(m_binaryFileModeGroup->checkedId() == BinaryOutputModule::OutputModeGrouped)
        {
            itemString = "file";

            itemSizeString = QString(" (%1 %2)").arg(displaySize, 0, 'f', 2).arg(sizeUnits);
            
            countString = "1";
        }
        break;
    default:
        break;
    }

	m_infoLabel->setText(QString("%1 %2 will be exported%3.")
        .arg(countString)
        .arg(itemString)
        .arg(itemSizeString));

    updateFileName();
}

void FileExporter::setBinaryOutputType(int)
{
    updateInfo();

    switch (m_emdTypeBox->currentType())
    {
    case DataTypeInt8:
		m_binaryDataLimits->setLimits(std::numeric_limits<char>::min(), std::numeric_limits<char>::max());
		break;
	case DataTypeUInt8:
		m_binaryDataLimits->setLimits(std::numeric_limits<unsigned char>::min(), std::numeric_limits<unsigned char>::max());
		break;
	case DataTypeInt16:
		m_binaryDataLimits->setLimits(std::numeric_limits<short>::min(), std::numeric_limits<short>::max());
		break;
	case DataTypeUInt16:
		m_binaryDataLimits->setLimits(std::numeric_limits<unsigned short>::min(), std::numeric_limits<unsigned short>::max());
		break;
	case DataTypeInt32:
	case DataTypeInt64:
		m_binaryDataLimits->setLimits(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
		break;
	case DataTypeUInt32:
	case DataTypeUInt64:
		m_binaryDataLimits->setLimits(std::numeric_limits<uint32_t>::min(), std::numeric_limits<int32_t>::max());
		break;
	case DataTypeFloat32:
	case DataTypeFloat64:
		m_binaryDataLimits->setLimits(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
		break;
    default:
        break;
    }
}

void FileExporter::setDataConversionMode(int)
{
    switch (this->binaryProcessingMode())
    {
    case BinaryOutputModule::ProcessingModeTruncate:
        m_binaryDataLimits->setEnabled(false);
        break;
    case BinaryOutputModule::ProcessingModeScale:
        m_binaryDataLimits->setEnabled(true);
        break;
    default:
        break;
    }
}

void FileExporter::setFileOutputMode(int)
{
    updateInfo();
}

void FileExporter::beginExport()
{
    if(!m_exportInProgress)
    {
        m_exportInProgress = true;

        m_exportButton->setText("Cancel");

        ExportOperation *exportOperation = nullptr;

        switch(m_mode)
        {
        case ViewMode:
            exportOperation = new ViewExport();
            break;
        case ImageMode:
            exportOperation = new ImageExport();
            break;
        case BinaryMode:
            BinaryExport *op = new BinaryExport();

            BinaryOutputModule *module = new BinaryOutputModule();
            module->setOutputMode(binaryOutputMode());
            module->setProcessingMode(binaryProcessingMode());
            module->setOutputType(binaryDataType());

            if(binaryProcessingMode() == BinaryOutputModule::ProcessingModeScale)
            {
                if(isFloatType(binaryDataType()))
                {
                    float min, max;
                    m_binaryDataLimits->getFloatValues(min, max);
                    module->setScalingLimits(QVariant(min), QVariant(max));
                }
                else
                {
                    float min, max;
                    m_binaryDataLimits->getFloatValues(min, max);
                    module->setScalingLimits(QVariant(min), QVariant(max));
                }
            }

            op->setOutputModule(module);
            exportOperation = op;
            break;
        }

        if(exportOperation)
        {
            exportOperation->setFileStem(m_nameLine->text());
            exportOperation->setOutputDirectory(exportDirectory());
            exportOperation->setFileSuffix(fileSuffix());

            emit(exportStarted(exportOperation));
        }
    }
    else
    {
        emit(exportCancelled());
    }
}

} // namespace emd
