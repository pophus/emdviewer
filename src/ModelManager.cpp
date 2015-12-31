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

#include "ModelManager.h"

#include <QAction>
#include <QApplication>
#include <qboxlayout.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include "Attribute.h"
#include "DataTypeBox.h"
#include "Model.h"

namespace emd
{

ModelManager::ModelManager()
    : QAbstractItemModel(),
    m_displayMode(DisplayModeDataGroups)
{
    
}

ModelManager::~ModelManager()
{
    qDeleteAll(m_models);
}

// Public

void ModelManager::addModel(Model *model, bool setActive)
{
    beginInsertRows(QModelIndex(), m_models.count(), m_models.count());
    m_models.append(model);
    endInsertRows();

    if(setActive)
    {
        this->displayDataGroup(model);
    }
}

void ModelManager::removeModel(Model *model)
{
    int index = m_models.indexOf(model);
    if(index < 0)
        return;

    this->closeModel(model);

    beginRemoveRows(QModelIndex(), index, index);
    m_models.removeAt(index);
    endRemoveRows();

    delete model;
}

void ModelManager::removeModel(const QModelIndex &index)
{
    Model *model = modelForIndex(index);

    if(model)
        removeModel(model);
}

void ModelManager::displayDataGroup(Model *model, int dataGroupIndex)
{
    if(!model)
    {
        return;
    }

    int index = m_models.indexOf(model);
    if(index < 0)
        return;
        
    if(dataGroupIndex < 0)
        dataGroupIndex = 0;

    if(dataGroupIndex >= model->dataGroupCount())
        return;

    if(!model->dataGroupAtIndex(dataGroupIndex)->isLoaded())
    {
        bool success = model->loadDataGroup(dataGroupIndex);

        if(!success)
        {
            qDebug() << "Failed to load data group at index: " << dataGroupIndex;
            return;
        }
    }

    QModelIndex modelIndex = this->index(m_models.indexOf(model), 0);

    QVector<int> roles;
    roles.append(Qt::FontRole);
    emit(dataChanged(modelIndex, modelIndex, roles));
    
    QModelIndex groupIndex = this->index(dataGroupIndex, 0, modelIndex);
    emit(dataChanged(groupIndex, groupIndex, roles));

    emit(dataGroupLoaded(model->dataGroupAtIndex(dataGroupIndex)));
}

void ModelManager::closeModel(Model *model)
{
    if(!model || !model->anyLoaded())
        return;

    QModelIndex modelIndex = this->index(m_models.indexOf(model), 0);

    QVector<int> roles;
    roles.append(Qt::FontRole);

    for(int index = 0; index < model->dataGroupCount(); ++index)
    {
        if(model->dataGroupAtIndex(index)->isLoaded())
        {
            emit(dataGroupUnloaded(model->dataGroupAtIndex(index)));
    
            QModelIndex groupIndex = this->index(index, 0, modelIndex);
            emit(dataChanged(groupIndex, groupIndex, roles));
        }
    }

    model->unloadDataGroups();

    emit(dataChanged(modelIndex, modelIndex, roles));
}

void ModelManager::closeModel(const QModelIndex &index)
{
    Model *model = modelForIndex(index);

    if(model)
        closeModel(model);
}

void ModelManager::closeDataGroup(DataGroup *dataGroup)
{
    if(dataGroup->isLoaded())
    {
        emit(dataGroupUnloaded(dataGroup));

        dataGroup->unload();

        Model *model = dataGroup->model();

        QModelIndex modelIndex = this->index(m_models.indexOf(model), 0);
        QVector<int> roles;
        roles.append(Qt::FontRole);
        emit(dataChanged(modelIndex, modelIndex, roles));
    
        QModelIndex groupIndex = this->index(model->indexOfDataGroup(dataGroup), 0, modelIndex);
        emit(dataChanged(groupIndex, groupIndex, roles));
    }
}

ModelManager::DisplayMode ModelManager::displayMode() const
{
    return m_displayMode;
}

void ModelManager::setDisplayMode(ModelManager::DisplayMode mode)
{
    beginResetModel();

    m_displayMode = mode;

    endResetModel();
}

void ModelManager::getActionsForIndex(const QModelIndex &index, QList<QAction*> &actions)
{
    m_contextMenuModel = modelForIndex(index);
    if(!m_contextMenuModel)
        return;

    m_contextMenuNode = static_cast<Node*>(index.internalPointer());
    if(!m_contextMenuNode)
        return;

    if(!index.parent().isValid())
    {
        QAction *activateModelAction = new QAction(tr("Activate"), NULL);
        connect(activateModelAction, SIGNAL(triggered()),
            this, SLOT(handleActivateAction()));

        actions.append(activateModelAction);

        QAction *closeModelAction = new QAction(tr("Close"), NULL);
        connect(closeModelAction, SIGNAL(triggered()),
            this, SLOT(handleCloseAction()));

        actions.append(closeModelAction);
    }
    else
    {
        if(index.column() == 0)
        {

		    if((m_contextMenuNode->status() & Node::DATAGROUP) == (m_contextMenuNode->status() | Node::DATAGROUP))
		    {
			    QAction *displayDataGroupAction = new QAction(tr("Display"), NULL);
			    connect(displayDataGroupAction, SIGNAL(triggered()), 
                    this, SLOT(handleDisplayAction()));

			    actions.append(displayDataGroupAction);
		    }

		    if(m_contextMenuNode->status() & (Node::GROUP | Node::DATASET))
		    {
			    QAction *addAttributeAction = new QAction(tr("Add Attribute"), NULL);
			    connect(addAttributeAction, SIGNAL(triggered()),
                    this, SLOT(handleAddAttribute()));

			    actions.append(addAttributeAction);
		    }

       //     if(m_contextMenuModel->canDeleteNode(m_contextMenuNode))
       //     {
       //         QAction *deleteNodeAction = new QAction(tr("Delete"), NULL);
			    ////connect(addAttributeAction, SIGNAL(triggered()),
       //             //this, SLOT(addAttribute()));

			    //actions.append(deleteNodeAction);
       //     }
        }
	}
}

bool ModelManager::fileOpen(const QString &path)
{
    for(int index = 0; index < m_models.count(); ++index)
    {
        if(m_models.at(index)->filePath().compare(path) == 0)
            return true;
    }

    return false;
}

void ModelManager::print() const
{
    QList<QModelIndex> indexes;

    for(int iii = 0; iii < this->rowCount(); ++iii)
        indexes.append(this->index(iii, 0));

    while(indexes.count() > 0)
    {
        QModelIndex index = indexes.takeFirst();

        for(int iii = 0; iii < this->rowCount(index); ++iii)
            indexes.append(this->index(iii, 0, index));

        qDebug() << this->data(index, Qt::DisplayRole).toString() << "(" << this->data(this->parent(index), Qt::DisplayRole).toString() << ")";
    }

    for(int iii = 0; iii < m_models.count(); ++iii)
    {
        qDebug() << "---" << m_models.at(iii)->fileName() << "---";
        m_models.at(iii)->print();
    }
}

// Actions

void ModelManager::handleDoubleClickAtIndex(const QModelIndex &index)
{
    if(!index.isValid())
        return;
        
    Model *model = this->modelForIndex(index);
    if(!model)
        return;

    int dataGroupIndex = -1;

    if(index.parent().isValid())
    {
        Node *node = static_cast<Node*>(index.internalPointer());
        dataGroupIndex = model->indexOfDataGroup((DataGroup*) node);

        // Don't change the model if we didn't click on a data group.
        if(dataGroupIndex < 0)
            return;
    }

    if(model->anyLoaded() && dataGroupIndex == -1)
    {
        // Clicked on a loaded model
        this->closeModel(model);
    }
    else if(dataGroupIndex >= 0 && model->dataGroupAtIndex(dataGroupIndex)->isLoaded())
    {
        // Clicked on a loaded data group
        this->closeDataGroup(model->dataGroupAtIndex(dataGroupIndex));
    }
    else
    {
        // Clicked on an unloaded model or group
        this->displayDataGroup(model, dataGroupIndex);
    }
}

void ModelManager::handleActivateAction()
{
    if(!m_contextMenuModel)
        return;

    this->displayDataGroup(m_contextMenuModel);
}

void ModelManager::handleAddAttribute()
{
    if(!m_contextMenuNode || !m_contextMenuModel)
        return;

	QLineEdit *nameBox = new QLineEdit("attribute");
	nameBox->setFixedWidth(200);
	DataTypeBox *typeBox = new DataTypeBox();
	typeBox->setFixedWidth(100);
	QLineEdit *valueBox = new QLineEdit("value");
	valueBox->setFixedWidth(100);
	QPushButton *cancelButton = new QPushButton("Cancel");
	cancelButton->setFixedWidth(80);
	QPushButton *okButton = new QPushButton("OK");
	okButton->setFixedWidth(80);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(nameBox, 0, 0, 1, 2, Qt::AlignCenter);
	layout->addWidget(typeBox, 1, 0, 1, 1, Qt::AlignRight);
	layout->addWidget(valueBox, 1, 1, 1, 1, Qt::AlignLeft);
	layout->addWidget(cancelButton, 2, 0, 1, 1, Qt::AlignLeft);
	layout->addWidget(okButton, 2, 1, 1, 1, Qt::AlignRight);

	QDialog *dialog = new QDialog();
	dialog->setLayout(layout);
	dialog->setWindowTitle("Add Attribute");
	connect(cancelButton, SIGNAL(clicked()),
		dialog, SLOT(reject()));
	connect(okButton, SIGNAL(clicked()),
		dialog, SLOT(accept()));

	int accepted = dialog->exec();
	if(accepted > 0)
	{
		QString name = nameBox->text();
		emd::DataType type = typeBox->currentType();

        QModelIndex parentIndex = createIndex(m_contextMenuNode->rowNumber(), 0, m_contextMenuNode);
        this->beginInsertRows(parentIndex, m_contextMenuNode->childCount(), m_contextMenuNode->childCount());

		Attribute *node = dynamic_cast<Attribute*>(
			m_contextMenuModel->addNode(name, Node::ATTRIBUTE, m_contextMenuNode));
		node->setType(type);

        this->endInsertRows();
		
		QString valueString = valueBox->text();
		QVariant value = emd::emdTypeFromString(valueString, type);
		node->setValue(value);
	}

	delete dialog;
}

void ModelManager::handleCloseAction()
{
    if(!m_contextMenuModel)
        return;
    
    this->removeModel(m_contextMenuModel);
}

void ModelManager::handleDisplayAction()
{
    if(!m_contextMenuModel)
        return;
    
    if(!m_contextMenuNode)
        return;

    this->displayDataGroup(m_contextMenuModel, m_contextMenuModel->indexOfDataGroup((DataGroup*)m_contextMenuNode));
}

// QAbstractProxyModel

int ModelManager::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return m_models.count();
    
