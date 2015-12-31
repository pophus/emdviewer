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

#include "CentralWidget.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qvalidator.h>

#include <Model.h>

#include "BinaryExport.h"
#include "BinaryOutputModule.h"
#include "DataGroupModule.h"
#include "DimensionsPane.h"
#include "ExportOperation.h"
#include "HistogramModule.h"
#include "ImageExport.h"
#include "ImageWindowModule.h"
#include "MainGraphicsImageWidget.h"
#include "MessageModel.h"
#include "StyleConfig.h"
#include "ViewExport.h"
#include "Workflow.h"

extern emd::MessageModel *messageLog;

namespace emd
{

CentralWidget::CentralWidget(QWidget *parent)
    : QWidget(parent),
    m_customWorkflowWidget(nullptr),
    m_axesLocked(true),
    m_workflow(nullptr),
    m_exportOperation(nullptr),
    m_histogramModule(nullptr)
{
    m_imageWidget = new MainGraphicsImageWidget();
    connect(m_imageWidget, SIGNAL(cursorPositionChanged(float, float)),
        this, SLOT(updateCursorPosition(float, float)));
    connect(m_imageWidget, SIGNAL(mouseWheelDown()),
        this, SLOT(handleMouseWheelDown()));
    connect(m_imageWidget, SIGNAL(mouseWheelUp()),
        this, SLOT(handleMouseWheelUp()));

    // Info label
    m_imageInfoWidget = new QGroupBox("Image");
    m_imageInfoWidget->setFlat(true);

    m_imageValueLabel = new QLabel("1.0000e00");
    //m_imageValueLabel->setAlignment(Qt::AlignRight);
    //m_imageValueLabel->adjustSize();
    //m_imageValueLabel->setFixedWidth(m_imageValueLabel->width());

    m_imageUnitsLabel = new QLabel("[long-units]");
    //m_imageUnitsLabel->adjustSize();
    //m_imageUnitsLabel->setFixedWidth(m_imageUnitsLabel->width());

    QHBoxLayout *imageValueLayout = new QHBoxLayout();
    imageValueLayout->addWidget(m_imageValueLabel, 0, Qt::AlignRight);
    imageValueLayout->addWidget(m_imageUnitsLabel, 0, Qt::AlignLeft);

    QLabel *commaLabel = new QLabel(",");

    m_imageCursorPositionXLabel = new QLabel("(000.000");
    m_imageCursorPositionXLabel->setAlignment(Qt::AlignRight);
    m_imageCursorPositionXLabel->adjustSize();
    m_imageCursorPositionXLabel->setFixedWidth(m_imageCursorPositionXLabel->width());

    m_imageCursorPositionYLabel = new QLabel("000.000)");
    m_imageCursorPositionYLabel->setAlignment(Qt::AlignLeft);
    m_imageCursorPositionYLabel->adjustSize();
    m_imageCursorPositionYLabel->setFixedWidth(m_imageCursorPositionYLabel->width());

    QHBoxLayout *cursorPositionLayout = new QHBoxLayout();
    cursorPositionLayout->addWidget(m_imageCursorPositionXLabel, 0, Qt::AlignRight);
    cursorPositionLayout->addWidget(commaLabel, 0, Qt::AlignLeft);
    cursorPositionLayout->addWidget(m_imageCursorPositionYLabel, 0, Qt::AlignLeft);

    QVBoxLayout *imageInfoLayout = new QVBoxLayout();
    imageInfoLayout->addLayout(imageValueLayout);
    imageInfoLayout->addLayout(cursorPositionLayout);
    //imageInfoLayout->setContentsMargins(0, 0, 0, 0);
    //imageInfoLayout->setSpacing(5);
    m_imageInfoWidget->setLayout(imageInfoLayout);

    // Axis lock
    QPushButton *lockedAxesButton = new QPushButton("Lock");
    lockedAxesButton->setCheckable(true);
    lockedAxesButton->setChecked(true);
    lockedAxesButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    lockedAxesButton->setMinimumWidth(40);
    connect(lockedAxesButton, SIGNAL(toggled(bool)),
        this, SLOT(toggleAxisLock(bool)));

    // First scale widget
    m_primaryScaleLabel = new QLabel("H:");

    QPushButton *primaryZoomOutButton = new QPushButton("-");
    primaryZoomOutButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QSize hint = primaryZoomOutButton->sizeHint();
    primaryZoomOutButton->setMaximumWidth(hint.height());
    connect(primaryZoomOutButton, SIGNAL(clicked()),
        this, SLOT(zoomOutPrimary()));

    m_primaryScaleBox = new QComboBox();
    m_primaryScaleBox->addItem("25", QVariant(25));
    m_primaryScaleBox->addItem("50", QVariant(50));
    m_primaryScaleBox->addItem("75", QVariant(75));
    m_primaryScaleBox->addItem("100", QVariant(100));
    m_primaryScaleBox->addItem("125", QVariant(125));
    m_primaryScaleBox->addItem("150", QVariant(150));
    m_primaryScaleBox->addItem("200", QVariant(200));
    m_primaryScaleBox->addItem("300", QVariant(300));
    m_primaryScaleBox->addItem("400", QVariant(400));
    m_primaryScaleBox->addItem("600", QVariant(600));
    m_primaryScaleBox->addItem("800", QVariant(800));
    m_primaryScaleBox->setEditable(true);
    m_primaryScaleBox->setCurrentIndex(3);
    connect(m_primaryScaleBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(zoomChangedPrimary(int)));
    connect(m_primaryScaleBox->lineEdit(), SIGNAL(editingFinished()),
        this, SLOT(zoomAddedPrimary()));

    QIntValidator *primaryValidator = new QIntValidator(10, 999);
    m_primaryScaleBox->setValidator(primaryValidator);

    QPushButton *primaryZoomInButton = new QPushButton("+");
    hint = primaryZoomInButton->sizeHint();
    primaryZoomInButton->setMaximumWidth(hint.height());
    connect(primaryZoomInButton, SIGNAL(clicked()),
        this, SLOT(zoomInPrimary()));

    QHBoxLayout *primaryScaleLayout = new QHBoxLayout();
    primaryScaleLayout->addWidget(m_primaryScaleLabel, 0, Qt::AlignRight);
    primaryScaleLayout->addWidget(primaryZoomOutButton, 0, Qt::AlignRight);
    primaryScaleLayout->addWidget(m_primaryScaleBox, 0, Qt::AlignCenter);
    primaryScaleLayout->addWidget(primaryZoomInButton, 0, Qt::AlignLeft);
    primaryScaleLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *primaryScaleWidget = new QWidget();
    primaryScaleWidget->setLayout(primaryScaleLayout);
    primaryScaleWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // Second scale widget
    QLabel *secondaryScaleLabel = new QLabel("V:");

    QPushButton *secondaryZoomOutButton = new QPushButton("-");
    secondaryZoomOutButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    hint = secondaryZoomOutButton->sizeHint();
    secondaryZoomOutButton->setMaximumWidth(hint.height());
    connect(secondaryZoomOutButton, SIGNAL(clicked()),
        this, SLOT(zoomOutSecondary()));

    m_secondaryScaleBox = new QComboBox();
    m_secondaryScaleBox->addItem("25", QVariant(25));
    m_secondaryScaleBox->addItem("50", QVariant(50));
    m_secondaryScaleBox->addItem("75", QVariant(75));
    m_secondaryScaleBox->addItem("100", QVariant(100));
    m_secondaryScaleBox->addItem("125", QVariant(125));
    m_secondaryScaleBox->addItem("150", QVariant(150));
    m_secondaryScaleBox->addItem("200", QVariant(200));
    m_secondaryScaleBox->addItem("300", QVariant(300));
    m_secondaryScaleBox->addItem("400", QVariant(400));
    m_secondaryScaleBox->addItem("600", QVariant(600));
    m_secondaryScaleBox->addItem("800", QVariant(800));
    m_secondaryScaleBox->setEditable(true);
    m_secondaryScaleBox->setCurrentIndex(3);
    connect(m_secondaryScaleBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(zoomChangedSecondary(int)));
    connect(m_secondaryScaleBox->lineEdit(), SIGNAL(editingFinished()),
        this, SLOT(zoomAddedSecondary()));

    QIntValidator *secondaryValidator = new QIntValidator(10, 999);
    m_secondaryScaleBox->setValidator(secondaryValidator);

    QPushButton *secondaryZoomInButton = new QPushButton("+");
    hint = secondaryZoomInButton->sizeHint();
    secondaryZoomInButton->setMaximumWidth(hint.height());
    connect(secondaryZoomInButton, SIGNAL(clicked()),
        this, SLOT(zoomInSecondary()));

    QHBoxLayout *secondaryScaleLayout = new QHBoxLayout();
    secondaryScaleLayout->addWidget(secondaryScaleLabel, 0, Qt::AlignRight);
    secondaryScaleLayout->addWidget(secondaryZoomOutButton, 0, Qt::AlignRight);
    secondaryScaleLayout->addWidget(m_secondaryScaleBox, 0, Qt::AlignCenter);
    secondaryScaleLayout->addWidget(secondaryZoomInButton, 0, Qt::AlignLeft);
    secondaryScaleLayout->setContentsMargins(0, 0, 0, 0);

    m_secondaryScaleWidget = new QWidget();
    m_secondaryScaleWidget->setLayout(secondaryScaleLayout);
    m_secondaryScaleWidget->setEnabled(false);

    // Tiling
    m_primaryTileButton = new QPushButton("Tile");
    m_primaryTileButton->setCheckable(true);
    connect(m_primaryTileButton, SIGNAL(toggled(bool)),
        this, SLOT(togglePrimaryTiling(bool)));

    m_secondaryTileButton = new QPushButton("Tile");
    m_secondaryTileButton->setCheckable(true);
    m_secondaryTileButton->setEnabled(false);
    connect(m_secondaryTileButton, SIGNAL(toggled(bool)),
        this, SLOT(toggleSecondaryTiling(bool)));
    
    // Axis controls widget
    QGridLayout *axisControlsLayout = new QGridLayout();
    axisControlsLayout->addWidget(lockedAxesButton, 0, 0, -1, 1, Qt::AlignLeft);
    axisControlsLayout->addWidget(primaryScaleWidget, 0, 1, 1, 1);
    axisControlsLayout->addWidget(m_secondaryScaleWidget, 1, 1, 1, 1);
    axisControlsLayout->addWidget(m_primaryTileButton, 0, 2, 1, 1);
    axisControlsLayout->addWidget(m_secondaryTileButton, 1, 2, 1, 1);
    //axisControlsLayout->setContentsMargins(0, 0, 0, 0);
    axisControlsLayout->setSpacing(5);

    QGroupBox *axisControlsBox = new QGroupBox("Axes");
    axisControlsBox->setFlat(true);
    axisControlsBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    axisControlsBox->setLayout(axisControlsLayout);

    // Region selection
	//QPushButton *polySelectionButton = new QPushButton("Polygon");
	//connect(polySelectionButton, SIGNAL(toggled(bool)),
	//	m_imageWidget, SLOT(setRegionSelectionEnabled(bool)));

 //   QHBoxLayout *regionSelectionLayout = new QHBoxLayout();
 //   regionSelectionLayout->addWidget(polySelectionButton);

 //   QGroupBox *regionSelectionBox = new QGroupBox("Region Selection");
 //   regionSelectionBox->setFlat(true);
 //   regionSelectionBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
 //   regionSelectionBox->setLayout(regionSelectionLayout);

    // Control layout
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(axisControlsBox, 0, Qt::AlignTop);
    //controlLayout->addWidget(regionSelectionBox, 0, Qt::AlignTop);
    controlLayout->addStretch();
    controlLayout->addWidget(m_imageInfoWidget, 0, Qt::AlignTop);
    controlLayout->setSpacing(10);
    controlLayout->setContentsMargins(10, 0, 10, 0);

    // Image controls widget
    m_imageControlsWidget = new QWidget();
    m_imageControlsWidget->setLayout(controlLayout);

    // Dimensions
    m_dimensionsPane = new DimensionsPane(NULL);
    m_dimensionsPane->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    m_dimensionGroup = new QGroupBox("Dimensions");
    m_dimensionGroup->setFlat(true);

    QHBoxLayout *dimGroupLayout = new QHBoxLayout();
    dimGroupLayout->addWidget(m_dimensionsPane);
    m_dimensionGroup->setLayout(dimGroupLayout);

    QHBoxLayout *dummyDimLayout = new QHBoxLayout();
    dummyDimLayout->addWidget(m_dimensionGroup);
    dummyDimLayout->setContentsMargins(10, 0, 10, 0);

    // Main layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_imageWidget);
    layout->addWidget(m_imageControlsWidget);
    layout->addLayout(dummyDimLayout);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);

