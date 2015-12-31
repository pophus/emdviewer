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

#ifndef EMD_FILEBROWSER_H
#define EMD_FILEBROWSER_H

#include <QModelIndex>
#include <qwidget.h>

class QListWidget;
class QListWidgetItem;
class QMenu;
class QPushButton;
class QSettings;
class QTimer;
class QTreeView;

namespace emd
{

class ModelManager;
class Node;

class FileBrowser : public QWidget
{
    Q_OBJECT

public:
    FileBrowser(ModelManager *modelManager, QWidget *parent = 0);
    ~FileBrowser();

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &directory);

    void setTreeExpanded(bool expanded);

private:
    void updateRecentFileMenu();

private slots:
	void selectFile();
    void openFile(const QString &path);
    void startRecentFileTimer();
    void cancelRecentFileTimer();
    void showRecentFileMenu();
    void hideRecentFileMenu();
    void recentFileSelected(QAction *action);
    void closeFile();
	void saveFile();
	void saveFileAs();
	void importFile();
    void toggleDisplayMode(bool);
	void resizeTreeView();
	void showTreeViewMenu(const QPoint &point);
    void setCurrent(const QModelIndex &current);

signals:
	void exportViewSelected();
    void exportImagesSelected();
    void exportDataSelected();

private:
	QString m_workingDirectory;
    QStringList m_recentFileList;

    ModelManager *m_modelManager;
    
    QPushButton *m_openFileButton;
    QMenu *m_recentFileMenu;
    QTreeView *m_treeView;

	Node *m_currentMenuNode;

    QModelIndex m_currentIndex;

    QTimer *m_recentFileTimer;
};

} // namespace emd

#endif