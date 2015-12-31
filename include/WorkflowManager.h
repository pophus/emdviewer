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

#ifndef EMD_WORKFLOWMANAGER_H
#define EMD_WORKFLOWMANAGER_H

#include <QAbstractItemModel>

namespace emd
{

class Workflow;

class WorkflowManager : public QAbstractItemModel
{
	Q_OBJECT

public:
	WorkflowManager();
	~WorkflowManager();

	// Interface 
    void refresh();

    Workflow *workflow(const QModelIndex &index);
    QString workflowDescription(const QModelIndex &index);

    // QAbstractItemModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	QModelIndex index(int row, int column,
					const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
    
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role) const;

private:
    struct WorkflowItem
    {
        QString name;
        WorkflowItem *group;
        QList<WorkflowItem*> children;
        WorkflowItem() : group(nullptr)  {}
    };

    void clear();

private:
    QList<WorkflowItem *> m_workflowItems;
};

} // namespace emd

#endif