    // Other setup
    m_zoomLevelCount = m_primaryScaleBox->count();

    this->updateCursorPosition(-1, -1);

    this->updateImageSizeLabel();
    
    // Connections
	connect(m_imageWidget, SIGNAL(imageHPos(float, float)), 
			m_dimensionsPane, SLOT(setHSliderPos(float, float)));
	connect(m_imageWidget, SIGNAL(imageVPos(float, float)), 
			m_dimensionsPane, SLOT(setVSliderPos(float, float)));

	connect(m_dimensionsPane, SIGNAL(hPositionChanged(int)),
		m_imageWidget, SLOT(setHorizontalPos(int)));
	connect(m_dimensionsPane, SIGNAL(vPositionChanged(int)),
		m_imageWidget, SLOT(setVerticalPos(int)));
	connect(m_dimensionsPane, SIGNAL(panningChanged(bool)),
		m_imageWidget, SLOT(setPanning(bool)));

	connect(m_dimensionsPane, SIGNAL(selectionChanged(const FrameSet::Selection &)),
		this, SLOT(updateSelection(const FrameSet::Selection &)));

    m_workflowControlWidget = new QWidget();

    m_workflowTitle = new QLabel("Title");
    m_workflowTitle->setAlignment(Qt::AlignCenter);

    m_workflowLayout = new QVBoxLayout();
    m_workflowLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_workflowLayout->addWidget(m_workflowTitle);

