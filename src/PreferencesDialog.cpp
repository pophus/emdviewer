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

#include "PreferencesDialog.h"

#include <QtWidgets>

#include "ColourManager.h"

namespace emd
{

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Preferences");
    setMinimumSize(600, 400);

    m_colourMapLabel = new QLabel();

    m_colourMapBox = new QComboBox();
    m_colourMapBox->setIconSize(QSize(ColourMap::iconSize()));
    m_colourMapBox->setModel(&ColourManager::instance());
    connect(m_colourMapBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(updateColourMapLabel(int)));

    QLabel *colourMapTitle = new QLabel("Default Colour Map:");

    QGridLayout *colourMapLayout = new QGridLayout();
    colourMapLayout->addWidget(colourMapTitle, 0, 0, 1, 1, Qt::AlignRight);
    colourMapLayout->addWidget(m_colourMapLabel, 0, 1, 1, 1, Qt::AlignLeft);
    colourMapLayout->addWidget(m_colourMapBox, 1, 0, 1, 2, Qt::AlignCenter);
    colourMapLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Maximum), 0, 2, 2, 1);


    QPushButton *cancelButton = new QPushButton("Cancel");
    connect(cancelButton, SIGNAL(clicked()),
        this, SLOT(cancel()));

    QPushButton *saveButton = new QPushButton("Save");
    connect(saveButton, SIGNAL(clicked()),
        this, SLOT(save()));

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(saveButton);

    QGroupBox *buttonGroup = new QGroupBox();
    buttonGroup->setFlat(true);
    buttonGroup->setLayout(buttonLayout);


    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(colourMapLayout);
    layout->addStretch();
    layout->addWidget(buttonGroup);

    setLayout(layout);

    loadSettings();
}

void PreferencesDialog::loadSettings()
{
    ColourMap defaultMap = ColourManager::instance().defaultMap();

    m_colourMapBox->setCurrentIndex(ColourManager::instance().indexOf(defaultMap));
    m_colourMapLabel->setText(defaultMap.name());
}

void PreferencesDialog::saveSettings()
{
    QSettings settings;

    ColourMap defaultMap = ColourManager::instance().colourMap(m_colourMapBox->currentIndex());
    settings.setValue("Preferences/DefaultColourMap", defaultMap.name());
}

void PreferencesDialog::cancel()
{
    reject();

    loadSettings();
}

void PreferencesDialog::save()
{
    accept();

    saveSettings();
}

void PreferencesDialog::updateColourMapLabel(int index)
{
    m_colourMapLabel->setText(ColourManager::instance().colourMap(index).name());
}

} // namespace emd
