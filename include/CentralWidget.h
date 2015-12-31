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

#ifndef EMD_CENTRALWIDGET_H
#define EMD_CENTRALWIDGET_H

#include <qwidget.h>

#include "Dataset.h"
#include "FrameSet.h"

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QVBoxLayout;

namespace emd
{

class BinaryOutputModule;
class DataGroup;
class DataGroupModule;
class DimensionsPane;
class ExportOperation;
class HistogramModule;
class ImageWindowModule;
class MainImageWidget;
class Model;
class Workflow;

class CentralWidget : public QWidget
{
    Q_OBJECT

public:
    CentralWidget(QWidget *parent = 0);
    ~CentralWidget();

    void reset(DataGroup *dataGroup);

    DataGroup *dataGroup() const;
    FrameSet::Selection selection() const;

    QWidget *workflowControlWidget();

    Workflow *workflow() const;
    void setWorkflow(Workflow *workflow);

    void setSelectionMode(bool selection); 

    void performExport(ExportOperation *op);
    void cancelExport();

private:
    void resetControls();
    void updateImageSizeLabel();
    void finishExport(bool cancelled);

private slots:
    void toggleAxisLock(bool);
    void zoomInPrimary();
    void zoomOutPrimary();
    void zoomChangedPrimary(int index);
    void zoomAddedPrimary();
    void zoomInSecondary();
    void zoomOutSecondary();
    void zoomChangedSecondary(int index);
    void zoomAddedSecondary();
    void handleMouseWheelUp();
    void handleMouseWheelDown();
    void togglePrimaryTiling(bool);
    void toggleSecondaryTiling(bool);

    void updateSelection(const FrameSet::Selection &);
    void updateCursorPosition(float x, float y);

    void workflowFinished();

    void requestDimensionSelection(int index, bool select);

signals:
    void selectionChanged(const FrameSet::Selection &state);
    void exportFinished();

private:
    DataGroup *m_dataGroup;
    Workflow *m_workflow;
    ImageWindowModule *m_imageOutputModule;
    HistogramModule *m_histogramModule;
    BinaryOutputModule *m_binaryOutputModule;
    
    QVBoxLayout *m_workflowLayout;
    QLabel *m_workflowTitle;
    QWidget *m_workflowControlWidget;
    QWidget *m_customWorkflowWidget;

    MainImageWidget *m_imageWidget;

    QGroupBox *m_imageInfoWidget;
    QLabel *m_imageValueLabel;
    QLabel *m_imageUnitsLabel;
    QLabel *m_imageCursorPositionXLabel;
    QLabel *m_imageCursorPositionYLabel;

    QWidget *m_imageControlsWidget;

    QComboBox *m_primaryScaleBox;
    QLabel *m_primaryScaleLabel;

    QComboBox *m_secondaryScaleBox;
    QWidget *m_secondaryScaleWidget;

    QPushButton *m_primaryTileButton;
    QPushButton *m_secondaryTileButton;

    DimensionsPane *m_dimensionsPane;
    QGroupBox *m_dimensionGroup;

    bool m_axesLocked;

    int m_zoomLevelCount;

    ExportOperation *m_exportOperation;

    FrameSet::Selection m_cachedSelection;
};

} // namespace emd

#endif