    m_workflowControlWidget->setLayout(m_workflowLayout);
}

CentralWidget::~CentralWidget()
{
    if(m_workflow)
        delete m_workflow;
}

void CentralWidget::reset(DataGroup *dataGroup)
{
    if(!dataGroup)
    {
        qWarning() << "CentralWidget reset with null model or data group";
        return;
    }

    m_dataGroup = dataGroup;

    m_imageWidget->reset();

    resetControls();

    m_dimensionsPane->reset(m_dataGroup);
    m_dimensionsPane->setMinimumSize(m_dimensionsPane->minimumSizeHint());

    // Update image labels
    m_imageUnitsLabel->setText(m_dataGroup->data()->units());
    
    if(m_workflow)
        m_workflow->reset();

    updateSelection(m_dimensionsPane->selection());
}

QWidget *CentralWidget::workflowControlWidget()
{
    return m_workflowControlWidget;
}

Workflow *CentralWidget::workflow() const
{
    return m_workflow;
}

void CentralWidget::setWorkflow(Workflow *workflow)
{
    if(!workflow)
        return;

    if(m_workflow)
        delete m_workflow;

    m_dimensionsPane->setDimensionSelection(-1, false);

    m_workflow = workflow;

    connect(workflow, SIGNAL(finishedProcessing()),
        this, SLOT(workflowFinished()));

    const QList<WorkflowModule *> &modules = workflow->modules();
    for(WorkflowModule *module : modules)
    {
        if(module->instanceId() == DataGroupModule::classId())
        {
            DataGroupModule *dg = dynamic_cast<DataGroupModule*>(module);

            if(dg->property("Source").toString().compare("Automatic") == 0)
            {
                //dg->setEnabled(false);
                QVariant var;
                var.setValue(const_cast<const DataGroup *>(m_dataGroup));
                dg->setProperty("DataGroup", var);
                dg->setSelection(m_dimensionsPane->selection());
                connect(m_dimensionsPane, SIGNAL(selectionChanged(const FrameSet::Selection &)),
                    dg, SLOT(setSelection(const FrameSet::Selection &)));
                //dg->setEnabled(true);
            }
        }
        else if(module->instanceId() == ImageWindowModule::classId())
        {
            ImageWindowModule *iw = dynamic_cast<ImageWindowModule*>(module);

            if(iw->property("Output").toString().compare("MainWindow") == 0)
            {
                m_imageOutputModule = iw;

                connect(iw, SIGNAL(imageGenerated(QImage*)),
		            m_imageWidget, SLOT(displayImage(QImage*)));
            }
        }
        else if(module->instanceId() == HistogramModule::classId())
        {
            m_histogramModule = dynamic_cast<HistogramModule*>(module);
        }
        
        // Features
        WorkflowModule::RequiredFeatures features = module->requiredFeatures();

        if(features & WorkflowModule::RangeSelectionFeature)
        {
            module->setSelectionDimensions(m_dimensionsPane->selectionDimensions());

            connect(module, SIGNAL(dimensionSelectionRequested(int, bool)),
                this, SLOT(requestDimensionSelection(int, bool)));
        }
    }

    if(m_customWorkflowWidget)
    {
        m_workflowLayout->removeWidget(m_customWorkflowWidget);
        delete m_customWorkflowWidget;
    }

    m_customWorkflowWidget = workflow->controlWidget();

    if(m_customWorkflowWidget)
        m_workflowLayout->addWidget(m_customWorkflowWidget);

    m_workflowTitle->setText(workflow->group() + "." + workflow->name());
}

