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

#include "HistogramView.h"

#include <qdebug.h>

namespace emd
{

static const int HISTOGRAM_H_MARGIN = 10;
static const int HISTOGRAM_V_MARGIN = 10;

HistogramView::HistogramView(QWidget *parent)
    : QGraphicsView(parent)
{

}

void HistogramView::resizeEvent(QResizeEvent *)
{
    //QWidget::resizeEvent(e);

    static const int AR = 2;

	int width = this->width();

    QSize size(width, width / AR);

    this->setFixedHeight(size.height());

    this->setSceneRect(QRect(-HISTOGRAM_H_MARGIN, -HISTOGRAM_V_MARGIN, size.width(), size.height()));

	emit(sizeChanged(size.width() - 2 * HISTOGRAM_H_MARGIN, size.height() - 2 * HISTOGRAM_V_MARGIN));
}

}