    Model *emdModel = modelForIndex(parent);

    switch(m_displayMode)
    {
    case DisplayModeAll:
        if(emdModel)
            return emdModel->rowCount(parent);
        break;
    case DisplayModeDataGroups:
        if(emdModel && parent.internalPointer() == emdModel->root())
            return emdModel->dataGroupCount();
        break;
    }

    return 0;
}

int ModelManager::columnCount(const QModelIndex &) const
{
    return 2;
}

QModelIndex ModelManager::index(int row, int column, const QModelIndex &parent) const
{
    if(!parent.isValid())
    {
        if(row < m_models.count())
            return createIndex(row, column, m_models.at(row)->root());
        else
            return QModelIndex();
    }

    Model *emdModel = modelForIndex(parent);

    switch(m_displayMode)
    {
    case DisplayModeAll:
        if(emdModel)
            return createIndex(row, column, emdModel->index(row, column, parent).internalPointer());
    case DisplayModeDataGroups:
        if(emdModel)
            return createIndex(row, column, emdModel->dataGroupAtIndex(row));
    }

    return QModelIndex();
}

QModelIndex ModelManager::parent(const QModelIndex &index) const
{
    if(!index.isValid())
        return QModelIndex();

    Model *emdModel = modelForIndex(index);

    if(m_displayMode == DisplayModeAll)
    {
        if(emdModel)
        {
            QModelIndex subIndex = emdModel->parent(index);
        
            if(subIndex.isValid())
                return createIndex(subIndex.row(), subIndex.column(), subIndex.internalPointer());
        }
    }
    else if(m_displayMode == DisplayModeDataGroups)
    {
        if(emdModel)
        {
            if(emdModel->root() == index.internalPointer())
                return QModelIndex();
            else
                return createIndex(m_models.indexOf(emdModel), 0, emdModel->root());
        }
    }

    return QModelIndex();
}