void CentralWidget::setSelectionMode(bool selection)
{
    m_dimensionsPane->setSelectionModeEnabled(selection);

    if(selection)
    {
        m_workflow->setPaused(true);

        m_cachedSelection = m_dimensionsPane->selection();
    }
    else
    {
        m_workflow->setPaused(false);

        if(m_cachedSelection.size() > 0)
            m_dimensionsPane->setSelection(m_cachedSelection);
    }
}

DataGroup *CentralWidget::dataGroup() const
{
    return m_dataGroup;
}

FrameSet::Selection CentralWidget::selection() const
{
    return m_dimensionsPane->selection();
}

void CentralWidget::performExport(ExportOperation *op)
{
    op->setItemCount(m_dimensionsPane->selection().count());

    if(ImageExport *imgExp = dynamic_cast<ImageExport *>(op))
    {
        disconnect(m_imageOutputModule, SIGNAL(imageGenerated(QImage *)),
            m_imageWidget, SLOT(displayImage(QImage *)));

        connect(m_imageOutputModule, SIGNAL(imageGenerated(QImage *)),
            imgExp, SLOT(saveExportImage(QImage *)));

        messageLog->addMessage(QString("Begin exporting %1 %2 image(s)")
            .arg(op->itemCount()).arg(op->fileSuffix()));
    }
    else if(BinaryExport *binExp = dynamic_cast<BinaryExport *>(op))
    {
        m_imageOutputModule->setActive(false);

        m_binaryOutputModule = binExp->outputModule();
        m_imageOutputModule->addOutput(m_binaryOutputModule);
        m_binaryOutputModule->addInput(m_imageOutputModule);
        m_workflow->addModule(m_binaryOutputModule);

        messageLog->addMessage(QString("Begin exporting %1 frame%2")
            .arg(op->itemCount()).arg(op->itemCount() > 1 ? "s" : ""));
    }
    else if(ViewExport *viewExp = dynamic_cast<ViewExport *>(op))
    {
        QImage *previewImage = m_imageWidget->windowImage();
        if(!previewImage)
            return;

        QString filePath = op->fullPath();

        bool success = previewImage->save(filePath);

        if(success)
        {
            messageLog->addMessage("Exported image: " + filePath);
        }
        else
        {
            qWarning() << "Failed to export file: " << filePath;
        }

        delete previewImage;

        emit(exportFinished());
        
        return;
    }

    if(m_histogramModule)
        m_histogramModule->setActive(false);
    
    m_workflow->setPaused(false);

    m_exportOperation = op;
}

