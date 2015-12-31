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

#ifndef EMD_MODELMANAGER_H
#define EMD_MODELMANAGER_H

#include <QAbstractItemModel>

class QAction;

namespace emd
{

class DataGroup;
class Model;
class Node;

class ModelManager : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum DisplayMode {
        DisplayModeAll,
        DisplayModeDataGroups
    };

    ModelManager();
    ~ModelManager();

    void addModel(Model *model, bool setActive = false);
    void removeModel(Model *model);
    void removeModel(const QModelIndex &index);

    void displayDataGroup(Model *model, int dataGroupIndex = -1);
    void closeModel(Model *model);
    void closeModel(const QModelIndex &index);
    void closeDataGroup(DataGroup *dataGroup);
    
    Model *modelForIndex(const QModelIndex &index) const;

    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode mode);

    void getActionsForIndex(const QModelIndex &index, QList<QAction*> &actions);

    bool fileOpen(const QString &path);

    void print() const;

    // QAbstractItemModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	QModelIndex index(int row, int column,
					const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;

	Qt::ItemFlags flags(const QModelIndex &index) const;
    
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value,
				int role = Qt::EditRole);

public slots:
    void handleDoubleClickAtIndex(const QModelIndex &);

private slots:
    void handleActivateAction();
    void handleAddAttribute();
    void handleCloseAction();
    void handleDisplayAction();

signals:
    void dataGroupLoaded(DataGroup *dataGroup);
    void dataGroupUnloaded(DataGroup *dataGroup);

private:
    QList<Model*> m_models;

    DisplayMode m_displayMode;

    Model *m_contextMenuModel;
    Node *m_contextMenuNode;
};

} // namespace emd


#endif