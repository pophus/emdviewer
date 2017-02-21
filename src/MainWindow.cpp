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

#include "MainWindow.h"

#include "CentralWidget.h"
#include "ColourManager.h"
#include "ComplexModule.h"
#include "DataGroupModule.h"
#include "DimensionsPane.h"
#include "Dataset.h"
#include "ExportOperation.h"
#include "FileBrowser.h"
#include "FileExporter.h"
#include "FileManager.h"
#include "Frame.h"
#include "HistogramModule.h"
#include "ImageWindowModule.h"
#include "LogView.h"
#include "MainGraphicsImageWidget.h"
#include "Model.h"
#include "Plugin.h"
#include "PreferencesDialog.h"
#include "StatusLabel.h"
#include "Workflow.h"
#include "WorkflowBrowser.h"

#ifdef EMD_VIEWER_PRO
#include "AberrationCorrectionPlugin.h"
#include "ExitWaveReconstructionPlugin.h"
#include "FourierTransformPlugin.h"
#endif

extern emd::MessageModel *messageLog;

namespace emd
{

static const QString kProgramVersion = "Beta 0.4";

static const QSize defaultWindowSize(1200, 600);

MainWindow::MainWindow() : 
    QMainWindow(),
    m_workflowControlWidget(nullptr),
    m_messageLogTimer(NULL)
{
    this->declareModules();

	loadPlugins();

	// Set up the UI
	setUpUI();

    connect(&m_modelManager, SIGNAL(dataGroupLoaded(DataGroup *)),
        this, SLOT(displayDataGroup(DataGroup *)));
    connect(&m_modelManager, SIGNAL(dataGroupUnloaded(DataGroup *)),
        this, SLOT(closeDataGroup(DataGroup *)));

	/************ General setup ************/
	setMinimumSize(defaultWindowSize);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setMargin(0);

	// Restore window settings
    QSettings settings;

	settings.beginGroup("MainWindow");
	
	if(settings.value("Maximized", QVariant(false)).toBool())
		showMaximized();
	else
		resize(settings.value("Size", defaultWindowSize).toSize());
	this->move(settings.value("Position").toPoint());

    this->restoreState(settings.value("WindowState").toByteArray());

	QString workingDirectory = settings.value("WorkingDirectory").toString();
	if(workingDirectory.isEmpty())
		workingDirectory = QDir::currentPath();
	else
	{
		QFileInfo fileInfo(workingDirectory);
		if(!fileInfo.isDir())
			workingDirectory = QDir::currentPath();
	}

    m_fileBrowser->setWorkingDirectory(workingDirectory);

	settings.endGroup();

    this->setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
    this->setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
}

//MainWindow::~MainWindow()
//{
//	
//}

void MainWindow::declareModules()
{
    WorkflowModule::declare("Core", "BinaryOutput", this,
        "Converts data from one type to another.");

    WorkflowModule::declare("Core", "Complex", this,
        "Converts complex data to real data.");
    
    WorkflowModule::declare("Core", "DataGroup", this,
        "Retrieves input data from a data group.");
    
    WorkflowModule::declare("Core", "Histogram", this,
        "Displays a histogram plot of the data.");
    
    WorkflowModule::declare("Core", "ImageWindow", this,
        "");
}

WorkflowModule *MainWindow::createModule(const std::string &group, 
                                         const std::string &name)
{
    if(group.compare("Core") == 0)
    {
        if(name.compare("BinaryOutput") == 0)
        {
            return new BinaryOutputModule();
        }
        else if(name.compare("Complex") == 0)
        {
            return new ComplexModule();
        }
        else if(name.compare("DataGroup") == 0)
        {
            return new DataGroupModule(nullptr);
        }
        else if(name.compare("Histogram") == 0)
        {
            return new HistogramModule();
        }
        else if(name.compare("ImageWindow") == 0)
        {
            return new ImageWindowModule();
        }
    }

    return nullptr;
}

/******************************** Setup ***********************************/

void MainWindow::setUpUI()
{
	/********************/
	/*** File toolbar ***/
	/********************/
	QToolBar *fileToolBar = addToolBar(tr("File"));

	// Window menu
	//fileToolBar->addSeparator();
	m_windowMenu = new QMenu("Window");
	connect(m_windowMenu, SIGNAL(triggered(QAction*)),
		this, SLOT(changeDockVisibility(QAction*)));

	m_fileManagerAction = m_windowMenu->addAction("File Manager");
	m_fileManagerAction->setCheckable(true);

	m_workflowControlAction = m_windowMenu->addAction("Workflow Manager");
	m_workflowControlAction->setCheckable(true);

	QToolButton* windowButton = new QToolButton();
	windowButton->setText("Window ");
	windowButton->setMenu(m_windowMenu);
	windowButton->setPopupMode(QToolButton::InstantPopup);
	fileToolBar->addWidget(windowButton);

    // Preferences menu
    m_preferencesDialog = new PreferencesDialog(this);
    fileToolBar->addAction("Preferences", m_preferencesDialog, SLOT(exec()));

	// About 
	fileToolBar->addSeparator();
	QAction *aboutAction = new QAction("About", fileToolBar);
	fileToolBar->addAction(aboutAction);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(displayAbout()));