Qt::ItemFlags ModelManager::flags(const QModelIndex &index) const
{
	if(!index.isValid())
		return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled;

    Node *node = static_cast<Node *>(index.internalPointer());
    if(node->status() & Node::ATTRIBUTE)
        flags |= Qt::ItemIsEditable;

	return flags;
}

QVariant ModelManager::data(const QModelIndex &index, int role) const
{
    if(index.isValid())
    {
        Model *emdModel = modelForIndex(index);

        if(emdModel)
        {
            switch (role)
            {
            case Qt::DisplayRole:
            {
                return emdModel->data(index, role);
            }
            case Qt::FontRole:
                if(index.column() != 0)
                    break;
                if((!index.parent().isValid() && emdModel->anyLoaded())
                    || (emdModel->isDataGroup(index.internalPointer())
                        && ((DataGroup*)index.internalPointer())->isLoaded()))
                {
                    QFont font = QApplication::font();
                    font.setBold(true);
                    return font;
                }
                break;
            default:
                break;
            }
        }
    }

    return QVariant();
}

bool ModelManager::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(index.isValid())
    {
        Model *emdModel = modelForIndex(index);

        if(emdModel)
            return emdModel->setData(index, value, role);
    }

    return false;
}

// Utility

Model *ModelManager::modelForIndex(const QModelIndex &index) const
{
    if(index.isValid())
    {
        foreach(Model *model, m_models)
        {
            if(model->ownsIndex(index))
                return model;
        }
    }

    return NULL;
}

} // namespace emd
