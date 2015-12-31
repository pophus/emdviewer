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

#include "WorkflowBrowser.h"

#include <QtWidgets>

#include "Workflow.h"
#include "WorkflowManager.h"

namespace emd
{

WorkflowBrowser::WorkflowBrowser(QWidget *parent)
    : QWidget(parent),
    m_workflowManager(new WorkflowManager())
{
    QPushButton *saveButton = new QPushButton("Save");
    connect(saveButton, SIGNAL(clicked()),
        this, SLOT(saveWorkflow()));

    QPushButton *createButton = new QPushButton("Create...");
    createButton->setEnabled(false);
    connect(createButton, SIGNAL(clicked()),
        this, SLOT(createWorkflow()));

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(createButton);

    QTreeView *treeView = new QTreeView();
    treeView->setModel(m_workflowManager);
    connect(treeView, SIGNAL(clicked(const QModelIndex &)),
        this, SLOT(highlightWorkflow(const QModelIndex &)));
    connect(treeView, SIGNAL(doubleClicked(const QModelIndex &)),
        this, SLOT(selectWorkflow(const QModelIndex &)));

    m_descriptionLabel = new QLabel();
    m_descriptionLabel->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(buttonLayout);
    layout->addWidget(treeView, 4);
    layout->addWidget(m_descriptionLabel, 1, Qt::AlignTop);
    this->setLayout(layout);

    declareWorkflows();

    m_workflowManager->refresh();

    treeView->expandAll();
}

void WorkflowBrowser::setCurrentWorkflowName(const QString &name)
{
    m_currentWorkflowName = name;
}

void WorkflowBrowser::declareWorkflows()
{
    QDir root = QDir(qApp->applicationDirPath() + "/workflows");
    root.makeAbsolute();
    if(!root.exists())
    {
        qWarning() << "Workflow directory not found.";
        return;
    }
    
    QStringList subDirs = root.entryList(QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot);
    
    for(const QString &subDirName : subDirs)
    {
        QDir subDir = root.absoluteFilePath(subDirName);

        QList<QFileInfo> workflowFiles = subDir.entryInfoList(QStringList("*.xml"));

        for(const QFileInfo &info : workflowFiles)
        {
            Workflow::declare(subDirName.toStdString(), info.baseName().toStdString(), this);
        }
    }
}

void WorkflowBrowser::configureWorkflow(const std::string &group,
                                        const std::string &name,
                                        Workflow &workflow)
{
    QDir root = QDir(qApp->applicationDirPath() + "/workflows");
    root.makeAbsolute();
    if(!root.exists())
    {
        qWarning() << "Workflow directory not found.";
        return;
    }

    QString path = root.absolutePath() + "/" + QString(group.c_str()) 
        + "/" + QString(name.c_str()) + ".xml";

    bool success = workflow.load(path);
    if(!success)
    {
        qCritical() << "Failed to load workflow file: " << path;
    }
}

/********************************** Slots **********************************/

void WorkflowBrowser::highlightWorkflow(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    m_descriptionLabel->setText(m_workflowManager->workflowDescription(index));
}

void WorkflowBrowser::selectWorkflow(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    Workflow *workflow = m_workflowManager->workflow(index);

    m_currentWorkflowName = workflow->name();

    emit(workflowSelected(workflow));
}

void WorkflowBrowser::saveWorkflow()
{
    QLabel *groupLabel = new QLabel("Group:");

    QLineEdit *groupLine = new QLineEdit("Custom");

    QLabel *nameLabel = new QLabel("Name:"); 

    QLineEdit *nameLine = new QLineEdit(m_currentWorkflowName);

    QPushButton *cancelButton = new QPushButton("Cancel");

    QPushButton *saveButton = new QPushButton("Save");
    saveButton->setDefault(true);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(saveButton);

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(groupLabel, 0, 0, 1, 1, Qt::AlignRight);
    layout->addWidget(groupLine, 0, 1, 1, 1, Qt::AlignLeft);
    layout->addWidget(nameLabel, 1, 0, 1, 1, Qt::AlignRight);
    layout->addWidget(nameLine, 1, 1, 1, 1, Qt::AlignLeft);   
    layout->addLayout(buttonLayout, 2, 0, 1, -1);

    QDialog *dialog = new QDialog();
    dialog->setWindowTitle("Save Workflow");
    dialog->setLayout(layout);
    connect(cancelButton, SIGNAL(clicked()),
        dialog, SLOT(reject()));
    connect(saveButton, SIGNAL(clicked()),
        dialog, SLOT(accept()));

    int continueSave = dialog->exec();

    while(continueSave == 1)
    {
        QDir rootDir(qApp->applicationDirPath() + "/workflows");
        if(!rootDir.exists())
        {
            QDir appDir(qApp->applicationDirPath());
            bool success = appDir.mkdir("workflows");
            if(!success)
            {
                qWarning() << "Failed to create root workflow dir.";
                delete dialog;
                return;
            }
        }

        if(!rootDir.exists(groupLine->text()))
        {
            bool success = rootDir.mkdir(groupLine->text());
            if(!success)
            {
                qWarning() << "Failed to create workflow dir: " << groupLine->text();
                delete dialog;
                return;
            }
        }

        QString filePath = qApp->applicationDirPath() + "/workflows/" 
            + groupLine->text() + "/" + nameLine->text() + ".xml";

        QFileInfo info(filePath);
        bool save = true;
        if(info.exists())
        {
            QMessageBox::StandardButton result = 
                QMessageBox::warning(dialog, "Workflow Name Conflict",
                    "A workflow with this name already exists within the specified group.\n"
                    "If you continue, the existing file will be annihilated.",
                    QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

            save = (result == QMessageBox::Ok);
        }
        
        if(save)
        {
            continueSave = 0;
            emit(saveRequested(filePath));
        }
        else
        {
            continueSave = dialog->exec();
        }
    }

    delete dialog;
}

void WorkflowBrowser::createWorkflow()
{

}

}