	// Debug menu
#ifdef DEBUG
	fileToolBar->addSeparator();
	m_debugMenu = new QMenu("Debug");

	QAction *printWorkflowAction = m_debugMenu->addAction("Print Workflow");
	connect(printWorkflowAction, SIGNAL(triggered()),
		this, SLOT(printCurrentWorkflow()));

    QAction *printModelsAction = m_debugMenu->addAction("Print Models");
    connect(printModelsAction, SIGNAL(triggered()),
        this, SLOT(printModels()));

	QToolButton* debugButton = new QToolButton();
	debugButton->setText("Debug ");
	debugButton->setMenu(m_debugMenu);
	debugButton->setPopupMode(QToolButton::InstantPopup);
	fileToolBar->addWidget(debugButton);
#endif


	/**********************/
	/*** Central Widget ***/
	/**********************/

    m_centralTabWidget = new QTabWidget();
    m_centralTabWidget->setElideMode(Qt::ElideMiddle);
    m_centralTabWidget->setTabsClosable(true);
    m_centralTabWidget->setMovable(true);
	setCentralWidget(m_centralTabWidget);
    connect(m_centralTabWidget, SIGNAL(currentChanged(int)),
        this, SLOT(centralTabChanged(int)));
    connect(m_centralTabWidget, SIGNAL(tabCloseRequested(int)),
        this, SLOT(centralTabClosed(int)));


	/*************************/
	/*** File Browser Dock ***/
	/*************************/

    // File Browser Widget
    m_fileBrowser = new FileBrowser(&m_modelManager);

    // File Exporter
    m_fileExporter = new FileExporter();
    m_fileExporter->setEnabled(false);
    connect(m_fileExporter, SIGNAL(exportStarted(ExportOperation *)),
        this, SLOT(startExport(ExportOperation *)));
    connect(m_fileExporter, SIGNAL(exportCancelled()),
        this, SLOT(cancelExport()));

    // Tab Widget
    m_fileTabWidget = new QTabWidget();
    m_fileTabWidget->addTab(m_fileBrowser, "Files");
    m_fileTabWidget->addTab(m_fileExporter, "Export");
    m_fileTabWidget->setTabEnabled(1, false);
    connect(m_fileTabWidget, SIGNAL(currentChanged(int)),
        this, SLOT(setFileManagerTab(int)));

	// Dock
	m_fileBrowserDock = new QDockWidget("File Manager", this);
	m_fileBrowserDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
	m_fileBrowserDock->setWidget(m_fileTabWidget);
	this->addDockWidget(Qt::LeftDockWidgetArea, m_fileBrowserDock);
	connect(m_fileBrowserDock, SIGNAL(visibilityChanged(bool)),
		this, SLOT(changeFileBrowserVisibility(bool)));
	m_fileBrowserDock->show();
	
	
	/*****************************/
	/*** Workflow Control Dock ***/
	/*****************************/

    QWidget *workflowContainer = new QWidget();
    //m_workflowControlContainer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    m_workflowControlLayout = new QVBoxLayout();
    m_workflowControlLayout->setContentsMargins(0, 0, 0, 0);
    workflowContainer->setLayout(m_workflowControlLayout);

    m_workflowBrowser = new WorkflowBrowser();
    connect(m_workflowBrowser, SIGNAL(workflowSelected(Workflow *)),
		this, SLOT(changeWorkflow(Workflow *)));
    connect(m_workflowBrowser, SIGNAL(saveRequested(const QString &)),
		this, SLOT(saveCurrentWorkflow(const QString &)));

    m_workflowTabWidget = new QTabWidget();
    m_workflowTabWidget->addTab(workflowContainer, "Control");
    m_workflowTabWidget->addTab(m_workflowBrowser, "Workflows");

