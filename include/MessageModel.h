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

#ifndef EMD_MESSAGEMODEL_H
#define EMD_MESSAGEMODEL_H

#include "Message.h"
#include <QAbstractListModel>
#include <qlist.h>

namespace emd
{

class MessageModel : public QAbstractListModel
{
    Q_OBJECT

public:
	MessageModel(QObject *parent = 0);
    ~MessageModel();

    void setEnabled(bool enabled);

	// These functions are required to subclass QAbstractListModel.
	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	QVariant data(const QModelIndex &index, int role) const;

	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;

	bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
	bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());

	// Mutators
	void addMessage(const QString &msg, const MessageType &type = MessageTypeNotification);

private:
	QList<Message> messageList;
    bool m_enabled;

signals:
    void messageLogged();
    void messageLogged(const QString &msg, MessageType type);
};

} // namespace emd


#endif