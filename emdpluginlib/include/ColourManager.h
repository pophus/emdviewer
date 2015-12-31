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

#ifndef EMD_COLOURMANAGER_H
#define EMD_COLOURMANAGER_H

#include "EmdPluginLib.h"

#include <vector>

#include <qabstractitemmodel.h>
#include <QLinearGradient>

#include "ColourMap.h"

class QDomDocument;
class QDomElement;

namespace emd
{

typedef std::vector<ColourMap>::const_iterator ColourMapIterator;

// The ColourManager is responsible for maintaining the set of ColourMaps used
// by the application. ColourMaps may only be created or destroyed through the
// ColourManager. The ColourManager is also responsible for tracking the
// "current" ColourMap, i.e. the ColourMap used to colour the data image.
//
// The ColourManager is a singleton object.
class EMDPLUGIN_API ColourManager : public QAbstractItemModel
{
	Q_OBJECT

public:
	static ColourManager &instance()
	{
		static ColourManager instance;

        static bool initialized = false;
        if(!initialized)
        {
            initialized = true;
            instance.loadColourMaps();
        }

		return instance;
	}

	~ColourManager();

	ColourMap colourMap(const int &index) const;
    ColourMap colourMap(const QString &name) const;

    ColourMap defaultMap() const;

	void addColourMap(const ColourMap &map, int index = -1);

	// Currently ColourMaps may only be removed by index.
	void removeColourMap(const int &index);

	// Iterators are provided to traverse the list of ColourMaps.
	ColourMapIterator const begin() const;
	ColourMapIterator const end() const;

    int indexOf(const ColourMap &map) const;

    // QAbstractItemModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex index(int row, int column,
					const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	QVariant data(const QModelIndex &index, int role) const override;

private:
	void loadColourMaps();
	void saveColourMaps();
	bool elementForColourMap(QDomDocument& mapDom, QDomElement& element, const ColourMap &map);
	
private:
	std::vector<ColourMap> m_colourMaps;
};

} // namespace emd

#endif
