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

#include "FileBrowser.h"

#include <QBoxLayout>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qlistwidget.h>
#include <qmenu.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qtreeview.h>
#include <QWidgetAction>

#include "Attribute.h"
#include "FileManager.h"
#include "MessageModel.h"
#include "Model.h"
#include "ModelManager.h"

extern emd::MessageModel *messageLog;

namespace emd
{

const int kRecentFileLimit = 10;

FileBrowser::FileBrowser(ModelManager *modelManager, QWidget *parent)
    : QWidget(parent),
    m_modelManager(modelManager),
    m_recentFileTimer(NULL)
{
    m_openFileButton = new QPushButton("Open");
	connect(m_openFileButton, SIGNAL(clicked()), 
        this, SLOT(selectFile()));
	connect(m_openFileButton, SIGNAL(pressed()), 
        this, SLOT(startRecentFileTimer()));
	connect(m_openFileButton, SIGNAL(released()), 
        this, SLOT(cancelRecentFileTimer()));
    
//    this->setStyleSheet(" emd--FileBrowser { background-color: red; } ");
    this->setAutoFillBackground(true);

    QSettings settings;
    settings.beginGroup("FileBrowser");
    QString fileList = settings.value("RecentFiles").toString();
    settings.endGroup();

    if(fileList.length() > 0)
        m_recentFileList = fileList.split("&");

    m_recentFileMenu = NULL;
    updateRecentFileMenu();

	QPushButton *importFileButton = new QPushButton("Import");
	connect(importFileButton, SIGNAL(clicked()), 
        this, SLOT(importFile()));

	QPushButton *newFileButton = new QPushButton("New");
    newFileButton->setEnabled(false);

    QHBoxLayout *openLayout = new QHBoxLayout();
    openLayout->addWidget(m_openFileButton);
    openLayout->addWidget(importFileButton);
    openLayout->addWidget(newFileButton);

	QPushButton *saveFileButton = new QPushButton("Save");
	connect(saveFileButton, SIGNAL(clicked()), 
        this, SLOT(saveFile()));

	QPushButton *saveFileAsButton = new QPushButton("Save As...");
	connect(saveFileAsButton, SIGNAL(clicked()), 
        this, SLOT(saveFileAs()));

    QPushButton *closeFileButton = new QPushButton("Close");
    connect(closeFileButton, SIGNAL(clicked()),
        this, SLOT(closeFile()));

    QHBoxLayout *saveLayout = new QHBoxLayout();
    saveLayout->addWidget(saveFileButton);
    saveLayout->addWidget(saveFileAsButton);
    saveLayout->addWidget(closeFileButton);

    QCheckBox *fileDisplayModeBox = new QCheckBox("Display all fields");
    connect(fileDisplayModeBox, SIGNAL(toggled(bool)),
        this, SLOT(toggleDisplayMode(bool)));

    // Tree view
	m_treeView = new QTreeView();
	m_treeView->setMinimumWidth(320);
	m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->setExpandsOnDoubleClick(false);
    m_treeView->setUniformRowHeights(true);
    m_treeView->setHeaderHidden(true);
    m_treeView->setFrameStyle(QFrame::NoFrame);
    m_treeView->setModel(m_modelManager);
	connect(m_treeView, SIGNAL(expanded(const QModelIndex &)),
				this, SLOT(resizeTreeView()));
	connect(m_treeView, SIGNAL(collapsed(const QModelIndex &)),
				this, SLOT(resizeTreeView()));
	connect(m_treeView, SIGNAL(customContextMenuRequested(const QPoint &)),
				this, SLOT(showTreeViewMenu(const QPoint &)));
    connect(m_treeView, SIGNAL(clicked(const QModelIndex &)),
        this, SLOT(setCurrent(const QModelIndex &)));
    connect(m_treeView, SIGNAL(doubleClicked(const QModelIndex &)),
        this, SLOT(setCurrent(const QModelIndex &)));
    connect(m_treeView, SIGNAL(doubleClicked(const QModelIndex &)),
        m_modelManager, SLOT(handleDoubleClickAtIndex(const QModelIndex &)));

    // File Browser Layout
    QVBoxLayout *fileBrowserLayout = new QVBoxLayout();
    fileBrowserLayout->addLayout(openLayout);
    fileBrowserLayout->addLayout(saveLayout);
    fileBrowserLayout->addWidget(fileDisplayModeBox);
    fileBrowserLayout->addWidget(m_treeView);
    fileBrowserLayout->setContentsMargins(0, 5, 5, 0);

    this->setLayout(fileBrowserLayout);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    this->setLayout(fileBrowserLayout);
    this->setContentsMargins(5, 0, 0, 0);
}

FileBrowser::~FileBrowser()
{
    QSettings settings;
    settings.beginGroup("FileBrowser");
    if(m_recentFileList.count() > 0)
        settings.setValue("RecentFiles", m_recentFileList.join("&"));
    else
        settings.remove("RecentFiles");
    settings.endGroup();

    delete m_recentFileMenu;
}

QString FileBrowser::workingDirectory() const
{
    return m_workingDirectory;
}

void FileBrowser::setWorkingDirectory(const QString &dir)
{
    m_workingDirectory = dir;
}

void FileBrowser::setTreeExpanded(bool expanded)
{
    if(expanded)
        m_treeView->expandAll();
    else
        m_treeView->collapseAll();
}

/********************************** Private *********************************/

void FileBrowser::updateRecentFileMenu()
{
    if(m_recentFileMenu)
    {
        m_recentFileMenu->hide();

        m_recentFileMenu->deleteLater();
    }

    if(m_recentFileList.count() > 0)
    {
        if(m_recentFileList.count() > kRecentFileLimit)
        {
            m_recentFileList.erase(m_recentFileList.begin() + kRecentFileLimit, m_recentFileList.end());
        }

        m_recentFileMenu = new QMenu();
        connect(m_recentFileMenu, SIGNAL(triggered(QAction *)),
            this, SLOT(recentFileSelected(QAction *)));

        for(int index = 0; index < m_recentFileList.count(); ++index)
        {
            QFileInfo info(m_recentFileList.at(index));
            m_recentFileMenu->addAction(info.fileName());
        }
    }
}

/************************************ Slots *********************************/

void FileBrowser::selectFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open an EMD File"), 
		m_workingDirectory, tr("Electron Microscopy Data (*.emd)"));

	if(fileName.isEmpty())
		return;

	qDebug() << "Selected file to open: " << fileName;

    this->openFile(fileName);
}

