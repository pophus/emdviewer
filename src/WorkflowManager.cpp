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

#include "WorkflowManager.h"

#include <qapplication.h>
#include <qfont.h>
#include <qdebug.h>

#include "Workflow.h"

namespace emd
{

WorkflowManager::WorkflowManager()
{
	
}

WorkflowManager::~WorkflowManager()
{
	clear();
}

void WorkflowManager::clear()
{
    for(WorkflowItem *group : m_workflowItems)
    {
        for(WorkflowItem *workflow : group->children)
            delete workflow;

        delete group;
    }
}

/***************************** Public functions **********************************/

void WorkflowManager::refresh()
{
    beginResetModel();

    clear();

    m_workflowItems.clear();

    std::vector<std::string> groups = Workflow::groups();

    for(auto &group : groups)
    {
        // Add the group
        WorkflowItem *groupItem = new WorkflowItem();
        groupItem->name = QString(group.c_str());
        m_workflowItems.append(groupItem);

        // Add the children
        std::vector<std::string> workflows = Workflow::workflows(group);
        for(auto &workflow : workflows)
        {
            WorkflowItem *workflowItem = new WorkflowItem();
            workflowItem->name = QString(workflow.c_str());
            workflowItem->group = groupItem;

            groupItem->children.append(workflowItem);
        }
    }

    endResetModel();
}

Workflow *WorkflowManager::workflow(const QModelIndex &index)
{
    if(!index.isValid() || !index.parent().isValid())
        return nullptr;

    WorkflowItem *workflowItem = static_cast<WorkflowItem*>(index.internalPointer());
    if(workflowItem)
    {
        Workflow *workflow = Workflow::create(workflowItem->group->name.toStdString(),
                                              workflowItem->name.toStdString());

        return workflow;
    }

    return nullptr;
}

QString WorkflowManager::workflowDescription(const QModelIndex &index)
{
    if(!index.isValid() || !index.parent().isValid())
        return QString();

    WorkflowItem *workflowItem = static_cast<WorkflowItem*>(index.internalPointer());
    if(workflowItem)
    {
        return QString(Workflow::description(workflowItem->group->name.toStdString(),
            workflowItem->name.toStdString()).c_str());
    }

    return QString();
}

/*********************************** QAbstractItemModel ******************************/

int WorkflowManager::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return m_workflowItems.count();
    
    WorkflowItem *group = static_cast<WorkflowItem*>(parent.internalPointer());
    if(group)
    {
        return group->children.count();
    }

    return 0;
}

int WorkflowManager::columnCount(const QModelIndex &) const
{
    return 1;
}

QModelIndex WorkflowManager::index(int row, int column, const QModelIndex &parent) const
{
    if(!parent.isValid())
    {
        if(row < m_workflowItems.count())
            return createIndex(row, column, m_workflowItems.at(row));
    }
    else
    {
        WorkflowItem *group = static_cast<WorkflowItem*>(parent.internalPointer());
        if(group)
        {
            return createIndex(row, column, group->children.at(row));
        }
    }

    return QModelIndex();
}

QModelIndex WorkflowManager::parent(const QModelIndex &parent) const
{
     WorkflowItem *workflowItem = static_cast<WorkflowItem*>(parent.internalPointer());
     if(workflowItem && workflowItem->group)
     {
         return createIndex(m_workflowItems.indexOf(workflowItem->group), 
             0, workflowItem->group);
     }

     return QModelIndex();
}

QVariant WorkflowManager::headerData(int section, Qt::Orientation /*orientation*/,
					                 int role) const
{
	if(role == Qt::DisplayRole)
	{
		if(section == 0)
			return QString("Available Workflows");
	}
	else if(role == Qt::TextAlignmentRole)
		return Qt::AlignCenter;

    return QVariant();
}


QVariant WorkflowManager::data(const QModelIndex &index, int role) const
{
    if(index.isValid())
    {
        WorkflowItem *workflowItem = static_cast<WorkflowItem*>(index.internalPointer());
        if(!workflowItem)
            return QVariant();

        switch (role)
        {
        case Qt::DisplayRole:
            return QVariant(workflowItem->name);
        case Qt::FontRole:
            if(workflowItem->group == nullptr)
            {
                QFont font = QApplication::font();
                font.setBold(true);
                return font;
            }
        default:
            break;
        }
    }

    return QVariant();
}

} // namespace emd