	// Dock
	m_workflowControlDock = new QDockWidget("Workflow Manager", this);
	m_workflowControlDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
	m_workflowControlDock->setWidget(m_workflowTabWidget);
	this->addDockWidget(Qt::RightDockWidgetArea, m_workflowControlDock);
	connect(m_workflowControlDock, SIGNAL(visibilityChanged(bool)),
		this, SLOT(changeWorkflowControlVisibility(bool)));
	m_workflowControlDock->show();


	/******************/
	/*** Status Bar ***/
	/******************/

    m_statusSpacerWidget = new StatusLabel();
    m_statusSpacerWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    connect(m_statusSpacerWidget, SIGNAL(hoverChanged(bool)),
        this, SLOT(startMessageLogTimer(bool)));

    m_tempStatusLabel = new QLabel();
    m_tempStatusLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

	m_progressBar = new QProgressBar();
	m_progressBar->setTextVisible(false);
    m_progressBar->setMinimumWidth(300);
    m_progressBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    
    QHBoxLayout *statusBarLayout = new QHBoxLayout();
    statusBarLayout->addWidget(m_statusSpacerWidget, 4);
    statusBarLayout->addWidget(m_tempStatusLabel, 3, Qt::AlignLeft);
    statusBarLayout->addWidget(m_progressBar, 2, Qt::AlignRight);
    statusBarLayout->setContentsMargins(0, 0, 10, 0);

    QWidget *statusInfoWidget = new QWidget();
    statusInfoWidget->setLayout(statusBarLayout);
    statusInfoWidget->setContentsMargins(0, 0, 0, 0);
    QFontMetrics metrics(this->font());
    statusInfoWidget->setMaximumHeight(2 * metrics.height());

    // Message log pane
	m_messageView = new LogView(this);
	//m_messageView->setWrapping(true);
	m_messageView->setModel(messageLog);
    m_messageView->setFrameStyle(QFrame::NoFrame);
    m_messageView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_messageView->raise();

    // To set the background colour.
    m_messageView->viewport()->setBackgroundRole(QPalette::Window);

    connect(m_messageView, SIGNAL(hoverChanged(bool)),
        this, SLOT(setMessageLogExpanded(bool)));

    connect(messageLog, SIGNAL(messageLogged()),
        m_messageView, SLOT(scrollToBottom()));

    m_statusBarDock = new QDockWidget();
    QWidget *dummyTitleBar = new QWidget();
    m_statusBarDock->setTitleBarWidget(dummyTitleBar);
    m_statusBarDock->setWidget(statusInfoWidget);
    m_statusBarDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    this->addDockWidget(Qt::BottomDockWidgetArea, m_statusBarDock);

    setMessageLogExpanded(false);


	/***************************/
	/*** Main Window Options ***/
	/***************************/

	this->setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks);
}

void MainWindow::loadPlugins()
{
	// Search the plugin dir and attempt to load whatever we find.
	QDir pluginsDir = QDir(qApp->applicationDirPath());
    bool success = pluginsDir.cd("plugins");
    if(!success)
    {
        qDebug() << "Failed to locate plugin directory.";
        loadBuiltinPlugins();
        return;
    }

	QStringList filter;
	filter << "*.dll";
	pluginsDir.setNameFilters(filter);

	QStringList files = pluginsDir.entryList(QDir::Files);

    QString fileName;

    for(int index = 0; index < files.count(); ++index) 
	{
        fileName = files.at(index);
        qDebug() << "Attempting to load plugin: " << fileName;
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if(plugin)
		{
			Plugin *emdPlugin = qobject_cast<Plugin*>(plugin);
            if(emdPlugin)
			{
				qDebug() << "Load successful.";

				addPlugin(emdPlugin);
			}
			else
			{
				qWarning() << "Could not create Plugin";
			}
        }
        else 
		{
			qDebug() << "Load failed.";
        }
    }
}

void MainWindow::loadBuiltinPlugins()
{
#ifdef EMD_VIEWER_PRO
	// Fourier transform
	Plugin *ftPlugin = new FourierTransformPlugin();
    addPlugin(ftPlugin);

	// Aberration correction
	Plugin *acPlugin = new AberrationCorrectionPlugin();
    addPlugin(acPlugin);

    // EWR
    Plugin *ewrPlugin = new ExitWaveReconstructionPlugin();
    addPlugin(ewrPlugin);
#endif
}