void FileBrowser::openFile(const QString &path)
{
    this->hideRecentFileMenu();

    if(m_modelManager->fileOpen(path))
    {
        qDebug() << "File is already open: " << path;
        return;
    }

	QFileInfo fileInfo(path);
	m_workingDirectory = fileInfo.path();

	// Attempt to load the image
	Model *emdModel = new Model();
	bool success = emdModel->open(path);

	if(success)
	{
        m_modelManager->addModel(emdModel, true);

		// Update the tree and dimension views
		m_treeView->resizeColumnToContents(0);

        int index = m_recentFileList.indexOf(path);
        if(index >= 0)
        {
            m_recentFileList.removeAt(index);
        }

        m_recentFileList.insert(0, path);

        this->updateRecentFileMenu();

		messageLog->addMessage("Opened file: " + path);
	}
	else
	{
		qWarning() << "File open failed.";
		delete emdModel;
	}
}

void FileBrowser::startRecentFileTimer()
{
    if(!m_recentFileTimer)
    {
        m_recentFileTimer = new QTimer();
        m_recentFileTimer->setSingleShot(true);
        m_recentFileTimer->setInterval(200);
        connect(m_recentFileTimer, SIGNAL(timeout()),
            this, SLOT(showRecentFileMenu()));
    }

    m_recentFileTimer->start();
}

void FileBrowser::cancelRecentFileTimer()
{
    if(!m_recentFileTimer)
        return;

    m_recentFileTimer->stop();
}

void FileBrowser::showRecentFileMenu()
{
    if(!m_recentFileMenu)
        return;

    QPoint pos = m_openFileButton->mapToGlobal(m_openFileButton->rect().bottomLeft());

    m_recentFileMenu->exec(pos);
}

void FileBrowser::hideRecentFileMenu()
{
    if(!m_recentFileMenu)
        return;

    m_recentFileMenu->hide();
}

