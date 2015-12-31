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

#include "LabeledSlider.h"

#include <qdebug.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qstyle.h>

#include "StyleConfig.h"

const QString kReferenceString("0.0000000");

namespace emd
{

LabeledSlider::LabeledSlider(int min, int max, QWidget *parent)
    : QAbstractSlider(parent),
    m_selectionMode(SelectionModeSingle),
    m_primaryIndex(min),
    m_secondaryIndex(min),
    m_draggedButton(NULL),
    m_draggingPrimary(false),
    m_draggingSecondary(false),
    m_flattening(false)
{
    this->setRange(min, max);

    // Determine the required button width for a reference string using
    // the current default font.
    QFontMetrics metrics(this->font());
    m_buttonWidth = (int) (1.1 * metrics.width(kReferenceString));
    m_buttonWidth += 4 - (m_buttonWidth % 4);

    m_sliderHeight = (int) (2.2 * metrics.height());
    m_sliderHeight += 4 - (m_sliderHeight % 4);

    this->setFixedHeight(m_sliderHeight);

    // Buttons
    m_leftButton = new QPushButton(this);
    m_leftButton->resize(m_buttonWidth, m_sliderHeight);
    connect(m_leftButton, SIGNAL(clicked()),
        this, SLOT(decrementIndex()));

    m_rightButton = new QPushButton(this);
    m_rightButton->resize(m_buttonWidth, m_sliderHeight);
    connect(m_rightButton, SIGNAL(clicked()),
        this, SLOT(incrementIndex()));

    m_primaryButton = new QPushButton(this);
    m_primaryButton->resize(m_buttonWidth, m_sliderHeight);
    m_primaryButton->installEventFilter(this);

    m_secondaryButton = new QPushButton(this);
    m_secondaryButton->resize(m_buttonWidth, m_sliderHeight / 2);
    m_secondaryButton->setVisible(false);
    m_secondaryButton->installEventFilter(this);

    updateButtonPositions();
}

/******************************* Public *******************************/

void LabeledSlider::enableFlattening(bool enabled)
{
    m_flattening = enabled;

    this->updateGeometry();
}

LabeledSlider::SelectionMode LabeledSlider::selectionMode() const
{
    return m_selectionMode;
}

void LabeledSlider::setSelectionMode(SelectionMode selectionMode)
{
    if(m_selectionMode == selectionMode)
        return;

    m_selectionMode = selectionMode;

    switch (m_selectionMode)
    {
    case emd::LabeledSlider::SelectionModeSingle:
        m_primaryButton->setVisible(true);
        m_secondaryButton->setVisible(false);
        break;
    case emd::LabeledSlider::SelectionModeVariableRange:
    case emd::LabeledSlider::SelectionModeFixedRange:
        m_primaryButton->setVisible(true);
        m_secondaryButton->setVisible(true);
        break;
    default:
        break;
    }

    this->updateGeometry();

    updateButtonPositions();
}

void LabeledSlider::setLeftText(const QString &text)
{
    m_leftButton->setText(text);
}

void LabeledSlider::setRightText(const QString &text)
{
    m_rightButton->setText(text);
}

void LabeledSlider::setPrimaryText(const QString &text)
{
    m_primaryButton->setText(text);
}

void LabeledSlider::setSecondaryText(const QString &text)
{
    m_secondaryButton->setText(text);
}

int LabeledSlider::primaryIndex() const
{
    return m_primaryIndex;
}

void LabeledSlider::setPrimaryIndex(int index)
{
    m_primaryIndex = index;

    updateButtonPositions();
}

int LabeledSlider::secondaryIndex() const
{
    return m_secondaryIndex;
}

void LabeledSlider::setSecondaryIndex(int index)
{
    m_secondaryIndex = index;

    updateButtonPositions();
}

void LabeledSlider::setIndexes(int primary, int secondary)
{
    if(primary == m_primaryIndex && secondary == m_secondaryIndex)
        return;

    m_primaryIndex = primary;
    m_secondaryIndex = secondary;

    updateButtonPositions();
}


/************************** QAbstractSlider ****************************/

int LabeledSlider::sliderPosition() const
{
    return m_primaryIndex;
}

void LabeledSlider::setSliderPosition(int position)
{
    this->setPrimaryIndex(position);
}


/****************************** QWidget *******************************/

void LabeledSlider::paintEvent(QPaintEvent *)
{
    static const QColor kBackgroundEnabledColour(COLOUR_HIGHLIGHT_1);
    static const QColor kBackgroundDisabledColour(200, 200, 200);
    static const QColor kBackgroundEdgeColour(0, 0, 0, 0);
    static const QColor kSingleTrackDarkColour(140, 180, 220);
    static const QColor kSingleTrackLightColour(230, 245, 255);
    static const QColor kDoubleTrackDarkColour(130, 130, 130);
    static const QColor kDoubleTrackLightColour(220, 220, 220);
    static const QColor kSelectionBoxColour(180, 180, 180);
	static const QColor kSelectionOutlineColour(140, 160, 190);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int height = this->height();
    int width = this->width();

    int halfHeight = height / 2;
    int quarterHeight = halfHeight / 2;
    int buttonAndAHalf = m_buttonWidth + m_buttonWidth / 2;
    int buttonAndAQuarter = m_buttonWidth + m_buttonWidth / 4;

    // Background
    painter.setPen(Qt::NoPen);

    QLinearGradient bgGrad;
    bgGrad.setStart(0, 0);
    bgGrad.setFinalStop(width, 0);
    bgGrad.setColorAt(0, kBackgroundEdgeColour);
    bgGrad.setColorAt(((float) buttonAndAHalf) / width, 
        m_selectionMode == SelectionModeSingle ? kBackgroundEnabledColour : kBackgroundDisabledColour);
    bgGrad.setColorAt(((float) (width - buttonAndAHalf)) / width, 
        m_selectionMode == SelectionModeSingle ? kBackgroundEnabledColour : kBackgroundDisabledColour);
    bgGrad.setColorAt(1, kBackgroundEdgeColour);

    painter.setBrush(QBrush(bgGrad));
    painter.drawRect(this->rect());

    // Selection box
    if(m_selectionMode == SelectionModeFixedRange || m_selectionMode == SelectionModeVariableRange)
    {
        painter.setPen(QPen(kSelectionOutlineColour));
        painter.setBrush(kSelectionBoxColour);

        painter.drawRect(m_primaryButton->pos().x() + m_buttonWidth / 2, 0, m_secondaryButton->pos().x() - m_primaryButton->pos().x(), m_sliderHeight);
    }

    //// Track lines
    //if(m_selectionMode == SelectionModeSingle)
    //{
    //    painter.setPen(QPen(kSingleTrackDarkColour, 1));
    //    painter.drawLine(QPointF(buttonAndAHalf, halfHeight - 1), QPointF(width - buttonAndAHalf, halfHeight - 1));

    //    painter.setPen(QPen(kSingleTrackLightColour, 1));
    //    painter.drawLine(QPointF(buttonAndAHalf, halfHeight), QPointF(width - buttonAndAHalf, halfHeight));
    //}
    //else
    //{
    //    painter.setPen(QPen(kDoubleTrackDarkColour, 1));
    //    painter.drawLine(QPointF(buttonAndAHalf, quarterHeight - 1), QPointF(width - buttonAndAHalf, quarterHeight - 1));
    //    painter.setPen(QPen(kDoubleTrackDarkColour, 1));
    //    painter.drawLine(QPointF(buttonAndAHalf, halfHeight + quarterHeight - 1), QPointF(width - buttonAndAHalf, halfHeight + quarterHeight - 1));

    //    painter.setPen(QPen(kDoubleTrackLightColour, 1));
    //    painter.drawLine(QPointF(buttonAndAHalf, quarterHeight), QPointF(width - buttonAndAHalf, quarterHeight));
    //    painter.drawLine(QPointF(buttonAndAHalf, halfHeight + quarterHeight), QPointF(width - buttonAndAHalf, halfHeight + quarterHeight));
    //}
}

void LabeledSlider::resizeEvent(QResizeEvent *e)
{
    QAbstractSlider::resizeEvent(e);

    m_maxX = this->width() - 2 * m_buttonWidth;

    m_rightButton->move(this->width() - m_buttonWidth, 0);

    this->updateButtonPositions();
}

bool LabeledSlider::eventFilter(QObject *receiver, QEvent *e)
{
    switch (e->type())
    {
    case QEvent::MouseButtonPress:
        if(receiver == m_primaryButton)
        {
            m_draggedButton = m_primaryButton;
            QPoint buttonPos = m_draggedButton->mapFromGlobal(((QMouseEvent*)e)->globalPos());
            m_draggedButtonPoint = buttonPos.x();
            m_draggingPrimary = true;

            if(m_selectionMode == SelectionModeFixedRange)
            {
                m_draggingSecondary = true;

                emit(panningChanged(true));
            }
        }
        else if(receiver == m_secondaryButton)
        {
            m_draggedButton = m_secondaryButton;
            QPoint buttonPos = m_draggedButton->mapFromGlobal(((QMouseEvent*)e)->globalPos());
            m_draggedButtonPoint = buttonPos.x();
            m_draggingSecondary = true;

            if(m_selectionMode == SelectionModeFixedRange)
            {
                m_draggingPrimary = true;

                emit(panningChanged(true));
            }
        }
        break;

    case QEvent::MouseMove:
        if(receiver == m_primaryButton || receiver == m_secondaryButton)
        {
            this->mouseMoveEvent((QMouseEvent*) e);
        }
        break;

    case QEvent::MouseButtonRelease:
        if(receiver == m_primaryButton || receiver == m_secondaryButton)
        {
            this->mouseReleaseEvent((QMouseEvent*) e);
        }
        break;

    default:
        break;
    } 

    return QAbstractSlider::eventFilter(receiver, e);
}

void LabeledSlider::mousePressEvent(QMouseEvent *e)
{
    QAbstractSlider::mousePressEvent(e);
}

void LabeledSlider::mouseMoveEvent(QMouseEvent *e)
{
    if(!m_draggedButton)
        return;

    QPoint globalPos = e->globalPos();
    QPoint localPos = this->mapFromGlobal(globalPos);

    int targetX = localPos.x() - m_draggedButtonPoint;
    int currentX = m_draggedButton->pos().x();
    int deltaX = targetX - currentX;

    if(m_draggingPrimary)
    {
        if(m_primaryButton->pos().x() + deltaX < m_buttonWidth)
            deltaX = m_buttonWidth - m_primaryButton->pos().x();
        else if(m_primaryButton->pos().x() + deltaX > m_maxX)
            deltaX = m_maxX - m_primaryButton->pos().x();
    }

    if(m_draggingSecondary)
    {
        if(m_secondaryButton->pos().x() + deltaX < m_buttonWidth)
            deltaX = m_buttonWidth - m_secondaryButton->pos().x();
        else if(m_secondaryButton->pos().x() + deltaX > m_maxX)
            deltaX = m_maxX - m_secondaryButton->pos().x();
    }

    if(deltaX != 0)
    {
        if(m_draggingPrimary)
        {
            m_primaryButton->move(m_primaryButton->pos() + QPoint(deltaX, 0));

            if(m_primaryButton->pos().x() > m_secondaryButton->pos().x())
                m_secondaryButton->move(m_primaryButton->pos().x(), m_secondaryButton->pos().y());
        }

        if(m_draggingSecondary)
        {
            m_secondaryButton->move(m_secondaryButton->pos() + QPoint(deltaX, 0));

            if(m_primaryButton->pos().x() > m_secondaryButton->pos().x())
                m_primaryButton->move(m_secondaryButton->pos().x(), m_primaryButton->pos().y());
        }
    }

    updateIndexes();

    e->accept();

    this->update();
}

void LabeledSlider::mouseReleaseEvent(QMouseEvent * /*e*/)
{
    m_draggedButton = NULL;
    m_draggingPrimary = false;
    m_draggingSecondary = false;

    emit(panningChanged(false));

    updateButtonPositions();
}

QSize LabeledSlider::sizeHint() const
{
    return QSize(3 * m_buttonWidth, (m_flattening && (m_selectionMode == SelectionModeSingle) ? m_sliderHeight / 2 : m_sliderHeight));
}

void LabeledSlider::updateGeometry()
{
    switch (m_selectionMode)
    {
    case emd::LabeledSlider::SelectionModeSingle:
        if(!m_flattening)
        {
            m_leftButton->resize(m_buttonWidth, m_sliderHeight);
            m_rightButton->resize(m_buttonWidth, m_sliderHeight);
            m_primaryButton->resize(m_buttonWidth, m_sliderHeight);
            this->setFixedHeight(m_sliderHeight);
        }
        else
        {
            m_leftButton->resize(m_buttonWidth, m_sliderHeight / 2);
            m_rightButton->resize(m_buttonWidth, m_sliderHeight / 2);
            m_primaryButton->resize(m_buttonWidth, m_sliderHeight / 2 + 2);
            this->setFixedHeight(m_sliderHeight / 2);
        }
        break;
    case emd::LabeledSlider::SelectionModeVariableRange:
    case emd::LabeledSlider::SelectionModeFixedRange:
        m_leftButton->resize(m_buttonWidth, m_sliderHeight);
        m_rightButton->resize(m_buttonWidth, m_sliderHeight);
        m_primaryButton->resize(m_buttonWidth, m_sliderHeight / 2);
        this->setFixedHeight(m_sliderHeight);
        break;
    default:
        break;
    }

    QAbstractSlider::updateGeometry();
}

/************************************* Private ***************************************/

void LabeledSlider::updateButtonPositions()
{
    int usableWidth = this->width() - 3 * m_buttonWidth;

    int primaryPos = m_buttonWidth 
        + QStyle::sliderPositionFromValue(this->minimum(), this->maximum(),
            m_primaryIndex, usableWidth);
    int height = 0;//(m_selectionMode == SelectionModeSingle) ? 0 : -1;

    m_primaryButton->move(primaryPos, height);

    if(m_selectionMode == SelectionModeFixedRange || m_selectionMode == SelectionModeVariableRange)
    {
        int secondaryPos = m_buttonWidth 
            + QStyle::sliderPositionFromValue(this->minimum(), this->maximum(),
                m_secondaryIndex, usableWidth);

        m_secondaryButton->move(secondaryPos, m_sliderHeight / 2);
    }

    this->update();
}

void LabeledSlider::updateIndexes()
{
    int usableWidth = this->width() - 3 * m_buttonWidth;

    int primaryIndex = QStyle::sliderValueFromPosition(this->minimum(), this->maximum(),
        m_primaryButton->pos().x() - m_buttonWidth, usableWidth);

    bool primaryChanged = (primaryIndex != m_primaryIndex);

    m_primaryIndex = primaryIndex;
    
    if(m_selectionMode != SelectionModeSingle)
    {
        int secondaryIndex = QStyle::sliderValueFromPosition(this->minimum(), this->maximum(),
            m_secondaryButton->pos().x() - m_buttonWidth, usableWidth);

        if(secondaryIndex != m_secondaryIndex)
        {
            m_secondaryIndex = secondaryIndex;

            if(primaryChanged)
            {
                emit(indexesChanged());
            }
            else
                emit(indexesChanged());
        }
        else if(primaryChanged)
            emit(indexesChanged());
    }
    else if(primaryChanged)
    {
        m_secondaryIndex = m_primaryIndex;

        emit(indexesChanged());
    }
}


/************************************** Private Slots **********************************/

void LabeledSlider::incrementIndex()
{
    if(m_secondaryIndex < this->maximum())
    {
        ++m_primaryIndex;
        ++m_secondaryIndex;

        emit(indexesChanged());

        updateButtonPositions();
    }
    else if(m_selectionMode != SelectionModeFixedRange && m_primaryIndex < this->maximum())
    {
        ++m_primaryIndex;

        emit(indexesChanged());

        updateButtonPositions();
    }
}

void LabeledSlider::decrementIndex()
{
    if(m_primaryIndex > this->minimum())
    {
        --m_primaryIndex;
        --m_secondaryIndex;

        emit(indexesChanged());

        updateButtonPositions();
    }
    else if(m_selectionMode == SelectionModeVariableRange && m_secondaryIndex > this->minimum())
    {
        --m_secondaryIndex;

        emit(indexesChanged());

        updateButtonPositions();
    }
}

} // namespace emd