void CentralWidget::cancelExport()
{
    if(!m_exportOperation)
        return;

    finishExport(true);
}

/************************************ Private ************************************/

void CentralWidget::resetControls()
{
    int defaultIndex = 0;
    while(m_primaryScaleBox->itemData(defaultIndex).toInt() != 100)
        ++defaultIndex;

    m_primaryScaleBox->setCurrentIndex(defaultIndex);
    m_secondaryScaleBox->setCurrentIndex(defaultIndex);
}

void CentralWidget::updateImageSizeLabel()
{
    if(!m_dimensionsPane)
        return;

    FrameSet::Selection selection = m_dimensionsPane->selection();

    if(selection.size() > 0)
    {
        m_imageInfoWidget->setTitle(QString("Image (%1x%2)").arg(selection.horizontalSize()).arg(selection.verticalSize()));
    }
}

void CentralWidget::finishExport(bool cancelled)
{
    QString typeString("unknown");

    if(ImageExport *imgExp = dynamic_cast<ImageExport *>(m_exportOperation))
    {
        connect(m_imageOutputModule, SIGNAL(imageGenerated(QImage *)),
            m_imageWidget, SLOT(displayImage(QImage *)));

        typeString = "image";
    }
    else if(BinaryExport *binExp = dynamic_cast<BinaryExport *>(m_exportOperation))
    {
        m_workflow->removeModule(m_binaryOutputModule);
        m_binaryOutputModule = nullptr;

        m_imageOutputModule->setActive(true);

        typeString = "frame";
    }

    if(!cancelled)
    {
        messageLog->addMessage(QString("Finished exporting %1 %3%2").arg(m_exportOperation->itemCount()).arg(m_exportOperation->itemCount() > 1 ? "s" : "").arg(typeString));

        m_exportOperation->finish();
    }
    else
    {
        messageLog->addMessage("Export cancelled.");

        m_exportOperation->cancel();
    }    

    delete m_exportOperation;
    m_exportOperation = nullptr;

    m_workflow->setPaused(true);

    if(m_histogramModule)
        m_histogramModule->setActive(true);

    emit(exportFinished());
}

