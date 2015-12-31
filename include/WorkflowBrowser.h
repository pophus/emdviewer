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

#ifndef EMD_WORKFLOWBROWSER_H
#define EMD_WORKFLOWBROWSER_H

#include <qwidget.h>

#include "WorkflowSource.h"

class QLabel;

namespace emd
{

class Workflow;
class WorkflowManager;

class WorkflowBrowser : public QWidget,
                        public WorkflowSource
{
    Q_OBJECT

public:
    WorkflowBrowser(QWidget *parent = 0);

    void setCurrentWorkflowName(const QString &name);

    void declareWorkflows() override;
    void configureWorkflow(const std::string &group,
                           const std::string &name,
                           Workflow &workflow) override;

private slots:
    void highlightWorkflow(const QModelIndex &index);
    void selectWorkflow(const QModelIndex &index);
    void saveWorkflow();
    void createWorkflow();

signals:
    void workflowSelected(Workflow *workflow);
    void saveRequested(const QString &path);

private:
    WorkflowManager *m_workflowManager;
    QLabel *m_descriptionLabel;
    QString m_currentWorkflowName;
};

} // namespace emd

#endif