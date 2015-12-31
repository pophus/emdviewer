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

#include "ImageWindow.h"

#include "GraphicsImageWidget.h"
#include "ImageWindowModule.h"

namespace emd
{

ImageWindow::ImageWindow(ImageWindowModule *module, QWidget *parent)
	: QDialog(parent),
	m_module(module)
{
	m_imageWidget = new GraphicsImageWidget(this);
	connect(m_module, SIGNAL(imageGenerated(QImage*)),
		m_imageWidget, SLOT(displayImage(QImage*)));

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_imageWidget);
	setLayout(layout);
}

void ImageWindow::closeEvent(QCloseEvent *e)
{
	emit(windowClosed());

	QDialog::closeEvent(e);
}

} // namespace emd