void MainWindow::addPlugin(Plugin *plugin)
{
	if(!plugin)
		return;
	
    m_plugins.append(plugin);

    plugin->declareModules();
}

/************************************** Slots ********************************************/

void MainWindow::startMessageLogTimer(bool start)
{
    if(start)
    {
        m_messageLogTimer = new QTimer(this);
        m_messageLogTimer->setSingleShot(true);
        connect(m_messageLogTimer, SIGNAL(timeout()),
            this, SLOT(expandMessageLog()));

        // Open the message log after 500 ms.
        m_messageLogTimer->start(500);
    }
    else
    {
        if(m_messageLogTimer)
        {
            m_messageLogTimer->stop();
            delete m_messageLogTimer;
            m_messageLogTimer = NULL;
        }
    }
}

void MainWindow::expandMessageLog()
{
    setMessageLogExpanded(true);
}

void MainWindow::setMessageLogExpanded(bool expanded)
{
    QFontMetrics metrics(this->font());
    int minHeight = (int) (1.4f * metrics.height());
    int offset = (m_statusBarDock->height() - minHeight) / 2;

    if(expanded)
    {
        m_messageView->setFixedHeight(this->height() / 4);
        m_messageView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        //m_messageView->viewport()->setAutoFillBackground(true);
    }
    else
    {
        m_messageView->setFixedHeight(minHeight);
        m_messageView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        //m_messageView->viewport()->setAutoFillBackground(false);

        this->startMessageLogTimer(false);
    }

    m_messageView->move(10, this->height() - m_messageView->height() - offset);
    m_messageView->scrollToBottom();
}

void MainWindow::displayTempMessage(const QString &msg)
{
    m_tempStatusLabel->setText(msg);

    m_tempStatusLabel->update();
}

void MainWindow::changeWorkflowControlVisibility(bool visible)
{
	m_workflowControlAction->setChecked(visible);
}

void MainWindow::changeFileBrowserVisibility(bool visible)
{
	m_fileManagerAction->setChecked(visible);
}

void MainWindow::changeDockVisibility(QAction *action)
{
	QDockWidget *dock = NULL;

	if(action == m_workflowControlAction)
		dock = m_workflowControlDock;
	else if(action == m_fileManagerAction)
		dock = m_fileBrowserDock;

	if(dock)
	{
		if(dock->isVisible())
			dock->hide();
		else
			dock->show();
	}
}

void MainWindow::reverseDataOrder(bool)
{
 //   Model *emdModel = m_modelManager.currentModel();
 //   if(!emdModel)
 //       return;

	//emdModel->setDataOrder(!(emdModel->dataOrder()));
}

void MainWindow::centralTabChanged(int index)
{
    if(index < 0)
    {
        m_currentCentralWidget = nullptr;

        // TODO: magic numbers
        m_fileTabWidget->setTabEnabled(1, false);
    }
    else
    {
        m_currentCentralWidget = static_cast<CentralWidget*>(m_centralTabWidget->widget(index));
        m_workflowBrowser->setCurrentWorkflowName(m_currentCentralWidget->workflow()->name());
        
        m_fileTabWidget->setTabEnabled(1, true);

        DataGroup *dataGroup = m_currentCentralWidget->dataGroup();
        m_fileExporter->setFileNameStem(dataGroup->model()->fileName() % "-" % dataGroup->name());
    }

    updateWorkflowControl();
}

void MainWindow::centralTabClosed(int index)
{
    if(index < 0)
        return;

    CentralWidget *centralWidget = static_cast<CentralWidget*>(m_centralTabWidget->widget(index));

    m_modelManager.closeDataGroup(centralWidget->dataGroup());
}

/******************************** Display *************************************/

void MainWindow::displayDataGroup(DataGroup *dataGroup)
{    
    if(!dataGroup)
        return;

    CentralWidget *centralWidget = new CentralWidget();

    centralWidget->reset(dataGroup);

    m_currentCentralWidget = centralWidget;
    
    Workflow *workflow = Workflow::create("Core", "Basic");
    
    this->changeWorkflow(workflow);

    m_centralTabWidget->addTab(centralWidget, dataGroup->model()->fileName() % " - " % dataGroup->name());

    m_centralTabWidget->setCurrentWidget(centralWidget);

    //if(!m_fileExporter->isEnabled())
    //{
    //    m_fileExporter->setEnabled(true);
    //    m_fileExporter->setFileNameStem(dataGroup->model()->fileName() % "-" % dataGroup->name());
    //}

    if(m_modelManager.displayMode() == ModelManager::DisplayModeDataGroups)
        m_fileBrowser->setTreeExpanded(true);
}

