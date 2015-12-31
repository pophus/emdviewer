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

#include "StatusLabel.h"

#include <qboxlayout.h>
#include <QPixmap>

namespace emd
{

StatusLabel::StatusLabel()
    : QWidget()
{
    m_iconLabel = new QLabel();

    m_textLabel = new QLabel();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_textLabel, 0, Qt::AlignLeft);

    this->setLayout(layout);
}

void StatusLabel::setText(const QString &text)
{
    m_textLabel->setText(text);
}

void StatusLabel::setPixmap(const QPixmap &pixmap)
{
    m_iconLabel->setPixmap(pixmap);
}

void StatusLabel::enterEvent(QEvent *)
{
    emit(hoverChanged(true));
}

void StatusLabel::leaveEvent(QEvent *)
{
    emit(hoverChanged(false));
}

} //namespace emd