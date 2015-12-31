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

#ifndef EMD_LABELEDSLIDER_H
#define EMD_LABELEDSLIDER_H

#include <qabstractslider.h>

class QPushButton;

namespace emd
{

class LabeledSlider : public QAbstractSlider
{
    Q_OBJECT

public:
    enum SelectionMode {
        SelectionModeSingle,
        SelectionModeVariableRange,
        SelectionModeFixedRange
    };

    LabeledSlider(int min, int max, QWidget *parent = 0);

    void enableFlattening(bool enabled);

    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode selectionMode);

    void setLeftText(const QString &text);
    void setRightText(const QString &text);
    void setPrimaryText(const QString &text);
    void setSecondaryText(const QString &text);

    int primaryIndex() const;
    void setPrimaryIndex(int index);

    int secondaryIndex() const;
    void setSecondaryIndex(int index);

    void setIndexes(int primary, int secondary);

    // From QAbstractSlider
    virtual int sliderPosition() const;
    virtual void setSliderPosition(int position);

    // From QWidget
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    virtual bool eventFilter(QObject *receiver, QEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

    virtual QSize sizeHint() const;
    
    virtual void updateGeometry();

private:
    void updateButtonPositions();
    void updateIndexes();

private slots:
    void incrementIndex();
    void decrementIndex();

signals:
	void indexesChanged();
	void panningChanged(bool panning);

private:
    QPushButton *m_leftButton;
    QPushButton *m_rightButton;
    QPushButton *m_primaryButton;
    QPushButton *m_secondaryButton;

    int m_buttonWidth;
    int m_sliderHeight;
    int m_maxX;

    bool m_flattening;
    
    SelectionMode m_selectionMode;

    int m_primaryIndex;
    int m_secondaryIndex;

    // Current state
    QPushButton *m_draggedButton;
    int m_draggedButtonPoint;
    bool m_draggingPrimary;
    bool m_draggingSecondary;
};

} // namespace emd

#endif
