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

#ifndef EMD_GRADIENTDIALOG_H
#define EMD_GRADIENTDIALOG_H

#include <QtGui>
#include <QtWidgets>

namespace emd
{

class GradientScene : public QGraphicsScene
{
	Q_OBJECT

public:
	GradientScene(QLinearGradient &gradient, QObject *parent = 0);
	~GradientScene();

	bool isHandleSelected() const {return selectedHandle ? true : false;}
	QColor currentHandleColour() const;
	void setActiveHandleColour(const QColor &colour);
	void removeCurrentHandle();

	void mousePressEvent(QGraphicsSceneMouseEvent *e);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);

private:
	QLinearGradient &m_gradient;
	QGraphicsRectItem rect;
	QGradientStops stops;
	QList<QGraphicsItem*> stopHandles;
	QGraphicsItem *draggedHandle;
	int draggedIndex;
	QGraphicsItem *selectedHandle;
	int selectedIndex;

	void createStops();
	QGraphicsItem *createHandleFromStop(QGradientStop stop, int index);
	void updateGradient();

public slots:
	void addStop(QColor colour);

signals:
	void selectedStopChanged(int index);
};

class GradientDialog : public QDialog
{
	Q_OBJECT

public:
	GradientDialog(const QLinearGradient &gradient, const QString &name, 
		QWidget *parent = 0);
	~GradientDialog();

	QLinearGradient gradient() const {return m_gradient;}
	QString name() const {return nameEdit->text();}

private:
	QLinearGradient m_gradient;

	GradientScene *scene;
	QLineEdit *nameEdit;

private slots:
	void addColour();
	void editColour();
	void removeColour();

};

} // namespace emd


#endif