void MainWindow::closeDataGroup(DataGroup *dataGroup)
{
    if(!dataGroup)
        return;

    int tabIndex;
    CentralWidget *centralWidget = this->widgetForDataGroup(dataGroup, &tabIndex);

    if(centralWidget)
    {
        m_centralTabWidget->removeTab(tabIndex);
        delete centralWidget;
    }
}

/******************************* File Operations *****************************/

void MainWindow::setFileManagerTab(int index)
{
    m_fileTabWidget->setCurrentIndex(index);

    if(index == 0)     // File Browser
    {
        
    }
    
    if(index == 1)     // Export
    {
        if(m_fileExporter->exportDirectory().size() == 0)
            m_fileExporter->setExportDirectory(m_fileBrowser->workingDirectory());

        m_fileExporter->setSelection(m_currentCentralWidget->selection());

        m_fileExporter->setFileNameStem(m_currentCentralWidget->dataGroup()->model()->fileName() % "-" % m_currentCentralWidget->dataGroup()->name());

        connect(m_currentCentralWidget, SIGNAL(selectionChanged(const FrameSet::Selection &)),
            m_fileExporter, SLOT(setSelection(const FrameSet::Selection &)));

        m_currentCentralWidget->setSelectionMode(true);
    }
    else
    {
        disconnect(m_currentCentralWidget, SIGNAL(selectionChanged(const FrameSet::Selection &)),
            m_fileExporter, SLOT(setSelection(const FrameSet::Selection &)));

        m_currentCentralWidget->setSelectionMode(false);
    }
}

void MainWindow::startExport(ExportOperation *exportOperation)
{
    if(!m_currentCentralWidget)
        return;
    
    m_centralTabWidget->setEnabled(false);
    m_fileBrowser->setEnabled(false);
    
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(exportOperation->itemCount());

    connect(m_currentCentralWidget, SIGNAL(exportFinished()),
        this, SLOT(finishExport()));

    m_currentCentralWidget->performExport(exportOperation);
}

void MainWindow::finishExport()
{ 
    m_centralTabWidget->setEnabled(true);
    m_fileBrowser->setEnabled(true);

    m_progressBar->reset();

    m_fileExporter->finishExport();

    this->displayTempMessage("");
}

void MainWindow::cancelExport()
{
    if(!m_currentCentralWidget)
        return;

    m_currentCentralWidget->cancelExport();
}

void MainWindow::changeWorkflow(Workflow *workflow)
{
    if(!workflow || !m_currentCentralWidget)
        return;

    m_currentCentralWidget->setWorkflow(workflow);

    m_workflowTabWidget->setCurrentIndex(0);

    this->handleFeatures(workflow);

    workflow->start();
}

void MainWindow::saveCurrentWorkflow(const QString &path)
{
    if(!m_currentCentralWidget)
        return;

    bool success = m_currentCentralWidget->workflow()->save(path);

    if(!success)
    {
        qWarning() << "Failed to save workflow: " << path;
    }
}

void MainWindow::handleFeatures(Workflow *workflow)
{
    //if(!m_modelManager.currentModel())
    //    return;

    //if(!workflow)
    //{
    //    return;
    //}

    //WorkflowModule::RequiredFeatures features = workflow->requiredFeatures();

    //if(features & WorkflowModule::RangeSelectionFeature)
    //{
    //    m_centralWidget->dimensionsPane()->setSelectionModeEnabled(true);
    //}
    //else
    //{
    //    m_centralWidget->dimensionsPane()->setSelectionModeEnabled(false);
    //}
}

CentralWidget *MainWindow::widgetForDataGroup(DataGroup *dataGroup, int *tabIndex)
{
    int tabCount = m_centralTabWidget->tabBar()->count();

    for(int index = 0; index < tabCount; ++index)
    {
        CentralWidget *widget = static_cast<CentralWidget*>(m_centralTabWidget->widget(index));
        if(widget && widget->dataGroup() == dataGroup)
        {
            if(tabIndex)
                *tabIndex = index;
            return widget;
        }
    }

    return nullptr;
}