/*************************************** Slots **********************************/

void CentralWidget::toggleAxisLock(bool locked)
{
    m_axesLocked = locked;

    if(locked)
    {
        QVariant hScale = m_primaryScaleBox->currentData();
        QVariant vScale = m_secondaryScaleBox->currentData();
        if(hScale.toInt() != vScale.toFloat())
        {
            //m_imageWidget->setVScaling(0.01 * hScale.toFloat());
            m_secondaryScaleBox->setCurrentIndex(m_primaryScaleBox->currentIndex());
        }

        if(m_primaryTileButton->isChecked() != m_secondaryTileButton->isChecked())
        {
            //m_imageWidget->setVTiling(m_primaryTileButton->isChecked());
            m_secondaryTileButton->setChecked(m_primaryTileButton->isChecked());
        }

        m_secondaryScaleWidget->setEnabled(false);

        m_secondaryTileButton->setEnabled(false);
    }
    else
    {
        m_secondaryScaleWidget->setEnabled(true);

        m_secondaryTileButton->setEnabled(true);
    }
}

void CentralWidget::zoomInPrimary()
{
    int index = m_primaryScaleBox->currentIndex();
    if(index < m_primaryScaleBox->count() - 1)
    {
        ++index;
        // Calls zoomChangedPrimary()
        m_primaryScaleBox->setCurrentIndex(index);
    }
}

void CentralWidget::zoomOutPrimary()
{
    int index = m_primaryScaleBox->currentIndex();
    if(index > 0)
    {
        --index;
        // Calls zoomChangedPrimary()
        m_primaryScaleBox->setCurrentIndex(index);
    }
}

void CentralWidget::zoomChangedPrimary(int index)
{
    if(m_axesLocked)
    {
        QVariant scalingPercent = m_primaryScaleBox->itemData(index);
        m_imageWidget->setScaling(0.01 * scalingPercent.toFloat(), 0.01 * scalingPercent.toFloat());
        m_secondaryScaleBox->setCurrentIndex(m_primaryScaleBox->currentIndex());
    }
    else
    {
        QVariant scalingPercent = m_primaryScaleBox->itemData(index);
        m_imageWidget->setHScaling(0.01 * scalingPercent.toFloat());
    }
}

void CentralWidget::zoomAddedPrimary()
{
    if(m_zoomLevelCount == m_primaryScaleBox->count())
        return;

    ++m_zoomLevelCount;

    int lastIndex = m_primaryScaleBox->count() - 1;
    QString itemText = m_primaryScaleBox->itemText(lastIndex);
    m_primaryScaleBox->removeItem(lastIndex);
    int value = itemText.toInt();

    int sortedIndex = 0;
    while(value > m_primaryScaleBox->itemData(sortedIndex).toInt() && sortedIndex < m_primaryScaleBox->count())
        ++sortedIndex;

    m_primaryScaleBox->insertItem(sortedIndex, itemText, QVariant(value));
    m_secondaryScaleBox->insertItem(sortedIndex, itemText, QVariant(value));

    m_primaryScaleBox->setCurrentIndex(sortedIndex);
}

