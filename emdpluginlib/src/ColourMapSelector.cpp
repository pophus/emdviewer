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

#include "ColourMapSelector.h"

#include "ColourManager.h"
#include "ColourMap.h"
#include "GradientDialog.h"

namespace emd
{

ColourMapSelector::ColourMapSelector(QWidget *parent)
    : QWidget(parent)
{
    QPushButton *newColourMapButton = new QPushButton("+");
	newColourMapButton->setMaximumWidth(30);
	connect(newColourMapButton, SIGNAL(clicked()), 
        this, SLOT(newColourMap()));

    ColourMap defaultMap = ColourManager::instance().defaultMap();
	m_editColourMapButton = new QPushButton(defaultMap.name());
	m_editColourMapButton->setFixedWidth(110);
	connect(m_editColourMapButton, SIGNAL(clicked()), 
        this, SLOT(editColourMap()));

	QPushButton *deleteColourMapButton = new QPushButton("-");
	deleteColourMapButton->setMaximumWidth(30);
	connect(deleteColourMapButton, SIGNAL(clicked()), 
        this, SLOT(deleteColourMap()));

	QHBoxLayout *colourMapTitleLayout = new QHBoxLayout();
	colourMapTitleLayout->addStretch();
	colourMapTitleLayout->addWidget(newColourMapButton);
	colourMapTitleLayout->addWidget(m_editColourMapButton);
	colourMapTitleLayout->addWidget(deleteColourMapButton);
	colourMapTitleLayout->addStretch();

	m_colourMapBox = new QComboBox();
	m_colourMapBox->setIconSize(ColourMap::iconSize());
    m_colourMapBox->setModel(&ColourManager::instance());
    m_colourMapBox->setCurrentIndex(ColourManager::instance().indexOf(defaultMap));
	connect(m_colourMapBox, SIGNAL(activated(int)), 
        this, SLOT(setCurrentIndex(int)));

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addLayout(colourMapTitleLayout);
	layout->addWidget(m_colourMapBox);
	layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);
}

void ColourMapSelector::setCurrent(const QString &mapName)
{
    m_colourMapBox->setCurrentIndex(ColourManager::instance().indexOf(ColourManager::instance().colourMap(mapName)));

    m_editColourMapButton->setText(mapName);

	emit(currentChanged(mapName));
}

void ColourMapSelector::newColourMap()
{
	// Launch a create gradient dialog with a default gradient
	QLinearGradient gradient(0, 0, 1, 0);
	gradient.setColorAt(0, QColor(0, 0, 0));
	gradient.setColorAt(1, QColor(255, 255, 255));
	GradientDialog *dialog = new GradientDialog(gradient, "New Colour Map");
	bool created = (bool)dialog->exec();
	if(!created)
		return;

	ColourMap map(dialog->gradient(), dialog->name());
	ColourManager::instance().addColourMap(map);

    setCurrent(map.name());

	delete dialog;
}

void ColourMapSelector::editColourMap()
{
	int index = m_colourMapBox->currentIndex();
	const ColourMap &colourMap = ColourManager::instance().colourMap(index);
	GradientDialog *dialog = new GradientDialog(colourMap.gradient(), colourMap.name());
	bool created = (bool)dialog->exec();
	if(!created)
		return;

	ColourManager::instance().removeColourMap(index);

	ColourMap map(dialog->gradient(), dialog->name());
	ColourManager::instance().addColourMap(map, index);

    setCurrent(map.name());

	delete dialog;
}

void ColourMapSelector::deleteColourMap()
{
	int index = m_colourMapBox->currentIndex();
	if(index == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("A thousand apologies, User. The default\n"
						"colour map cannot be deleted.");
		msgBox.exec();
		return;
	}
	
	ColourManager::instance().removeColourMap(index);
	m_colourMapBox->removeItem(index);
	
	setCurrent(ColourManager::instance().defaultMap().name());
}

void ColourMapSelector::setCurrentIndex(int index)
{
    setCurrent(ColourManager::instance().colourMap(index).name());
}

} // namespace emd