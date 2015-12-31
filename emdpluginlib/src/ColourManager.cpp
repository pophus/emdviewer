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

#include "ColourManager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDomElement>
#include <QFile>
#include <qsettings.h>

#include "ColourMap.h"

namespace emd
{

ColourManager::~ColourManager()
{
	qDebug() << "enter ~ColourManager()";
	m_colourMaps.clear();
	qDebug() << "exit ~ColourManager()";
}

ColourMap ColourManager::colourMap(const int &index) const
{
	if(index < 0 || index >= m_colourMaps.size())
		return ColourMap();

	return m_colourMaps.at(index);
}

ColourMap ColourManager::colourMap(const QString &name) const
{
    for(const ColourMap &map : m_colourMaps)
    {
        if(map.name().compare(name) == 0)
            return map;
    }

    return defaultMap();
}

ColourMap ColourManager::defaultMap() const
{
    if (m_colourMaps.size() == 0) {
        return ColourMap();
    }
    
    QSettings settings;

    QVariant var = settings.value("Preferences/DefaultColourMap", QVariant());

    if(var.isValid())
    {
        return colourMap(var.toString());
    }

    return m_colourMaps.at(0);
}

void ColourManager::addColourMap(const ColourMap &map, int index)
{

	if(index < 0 || index > (int) m_colourMaps.size())
		index = (int) m_colourMaps.size();

    beginInsertRows(QModelIndex(), index, index);

	m_colourMaps.insert(m_colourMaps.begin() + index, map);

    endInsertRows();

    this->saveColourMaps();
}

void ColourManager::removeColourMap(const int &index)
{
	if(index < 0 || index >= m_colourMaps.size())
		return;

    beginRemoveRows(QModelIndex(), index, index);

	m_colourMaps.erase(m_colourMaps.begin() + index);

    endRemoveRows();

    this->saveColourMaps();
}

ColourMapIterator const ColourManager::begin() const
{
	return m_colourMaps.begin();
}

ColourMapIterator const ColourManager::end() const
{
	return m_colourMaps.end();
}

int ColourManager::indexOf(const ColourMap &map) const
{
    for(int index = 0; index < m_colourMaps.size(); ++index)
    {
        if(m_colourMaps.at(index) == map)
            return index;
    }

    return -1;
}

/********************************* File Management *********************************/

void ColourManager::loadColourMaps()
{
	// Attempt to open the colour map file. If it doesn't exist, copy the default
	// file from the resources.
	QString filePath = QCoreApplication::applicationDirPath() + "/colourmaps.xml";
	qDebug() << "map file: " << filePath;
	QFile colourMapFile(filePath);
	if(!colourMapFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Colour map file not found; loading defaults";

        if(!QFile::copy(":/colourmaps.xml", filePath))
        {
            qCritical() << "Failed to copy default colour map file";
            return;
        }

		// Try again
		if(!colourMapFile.open(QIODevice::ReadOnly))
		{
			qCritical() << "Failed to open default colour map file";
			return;
		}
	}

	QDomDocument mapDom("FSDF");

	// Try to open the existing colour map file
	if(!mapDom.setContent(&colourMapFile))
	{
		// Something wrong with the existing colour map file--use default.
		colourMapFile.close();
		qWarning() << "Bad colour map file; loading defaults";
		QFile defaultColourMaps(":/colourmaps.xml");
		colourMapFile.remove();
		defaultColourMaps.copy(filePath);

		// Try again
		if(!colourMapFile.open(QIODevice::ReadOnly))
		{
			qCritical() << "Failed to load default colourmaps";
			return;
		}
		if(!mapDom.setContent(&colourMapFile))
		{
			colourMapFile.close();
			qCritical() << "Default colour map file is corrupted";
			return;
		}
	}
	colourMapFile.close();
	
	// Parse the colour map file
	QDomElement root = mapDom.documentElement();
	QDomNodeList maps = root.elementsByTagName("colourmap");
	float p, r, g, b;
	bool success;
	int validStops;
	for(int iii = 0; iii < maps.length(); ++iii)
	{
		QDomElement map = maps.item(iii).toElement();
		QDomNodeList stops = map.elementsByTagName("stop");
		QLinearGradient gradient(0, 0, 1, 0);
		if(stops.length() < 2)
			continue;
		validStops = 0;
		for(int jjj = 0; jjj < stops.length(); ++jjj)
		{
			QDomElement stop = stops.item(jjj).toElement();

			p = stop.attribute("point").toFloat(&success);
			if(!success) continue;

			r = stop.attribute("r").toFloat(&success);
			if(!success) continue;

			g = stop.attribute("g").toFloat(&success);
			if(!success) continue;

			b = stop.attribute("b").toFloat(&success);
			if(!success) continue;

			// We have a valid stop
			validStops++;
			gradient.setColorAt(p, QColor(r * 255, g * 255, b * 255));
		}
		// If we found at least 2 stops, create the colour map
		if(validStops < 2)
			continue;

		// Add it to the map list
		m_colourMaps.push_back(ColourMap(gradient, map.attribute("name", "ColourMap")));
	}
}

void ColourManager::saveColourMaps()
{
	QString filePath = QCoreApplication::applicationDirPath() + "/colourmaps.xml";
	QFile colourMapFile(filePath);
	QDomDocument mapDom("");
	QDomElement root = mapDom.createElement("data");
	mapDom.appendChild(root);
	QDomElement mapElement;
	bool success;
	ColourMapIterator iter = this->begin();
	ColourMapIterator last = this->end();
	while(iter != last)
	{
		success = elementForColourMap(mapDom, mapElement, *iter);
		if(success)
			root.appendChild(mapElement);
		else
			qDebug() << "Failed to create colour map element";
		iter++;
	}

	// Write the updated xml back to the file
	colourMapFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
	if(!colourMapFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qWarning() << "Failed to open colour map file for writing";
	}
	else
	{
		QTextStream out(&colourMapFile);
		mapDom.save(out, 2);
	}
}

bool ColourManager::elementForColourMap(QDomDocument& mapDom, 
									 QDomElement& element, const ColourMap &map)
{	
	element = mapDom.createElement("colourmap");
	element.setAttribute("name", map.name());
	QGradientStops stops = map.gradient().stops();
	for(QGradientStop stop : stops)
	{
		QDomElement stopElement = mapDom.createElement("stop");
		stopElement.setAttribute("point", stop.first);
		QColor colour = stop.second;
		stopElement.setAttribute("r", colour.redF());
		stopElement.setAttribute("g", colour.greenF());
		stopElement.setAttribute("b", colour.blueF());
		element.appendChild(stopElement);
	}
	
	if(element.isNull())
	{
		qDebug() << "Failed to add new colour map element";
		return false;
	}
	
	return true;
}

// -----------------------------------------------------------------------------
// QAbstractItemModel

int ColourManager::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return (int)m_colourMaps.size();

    return 0;
}

int ColourManager::columnCount(const QModelIndex &) const
{
    return 1;
}

QModelIndex ColourManager::index(int row, int column, 
                                 const QModelIndex &parent) const
{
    if(parent.isValid())
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex ColourManager::parent(const QModelIndex &) const
{
    return QModelIndex();
}
    
QVariant ColourManager::data(const QModelIndex &index, int role) const
{
    if(index.isValid())
    {
        switch (role)
        {
        //case Qt::DisplayRole:
        //    return m_colourMaps.at(index.row()).name();
        case Qt::DecorationRole:
            return m_colourMaps.at(index.row()).icon();
        default:
            break;
        }
    }

    return QVariant();
}

} // namespace emd
