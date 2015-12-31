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

#include "GradientDialog.h"

namespace emd
{

#define GRADIENT_WIDTH 280

GradientScene::GradientScene(QLinearGradient &gradient, QObject *parent)
	: QGraphicsScene(parent), m_gradient(gradient)
{
	m_gradient.setStart(0, 0);
	m_gradient.setFinalStop(GRADIENT_WIDTH, 0);
	draggedHandle = 0;
	selectedHandle = 0;

	this->setSceneRect(-10, -25, GRADIENT_WIDTH + 20, 100);
	rect.setRect(0, 0, GRADIENT_WIDTH, 65);
	this->addItem(&rect);
	rect.setPen(QPen(QColor(Qt::white)));
	createStops();
	updateGradient();
}

GradientScene::~GradientScene()
{

}

QColor GradientScene::currentHandleColour() const
{
	if(!selectedHandle)
		return QColor();
	return stops[selectedIndex].second;
}

void GradientScene::setActiveHandleColour(const QColor &colour)
{
	stops[selectedIndex].second = colour;
	((QGraphicsPolygonItem*)selectedHandle)->setBrush(QBrush(colour));
	m_gradient.setStops(stops);
	updateGradient();
}

void GradientScene::removeCurrentHandle()
{
	if(selectedIndex < 1 || selectedIndex >= stops.count()-1)
		return;
	stops.remove(selectedIndex);
	stopHandles.removeOne(selectedHandle);
	removeItem(selectedHandle);
	selectedHandle = 0;
	selectedIndex = -1;
	m_gradient.setStops(stops);
	updateGradient();
}

void GradientScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
	if(e->button() == Qt::LeftButton)
	{
		QGraphicsItem *handle = itemAt(e->scenePos(), QTransform());
		if(!handle || handle == &rect)
		{
			if(selectedHandle)
				selectedHandle->setSelected(false);
			selectedHandle = 0;
			return;
		}
		int index = stopHandles.indexOf(handle);
		if(index < 0)
			return;

		if(selectedHandle != handle)
		{
			if(selectedHandle)
				selectedHandle->setSelected(false);
			selectedHandle = handle;
			selectedHandle->setSelected(true);
			selectedIndex = index;
		}

		if(index > 0 && index < stops.count()-1)
		{
			draggedHandle = handle;
			draggedIndex = index;
		}
	}

	e->accept();
}
void GradientScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
	if(!draggedHandle)
		return;
	float newX = draggedHandle->pos().x() + e->scenePos().x() - e->lastScenePos().x();
	if(newX < 0.0f)
		newX = 0.0f;
	if(newX > GRADIENT_WIDTH)
		newX = GRADIENT_WIDTH;
	draggedHandle->setPos(newX, 0);
	stops[draggedIndex].first = newX / GRADIENT_WIDTH;
	m_gradient.setStops(stops);
	updateGradient();
	e->accept();
}
void GradientScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
	if(e->button() == Qt::LeftButton)
	{
		draggedHandle = 0;
	}
	e->accept();
}
void GradientScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e)
{
	QGraphicsScene::mouseDoubleClickEvent(e);
}

void GradientScene::createStops()
{
	stops = m_gradient.stops();

	int index = 0;
	foreach(QGradientStop stop, stops)
		createHandleFromStop(stop, index++);
}

void GradientScene::addStop(QColor colour)
{
	float max = 0.0f, start, end, maxStart;
	int index;
	for(int iii = 0; iii < stops.count() - 1; ++iii)
	{
		start = stops.at(iii).first;
		end = stops.at(iii+1).first;
		if(end - start > max)
		{
			max = end - start;
			maxStart = start;
			index = iii;
		}
	}
	QGradientStop stop(maxStart + max/2, colour);
	stops.insert(index+1, stop);
	m_gradient.setStops(stops);
	if(selectedHandle)
		selectedHandle->setSelected(false);
	selectedHandle = createHandleFromStop(stop, index+1);
	selectedHandle->setSelected(true);
	updateGradient();
}

QGraphicsItem *GradientScene::createHandleFromStop(QGradientStop stop, int index)
{
	QPen outlinePen(Qt::black);	
	QGraphicsPolygonItem *handle;
	QPolygon outline;
	outline << QPoint(0, 0) << QPoint(5, -5) << QPoint(5, -15)
			<< QPoint(-5, -15) << QPoint(-5, -5) << QPoint(0, 0);
	handle = new QGraphicsPolygonItem(outline);
	handle->setBrush(QBrush(stop.second));
	handle->setPen(outlinePen);
		
	this->addItem(handle);
	handle->setPos(stop.first * GRADIENT_WIDTH, 0);
	handle->setFlag(QGraphicsItem::ItemIsSelectable);
	stopHandles.insert(index, handle);
	
	return handle;
}

void GradientScene::updateGradient()
{
	rect.setBrush(QBrush(m_gradient));
	update();
}


GradientDialog::GradientDialog(const QLinearGradient &gradient, 
	const QString &name, QWidget *parent)
	: QDialog(parent)
{
	m_gradient = gradient;

	QPushButton *addColourButton = new QPushButton("Add Colour");
	connect(addColourButton, SIGNAL(clicked()), this, SLOT(addColour()));
	QPushButton *editColourButton = new QPushButton("Edit Colour");
	connect(editColourButton, SIGNAL(clicked()), this, SLOT(editColour()));
	QPushButton *removeColourButton = new QPushButton("Remove Colour");
	connect(removeColourButton, SIGNAL(clicked()), this, SLOT(removeColour()));

	scene = new GradientScene(m_gradient, this);
	QGraphicsView *view = new QGraphicsView(scene);

	QHBoxLayout *controlsLayout = new QHBoxLayout();
	controlsLayout->addStretch();
	controlsLayout->addWidget(addColourButton);
	controlsLayout->addWidget(editColourButton);
	controlsLayout->addWidget(removeColourButton);
	controlsLayout->addStretch();

	nameEdit = new QLineEdit(name);
	QPushButton *saveButton = new QPushButton("Save");
	connect(saveButton, SIGNAL(clicked()), this, SLOT(accept()));
	QPushButton *cancelButton = new QPushButton("Cancel");
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	
	QHBoxLayout *saveCancelLayout = new QHBoxLayout();
	saveCancelLayout->addWidget(nameEdit);
	saveCancelLayout->addStretch();
	saveCancelLayout->addWidget(saveButton);
	saveCancelLayout->addWidget(cancelButton);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->addLayout(controlsLayout);
	mainLayout->addWidget(view);
	mainLayout->addLayout(saveCancelLayout);

	this->setLayout(mainLayout);
	setWindowTitle("Edit Colour Map");
}

GradientDialog::~GradientDialog()
{
	delete scene;
}

void GradientDialog::addColour()
{
	QColor colour = QColorDialog::getColor();
	if(!colour.isValid())
		return;
	scene->addStop(colour);
}

void GradientDialog::editColour()
{
	if(!scene->isHandleSelected())
		return;
	QColor colour = QColorDialog::getColor(scene->currentHandleColour());
	if(!colour.isValid())
		return;
	scene->setActiveHandleColour(colour);
}

void GradientDialog::removeColour()
{
	if(!scene->isHandleSelected())
		return;
	scene->removeCurrentHandle();
}

} // namespace emd