void CentralWidget::zoomInSecondary()
{
    int index = m_secondaryScaleBox->currentIndex();
    if(index < m_secondaryScaleBox->count() - 1)
    {
        ++index;
        // Calls zoomChangedSecondary()
        m_secondaryScaleBox->setCurrentIndex(index);
    }
}

void CentralWidget::zoomOutSecondary()
{
    int index = m_secondaryScaleBox->currentIndex();
    if(index > 0)
    {
        --index;
        // Calls zoomChangedSecondary()
        m_secondaryScaleBox->setCurrentIndex(index);
    }
}

void CentralWidget::zoomChangedSecondary(int index)
{
    QVariant scalingPercent = m_secondaryScaleBox->itemData(index);
    m_imageWidget->setVScaling(0.01 * scalingPercent.toFloat());
}

void CentralWidget::zoomAddedSecondary()
{
    if(m_zoomLevelCount == m_secondaryScaleBox->count())
        return;

    ++m_zoomLevelCount;

    int lastIndex = m_secondaryScaleBox->count() - 1;
    QString itemText = m_secondaryScaleBox->itemText(lastIndex);
    m_secondaryScaleBox->removeItem(lastIndex);
    int value = itemText.toInt();

    int sortedIndex = 0;
    while(value > m_primaryScaleBox->itemData(sortedIndex).toInt() && sortedIndex < m_secondaryScaleBox->count())
        ++sortedIndex;

    m_secondaryScaleBox->insertItem(sortedIndex, itemText, QVariant(value));
    m_primaryScaleBox->insertItem(sortedIndex, itemText, QVariant(value));

    m_secondaryScaleBox->setCurrentIndex(sortedIndex);
}

void CentralWidget::handleMouseWheelUp()
{
    this->zoomInPrimary();

    if(!m_axesLocked)
        this->zoomInSecondary();
}

void CentralWidget::handleMouseWheelDown()
{
    this->zoomOutPrimary();

    if(!m_axesLocked)
        this->zoomOutSecondary();
}

void CentralWidget::togglePrimaryTiling(bool tile)
{
    m_imageWidget->setHTiling(tile);

    if(m_axesLocked)
    {
        //m_imageWidget->setVTiling(tile);
        m_secondaryTileButton->setChecked(m_primaryTileButton->isChecked());
    }
}

void CentralWidget::toggleSecondaryTiling(bool tile)
{
    m_imageWidget->setVTiling(tile);
}

void CentralWidget::updateSelection(const FrameSet::Selection &selection)
{
    updateImageSizeLabel();

    emit(selectionChanged(selection));
}

void CentralWidget::updateCursorPosition(float x, float y)
{
    if(!m_dataGroup)
        return;

    if(x < 0 || y < 0)
    {
        m_imageCursorPositionXLabel->setText("(null");
        m_imageCursorPositionYLabel->setText("null)");

        m_imageValueLabel->setText("(null)");
    }
    else
    {
        m_imageCursorPositionXLabel->setText(QString("(%1").arg(x, -1, 'G', 6));
        m_imageCursorPositionYLabel->setText(QString("%1)").arg(y, -1, 'G', 6));

        int xIndex = (int) x;
        int yIndex = (int) y;

        const Dataset *data = m_dataGroup->data();

        Dataset::Slice slice = m_dimensionsPane->slice();
        for(int index = 0; index < slice.size(); ++index)
        {
            if(slice[index] == Dataset::HorizontalDimension)
                slice[index] = xIndex;
            else if(slice[index] == Dataset::VerticalDimension)
                slice[index] = yIndex;
        }

        QVariant value = data->variantDataValue(slice);

        m_imageValueLabel->setText(QString("%1").arg(value.toFloat(), -1, 'G', 6));
    }
}

void CentralWidget::workflowFinished()
{
    if(m_exportOperation)
    {
        finishExport(false);
    }
}

/********************************** Features ***********************************/

void CentralWidget::requestDimensionSelection(int index, bool select)
{
    m_dimensionsPane->setDimensionSelection(index, select);

    int dims = m_dimensionsPane->selectionDimensions();

    QList<WorkflowModule *> modules = m_workflow->modules();
    for(WorkflowModule *module : modules)
    {
        if(module->requiredFeatures() & WorkflowModule::RangeSelectionFeature)
        {
            module->setSelectionDimensions(dims);
        }
    }
}

} // namespace emd
