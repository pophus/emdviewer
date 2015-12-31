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

#ifndef EMD_MAINWINDOW_H
#define EMD_MAINWINDOW_H

#include <stdlib.h>

#include <QtGui>
#include <QtWidgets>

#include "Model.h"

#include "ModuleSource.h"
#include "WorkflowSource.h"

#include "MessageModel.h"
#include "ModelManager.h"

namespace emd
{

class CentralWidget;
class Dataset;
class ExportOperation;
class FileBrowser;
class FileExporter;
class Frame;
class LogView;
class Plugin;
class PreferencesDialog;
class ScaleWidget;
class StatusLabel;
class WorkflowBrowser;
class WorkflowModule;

class MainWindow : public QMainWindow,
                   public ModuleSource
{
	Q_OBJECT
	
public:
	MainWindow();
	//~MainWindow();

    void declareModules() override;
    WorkflowModule *createModule(const std::string &group, 
                                 const std::string &name) override;

private:
	// Setup functions
	void setUpUI();

	// File functions
	void saveEmd(const QString &fileName);
	void loadPlugins();
    void loadBuiltinPlugins();
	void addPlugin(Plugin *plugin);

    void handleFeatures(Workflow *workflow);

    CentralWidget *widgetForDataGroup(DataGroup *dataGroup, int *tabIndex = nullptr);

    void updateWorkflowControl();

	// QMainWindow functions
    void resizeEvent(QResizeEvent *e);
	void closeEvent(QCloseEvent * e);

private slots:
    // Status bar
    void startMessageLogTimer(bool start);
    void expandMessageLog();
    void setMessageLogExpanded(bool expanded);
    void displayTempMessage(const QString &msg);

	// Docks
	void changeWorkflowControlVisibility(bool);
	void changeFileBrowserVisibility(bool);
	void changeDockVisibility(QAction *);

	// Other controls
	void reverseDataOrder(bool);

    // Central Widget
    void centralTabChanged(int index);
    void centralTabClosed(int index);

	// File Browser
    void setFileManagerTab(int index);
    void startExport(ExportOperation *);
    void finishExport();
    void cancelExport();

	// Workflow
    void displayDataGroup(DataGroup *dataGroup);
    void closeDataGroup(DataGroup *dataGroup);
	void changeWorkflow(Workflow *);
    void saveCurrentWorkflow(const QString &path);

	// Other
	void displayAbout();

	// Debug
    void printCurrentWorkflow() const;
	void printWorkflow(Workflow *workflow) const;
    void printModels() const;

private:
	QSettings m_settings;

	// Window menu
	QMenu *m_windowMenu;
	QAction *m_workflowControlAction;
	QAction *m_fileManagerAction;

    // Preferences
    PreferencesDialog *m_preferencesDialog;

	// Debug menu
	QMenu *m_debugMenu;

	// Central widget
    QTabWidget *m_centralTabWidget;
    CentralWidget *m_currentCentralWidget;

	// File browser dock
	QDockWidget *m_fileBrowserDock;
    QTabWidget *m_fileTabWidget;
    FileBrowser *m_fileBrowser;
    FileExporter *m_fileExporter;

	// Workflow dock
    QVBoxLayout *m_workflowControlLayout;
    QWidget *m_workflowControlWidget;
    QTabWidget *m_workflowTabWidget;
	QDockWidget *m_workflowControlDock;
    WorkflowBrowser *m_workflowBrowser;

	// Status bar
    QDockWidget *m_statusBarDock;
	QProgressBar *m_progressBar;
    StatusLabel *m_statusSpacerWidget;
    QTimer *m_messageLogTimer;
	LogView *m_messageView;
    QLabel *m_tempStatusLabel;

	// Data
    ModelManager m_modelManager;
	QList<Plugin*> m_plugins;
};

} // namespace emd


#endif
