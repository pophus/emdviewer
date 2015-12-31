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

#include "MessageModel.h"

#include <qicon.h>

namespace emd
{

MessageModel::MessageModel(QObject *parent)
	: QAbstractListModel(parent),
    m_enabled(true)
{

}

MessageModel::~MessageModel()
{
    m_enabled = false;
}

void MessageModel::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

int MessageModel::rowCount(const QModelIndex &/*parent*/) const
{
	return messageList.size();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
	if(Qt::DisplayRole == role)
	{
		return messageList.at(index.row()).message();
	}
	else if(Qt::DecorationRole == role)
	{
		switch(messageList.at(index.row()).type())
		{
		case MessageTypeNotification:
			return QIcon(":/images/notification.png");
        case MessageTypeWarning:
			return QIcon(":/images/warning.png");
		case MessageTypeError:
			return QIcon(":/images/error.png");
        case MessageTypeDebug:
			return QIcon(":/images/debug.png");
		default:
			return QVariant();
		}
	}
	
	return QVariant();
}

QVariant MessageModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
	return QVariant();
}

bool MessageModel::insertRows(int position, int rows, const QModelIndex &parent)
{
	if(position < 0 || position > messageList.size())
		return false;

	beginInsertRows(parent, messageList.size(), messageList.size() + rows - 1);

	endInsertRows();

	return true;
}
bool MessageModel::removeRows(int position, int /*rows*/, const QModelIndex &/*parent*/)
{
	if(position < 0 || position > messageList.size())
		return false;

	return false;
}

void MessageModel::addMessage(const QString &msg, const MessageType &type)
{
    if(!m_enabled)
        return;

	messageList.append(Message(msg, type));
	
	insertRows(messageList.size(), 1);

    emit(messageLogged());
    emit(messageLogged(msg, type));
}

} // namespace emd
