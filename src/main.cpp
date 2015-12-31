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

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QTime>
 
#include "ColourMap.h"
#include "MainWindow.h"
#include "MessageModel.h"
#include "StyleConfig.h"

#define PREFIX "[emdViewerDebug]" << QTime::currentTime().toString("hh:mm:ss.zzz")

static const QString kProgramName = "emdViewer";
static const QString kCompanyName = "MesoBine";

QFile *logFile;
QTextStream *logStream;
emd::MessageModel *messageLog;
bool logFileOpened;

void setUpLog()
{
	logFile = new QFile(QDir::currentPath() + "//log.txt");

	if(logFile->exists())
		logFile->remove();
	logFileOpened = logFile->open(QIODevice::ReadWrite | QIODevice::Text);

	logStream = new QTextStream();
	logStream->setDevice(logFile);

	messageLog = new emd::MessageModel();
}

void LogHandler(QtMsgType type, const QMessageLogContext &, const QString &msg) 
{
	switch (type) 
	{
	case QtDebugMsg:
		if(logFileOpened)
			*logStream << PREFIX << " Debug: " << msg << "\n";
		messageLog->addMessage(msg, emd::MessageTypeDebug);
		break;
	case QtCriticalMsg:
		if(logFileOpened)
			*logStream << PREFIX << " Critical: " << msg << "\n";
        messageLog->addMessage(msg, emd::MessageTypeError);
		break;
	case QtWarningMsg:
		if(logFileOpened)
			*logStream << PREFIX << " Warning: " << msg << "\n";
        messageLog->addMessage(msg, emd::MessageTypeWarning);
		break;
	case QtFatalMsg:
		if(logFileOpened)
			*logStream << PREFIX <<  " Fatal: " << msg << "\n";
		abort();
    }
	if(logFileOpened)
		logStream->flush();
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(kCompanyName);
    QCoreApplication::setApplicationName(kProgramName);

	setUpLog();
	qInstallMessageHandler(&LogHandler);
    QApplication app(argc, argv);
    
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QFont newFont("Garamond", 8, QFont::Normal, false);
    QApplication::setFont(newFont);
	 
    emd::MainWindow mainWindow;
    mainWindow.setWindowTitle(kProgramName);
    mainWindow.setWindowIcon(QIcon(":/icons/window_icon.png"));
	app.setStyleSheet(
        QString(
		"QDockWidget::title {"
        "   text-align: center;"
        "}"
		"QDockWidget {"
        "   titlebar-close-icon: url(:/icons/close.png);"
        "   titlebar-normal-icon: url(:/icons/popout.png);"
        "}"
        "QDockWidget::close-button, QDockWidget::float-button {"
        //"   icon-size: 64px;"
        //"   width: 32px; height: 32px;"
        //"   padding: 0px"
        "}"
		"QTableView::item { border: 0pt; padding: 0pt; margin: 0pt }"
		"QPushButton:checked {"
		"	border: 1pt solid #000044;"
		"	border-radius: 3px;"
		//"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
  //      "	    stop: 0 " PRIMARY_HIGHLIGHT ","
  //      "	    stop: 0.1 " PRIMARY_COLOUR ","
  //      "	    stop: 0.8 " PRIMARY_COLOUR ","
  //      "       stop: 1 " PRIMARY_SHADOW ");"
        "   background-color: " BTN_BG_CHK_CLR ";"
        "   color: white;"
        //"   min-width: 80pt;"
        "   padding: 3pt;"
		" }"
		"QTreeView::item:!selected:hover {"
		"	background: " BTN_BG_UNCHK_CLR ";"
		"}"
        //"QGraphicsView { border-style: none; margin: 0px; padding: 0px; }"
						//"QGroupBox {"
						//"	padding: 0px;"
						//"}"
		//				//"QToolBar {border: 2px solid red}"
		//				"QStatusBar {background-color: rgb(220, 240, 220)}"
		//				"Scenewindow {background-color: green}"
		//				"QPushButton {background-color: rgb(200, 230, 200)}"
		//				"QStackedWidget {background-color: rgb(200, 240, 200)}"
		//				"QComboBox {background-color: rgb(200, 230, 200)}"
		//				//"QWidget {background-color: black}"
		//				//"* {border-color: red}"
//        "QTabWidget::pane {"
//        "   border-top: 5pt solid #000000;"
//        "}"

        "QTabWidget::tab-bar {"
//        "    left: 10px;"
        "    top: 0px;"
        "    alignment: bottom"
        "}"
            
        "QTabWidget {"
        "    margin-top: 20px;"
        "    background: white;"
        "}"

        "QTabBar::tab {"
        "   background:" BTN_BG_UNCHK_CLR ";"
            "color: black;"
//        "   border: 2px solid #C4C4C3;"
//        "   border-bottom-color: #C2C7CB;"
        "   border-top-left-radius: 2px;"
        "   border-top-right-radius: 2px;"
        "   min-width: 32ex;"
            "min-height: 2.5ex;"
        "   padding-left: 5px;"
        "   padding-right: 5px;"
        "   padding-top: 2px;"
        "   padding-bottom: 2px;"
        "   border: 1px solid;"
        "   border-color: #9B9B9B;"
        "}"
            
        "QTabBar::tab:!selected:hover {"
        "    background: " BTN_BG_UNCHK_HVR_CLR ";"
        "}"
            
        "QTabBar::tab:!selected {"
        "   margin-bottom: 1px;"
        "}"

        "QTabBar::tab:selected {"
        "    background:" BTN_BG_CHK_CLR ";"
        //"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
        //"       stop: 0 " BTN_BG_CHK_CLR ","
        //"       stop: 0.7 " BTN_BG_CHK_CLR ","
        //"       stop: 1 white);"
            "color: white;"
        "   border-bottom-color: " BTN_BG_CHK_CLR ";"
        "   margin-bottom: 1px;"
        "}"

        "QTabBar::tab:disabled {"
            "background:" BTN_BG_DSBLD_CLR ";"
            "color:" BTN_TXT_DSBLD_CLR ";"
        "}"

        "emd--FileBrowser {"
        "    background: white;"
        "}"
            
        //"QPushButton:!selected:hover {"
        //"	background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
        //"       stop: 0 " BTN_BG_UNCHK_HVR_CLR ","
        //"       stop: 1 white);"
        //"}"
            
        //"QTabBar::tab:selected:hover {"
        //"    background: " BTN_BG_CHK_HVR_CLR ";"
        //"}"
        "QGroupBox {"
        "    padding: 0ex;"
        "    border-top: 1px solid "LIGHT_GRAY ";" 
        "    margin-top: 3ex;"
        "}"

        //"QGroupBox::title {"
        //"    border: 4px solid red;"
        //"}"

        "QGraphicsView {"
        "    border: 0px solid red;"
        "}"

        "emd--MainGraphicsImageWidget {"
        "    border: 3px solid blue;"
        "}"

        "emd--CentralWidget {"
        "    border: 3px solid green;"
        "}"
        )
    );
	mainWindow.show();
	qDebug() << "Starting app...";

    int ret = app.exec();

	//delete messageLog;
	//delete logFile;
	//delete logStream;

	return ret;
 }