void MainWindow::updateWorkflowControl()
{
    if(m_workflowControlWidget)
    {
        m_workflowControlWidget->setVisible(false);
        m_workflowControlLayout->removeWidget(m_workflowControlWidget);
    }

    if(m_currentCentralWidget)
    {
        m_workflowControlWidget = m_currentCentralWidget->workflowControlWidget();
        m_workflowControlLayout->addWidget(m_workflowControlWidget);
        m_workflowControlWidget->setVisible(true);
    }
}

void MainWindow::displayAbout()
{
	QDialog* aboutDialog = new QDialog(this, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	aboutDialog->setWindowTitle("About emdViewer");
	QGridLayout* aboutLayout = new QGridLayout;

	QLabel* icon = new QLabel(aboutDialog);
	icon->setPixmap(QPixmap(":/images/debug.png"));
	QLabel* version = new QLabel(kProgramVersion, aboutDialog);
	//QLabel* by = new QLabel("by", aboutDialog);
	QLabel* authors = new QLabel("� 2012 Phil Ophus / Mesozoic Binaries", aboutDialog);
	QLabel* contact = new QLabel("Contact: Colin at <a href=\"mailto:mesozoicbinaries@gmail.com\">clophus@lbl.gov</a>");
	contact->setTextInteractionFlags(Qt::TextBrowserInteraction );
	contact->setOpenExternalLinks(true);
	QLabel* qtRef = new QLabel("Developed using the Qt Library.", aboutDialog);
	//QLabel* kissRef = new QLabel("Fourier transforms are performed using <a href=\"http://sourceforge.net/projects/kissfft/\">kissFFT</a> by Mark Borgerding.", aboutDialog);
	//kissRef->setTextInteractionFlags(Qt::TextBrowserInteraction );
	//kissRef->setOpenExternalLinks(true);
	QPushButton* qtAbout = new QPushButton("About Qt...", aboutDialog);
	connect(qtAbout, SIGNAL(clicked()), QCoreApplication::instance(), SLOT(aboutQt()));
	
	aboutLayout->addWidget(icon, 0, 0, 1, -1, Qt::AlignCenter);
	aboutLayout->addWidget(version, 1, 0, 1, -1, Qt::AlignCenter);
	aboutLayout->addWidget(authors, 2, 0, 1, -1, Qt::AlignCenter);
	aboutLayout->addWidget(contact, 3, 0, 1, -1, Qt::AlignCenter);
	aboutLayout->addWidget(qtRef, 4, 0, 1, 1, Qt::AlignLeft);
	aboutLayout->addWidget(qtAbout, 4, 1, 1, 1, Qt::AlignCenter);
	//aboutLayout->addWidget(kissRef, 5, 0, 1, 2, Qt::AlignLeft);

	aboutDialog->setLayout(aboutLayout);
	
	aboutDialog->exec();
}

// Debug

void MainWindow::printCurrentWorkflow() const
{
    if(!m_currentCentralWidget)
        return;

    printWorkflow(m_currentCentralWidget->workflow());
}

void MainWindow::printWorkflow(Workflow *workflow) const
{
	qDebug() << "Printing workflow...";

	if(!workflow)
	{
		qDebug() << "No workflow.";
		return;
	}

	const QList<WorkflowModule *> &modulesToPrint = workflow->modules();

    if(modulesToPrint.size() == 0)
    {
        qDebug() << "Workflow has no modules.";
        return;
    }

	for(int index = 0; index < modulesToPrint.count(); ++index)
	{
        WorkflowModule *module = modulesToPrint.at(index);
		QString eString = module->enabled() ? "enabled" : "disabled";
		QString oString = module->outdated() ? "outdated" : "up-to-date";
		qDebug() << "  " << module->name() << "(" << eString << ", " << oString << ")";
	}
}

void MainWindow::printModels() const
{
    m_modelManager.print();
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    m_messageView->setFixedWidth(m_statusSpacerWidget->width());

    this->setMessageLogExpanded(false);
}

void MainWindow::closeEvent(QCloseEvent * /*e*/)
{
	// Save settings
    QSettings settings;

	settings.beginGroup("MainWindow");

	if(this->isMaximized())
	{
		settings.setValue("Maximized", true);
		settings.setValue("Size", defaultWindowSize);
	}
	else
	{
		settings.setValue("Maximized", false);
		settings.setValue("Size", this->size());
	}
	settings.setValue("Position", this->pos());

    settings.setValue("WindowState", this->saveState());

	settings.setValue("WorkingDirectory", m_fileBrowser->workingDirectory());
    
	settings.endGroup();
}

} // namespace emd