void FileBrowser::recentFileSelected(QAction *action)
{
    int row = m_recentFileMenu->actions().indexOf(action);;

    QString path = m_recentFileList.value(row);

    this->openFile(path);

    m_openFileButton->clearFocus();
}

void FileBrowser::closeFile()
{
    m_modelManager->removeModel(m_currentIndex);

    m_currentIndex = QModelIndex();
}

void FileBrowser::saveFile()
{
    Model *emdModel = m_modelManager->modelForIndex(m_currentIndex);
    if(!emdModel)
        return;

	QString fileName = emdModel->fileName();

	if(fileName.isEmpty())
	{
		saveFileAs();
		return;
	}

    QString fileExtension = emdModel->fileExtension();
    
    if(fileExtension.toLower().compare("emd") != 0)
        fileExtension = "emd";

    QString filePath = emdModel->fileDir() + fileName + "." + fileExtension;

	qDebug() << "Attempting to save file: " << filePath;	

	emdModel->save(filePath);
}

void FileBrowser::saveFileAs()
{
    Model *emdModel = m_modelManager->modelForIndex(m_currentIndex);
    if(!emdModel)
        return;

	QString fileName = emdModel->fileDir() + emdModel->fileName() + ".emd";
	if(fileName.length() <= 4)
		fileName = m_workingDirectory + "untitled.emd";

	fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                            fileName, tr("Electron Microscopy Data (*.emd)"));


	QFileInfo fileInfo(fileName);
	if(fileInfo.completeBaseName().isEmpty())
		return;

	m_workingDirectory = fileInfo.path();
	
	QFile file(fileName);
	if(file.exists())
		file.remove();

	qDebug() << "Attempting to save file: " << fileName;	

	emdModel->setDirty();
	emdModel->save(fileName);
}

void FileBrowser::importFile()
{
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open a Microscope Data File"), 
		m_workingDirectory, 
        tr("All Data Files (*.dm3 *.ser *.tif *.tiff);;"
		"Digital Micrograph (*.dm3);;"
        "Series File Format (*.ser);;"
        "Tagged Image File Format (*.tiff *.tif);;"));

	if(filePath.isEmpty())
		return;

	qDebug() << "Selected file to open: " << filePath;

    QFileInfo fileInfo(filePath);
	m_workingDirectory = fileInfo.path();

	Model *emdModel = new Model();

    emd::FileManager::Error error = emd::FileManager::openFile(filePath.toUtf8(), emdModel);

	if(error == emd::FileManager::ErrorNone)
	{
        emdModel->setFilePath(filePath);

        // TODO: not the right place for this.
        emdModel->validateDataGroups();

        m_modelManager->addModel(emdModel, true);

		// Update the tree view
		m_treeView->resizeColumnToContents(0);

		messageLog->addMessage("Imported file: " + filePath);

		// No .emd exists, so mark as dirty in case we want to save
		emdModel->setDirty();
	}
	else
	{
		if(emdModel)
			delete emdModel;
	}
}

void FileBrowser::toggleDisplayMode(bool displayAllFields)
{
    if(displayAllFields)
    {
        m_modelManager->setDisplayMode(ModelManager::DisplayModeAll);
    }
    else
    {
        m_modelManager->setDisplayMode(ModelManager::DisplayModeDataGroups);
        m_treeView->expandAll();
    }
}

void FileBrowser::resizeTreeView()
{
	m_treeView->resizeColumnToContents(0);
	int width = m_treeView->columnWidth(0);
	m_treeView->setColumnWidth(0, width + 20);	// TODO: magic number
}

void FileBrowser::showTreeViewMenu(const QPoint &point)
{
    Model *emdModel = m_modelManager->modelForIndex(m_currentIndex);
    if(!emdModel)
        return;

	QModelIndex index = m_treeView->indexAt(point);
	if(!index.isValid())
		return;

    m_currentMenuNode = static_cast<Node *>(index.internalPointer());

	QList<QAction *> actions;

    m_modelManager->getActionsForIndex(index, actions);

	QMenu::exec(actions, m_treeView->mapToGlobal(point));

    qDeleteAll(actions);
}

void FileBrowser::setCurrent(const QModelIndex &current)
{
    m_currentIndex = current;
}

} // namespace emd