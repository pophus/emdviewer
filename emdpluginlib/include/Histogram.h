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

#ifndef EMD_HISTOGRAM_H
#define EMD_HISTOGRAM_H

#include <QtWidgets>

#include "HistogramScene.h"

namespace emd
{

const int HISTOGRAM_WIDTH = 205;
const int HISTOGRAM_HEIGHT = 120;

class DataGroup;
class Frame;
class HistogramView;
class NumberRangeWidget;

class Histogram : public QWidget
{
	Q_OBJECT

public:
	Histogram(QWidget *parent = 0);
	~Histogram();

    void reset(const DataGroup *dataGroup);

	void setLimits(const float &lower, const float &upper);
    void setValues(float lower, float upper);
    void setFloatType(bool floatType);

public slots:
	void setImage(QImage *image, float min, float max);
    void setColourMap(const QString &mapName);

private slots:
    void setHistogramSize(int width, int height);
    void setAutoScaling(bool);
    void changeScalingValuesFromBoxes(float, float);
    void changeScalingValuesFromHistogram(float, float);

signals:
    void scalingLocked(bool locked);
	void scalingValuesChanged(float lower, float upper);
    void sizeChanged(int width, int height);

private:
	HistogramScene m_scene;
	HistogramView *m_view;
    NumberRangeWidget *m_scalingBoxes;
    QPushButton *m_autoScalingButton;
    bool m_hasFirstImage;
};

}

#endif