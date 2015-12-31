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

#ifndef EMD_DIMENSIONWIDGET_H
#define EMD_DIMENSIONWIDGET_H

#include <QtGui>
#include <QtWidgets>

namespace emd
{

class Dataset;
class LabeledSlider;

class DimensionWidget : public QWidget
{
	Q_OBJECT

public:
	DimensionWidget(Dataset *data, int index, QWidget *parent = 0);
	virtual ~DimensionWidget();

    int dimLength() const;

	void setHButtonChecked(bool checked);
	void setVButtonChecked(bool checked);
    void setPrimaryIndex(int index);
    void setSecondaryIndex(int index);
	void setSliderPosition(float min, float max);

    bool selectionEnabled() const;
    void setSelectionEnabled(bool enabled);

    int nameWidth() const;
    void setNameWidth(int width);

    int unitsWidth() const;
    void setUnitsWidth(int width);

    void getRange(unsigned int &start, unsigned int &count) const;

	//virtual void paintEvent(QPaintEvent *);
    //virtual QSize minimumSizeHint() const;

private:
    void updateSliderLabels();

private slots:
	void changeHButton(bool);
	void changeVButton(bool);
	// slider
    void updateRange();
	void changeSliderPanning(bool);

signals:
	void hButtonChanged(int, bool);
	void vButtonChanged(int, bool);
	// slider
	void primaryIndexChanged(int, int);
	void secondIndexChanged(int, int);
    void indexesChanged(int dimIndex, int primary, int secondary);
    void rangeChanged(int sliderIndex);
	void sliderPositionChanged(int, int);
	void panningChanged(int, bool);

private:
	// GUI
	QLabel m_nameLabel;
	QLabel m_unitLabel;
	QPushButton m_hButton;
	QPushButton m_vButton;
    LabeledSlider *m_slider;
	QPushButton m_integrateButton;

	// State
    const Dataset *m_data;
	int m_index;
    bool m_selectionEnabled;
};

} // namespace emd


#endif
