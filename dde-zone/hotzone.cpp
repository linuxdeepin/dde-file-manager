/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "hotzone.h"

#include <QFontMetrics>

HotZone::HotZone(QWidget *parent, bool isRight, bool isBottom): QWidget(parent)
{

    initSeveralVar(isRight, isBottom);

    initItemAtCorrectCorner(parent);

    initImageAndTextAtCorrectCorner();

    initButtonColumnAtCorrectCorner();

    initAnimationCorrectly();

    initConnects();
}

HotZone::~HotZone()
{
}

void HotZone::initSeveralVar(bool isRight, bool isBottom)
{
    // isright and isbottom are different from isRight and isBottom,they are used to record the position of this Widget,and then to help define those slots correctly
    m_isRight = isRight;
    m_isBottom = isBottom;

    if (!isRight && !isBottom) {
        corner = TopLeft;
    } else if (isRight && !isBottom) {
        corner = TopRight;
    } else if (!isRight && isBottom) {
        corner = BottomLeft;
    } else {
        corner = BottomRight;
    }

    // this variable is used to record m_animation has started or not
    m_hasAnimationRun = true;

    // it's effect is as it's name
    m_mouseIsEnterFromButtonColumnAfterButtonClick = false;
}

void HotZone::initItemAtCorrectCorner(QWidget *mainWindow)
{
    // m_mainItem is the wrapper of images and text and buttons
    m_mainItem = new QWidget(mainWindow);
    m_mainItem->setFixedSize(WRAPPER_SIZE, WRAPPER_SIZE);

    // calculate the size of the screen
    int mainWidth = mainWindow->width();
    int mainHeight = mainWindow->height();

    switch (corner) {
    case TopLeft:
        m_mainItem->move(0, MAIN_ITEM_TOP_MARGIN);
        break;
    case TopRight:
        m_mainItem->move(mainWidth - WRAPPER_SIZE, MAIN_ITEM_TOP_MARGIN);
        break;
    case BottomLeft:
        m_mainItem->move(0, mainHeight - WRAPPER_SIZE);
        break;
    case BottomRight:
        m_mainItem->move(mainWidth - WRAPPER_SIZE, mainHeight - WRAPPER_SIZE);
        break;
    }
}

void HotZone::initImageAndTextAtCorrectCorner()
{
    // prepare the grayBlurImg.png source
    m_imageLabel = new QLabel(m_mainItem);
    m_pixmap.load(":/images/normal.png");

    // matrix is used to arrange rotation of blurImg
    m_matrix = new QMatrix;

    // initial m_selectedActionTextLabel and arrange it's alignment
    m_selectedActionTextLabel = new QLabel(m_mainItem);
    m_selectedActionTextLabel->setFixedWidth(SELECTED_LABEL_FIXED_WIDTH);
    m_selectedActionTextLabel->setStyleSheet("QLabel{font-size:13px;color:#ffffff}");
    m_isRight ? m_selectedActionTextLabel->setAlignment(Qt::AlignRight) : m_selectedActionTextLabel->setAlignment(Qt::AlignLeft);

    switch (corner) {
    case TopLeft:
        m_imageLabel->move(0, 0);
        m_selectedActionTextLabel->move(TEXT_LEFT_OR_RIGHT_MARGIN, TEXT_TOP_MARGIN);
        m_matrix->rotate(0);
        break;
    case TopRight:
        m_imageLabel->move(WRAPPER_SIZE - IMAGE_SIZE, 0);
        m_selectedActionTextLabel->move(WRAPPER_SIZE - TEXT_LEFT_OR_RIGHT_MARGIN - SELECTED_LABEL_FIXED_WIDTH, TEXT_TOP_MARGIN);
        m_matrix->rotate(90);
        break;
    case BottomLeft:
        m_imageLabel->move(0, WRAPPER_SIZE - IMAGE_SIZE);
        m_selectedActionTextLabel->move(TEXT_LEFT_OR_RIGHT_MARGIN, WRAPPER_SIZE - TEXT_BOTTOM_DISTANCE);
        m_matrix->rotate(270);
        break;
    case BottomRight:
        m_imageLabel->move(WRAPPER_SIZE - IMAGE_SIZE, WRAPPER_SIZE - IMAGE_SIZE);
        m_selectedActionTextLabel->move(WRAPPER_SIZE - TEXT_LEFT_OR_RIGHT_MARGIN - SELECTED_LABEL_FIXED_WIDTH, WRAPPER_SIZE - TEXT_BOTTOM_DISTANCE);
        m_matrix->rotate(180);
        break;
    }

    // with rotation,show grayBlurImg.png
    m_imageLabel->setPixmap(m_pixmap.transformed(*m_matrix));
}

void HotZone::initButtonColumnAtCorrectCorner()
{
    m_hoverItem = new HoverWidget(m_mainItem);
    m_hoverItem->resize(IMAGE_SIZE, IMAGE_SIZE);
    m_ButtonColumnItem = new HoverWidget(m_mainItem);

    m_pushButtonList = new PushButtonList(m_ButtonColumnItem, m_isRight, m_isBottom, m_selectedActionTextLabel);
    m_pushButtonList->setProperty("buttonHasClicked", true);

    switch (corner) {
    case TopLeft:
        m_hoverItem->move(0, 0);
        break;
    case TopRight:
        m_hoverItem->move(WRAPPER_SIZE - IMAGE_SIZE, 0);
        break;
    case BottomLeft:
        m_hoverItem->move(0, WRAPPER_SIZE - IMAGE_SIZE);
        break;
    case BottomRight:
        m_hoverItem->move(WRAPPER_SIZE - IMAGE_SIZE, WRAPPER_SIZE - IMAGE_SIZE);
        break;
    }
}

void HotZone::addButtons(QStringList buttonNames, QStringList actionStrs)
{
    for (int i = 0, length = buttonNames.length(); i < length; i++) {
        addButton(buttonNames[i], actionStrs[i], i == (length - 1));
    }
}

void HotZone::addButton(QString buttonName, QString actionStr, bool lastAddedButton)
{
    m_pushButtonList->addButton(buttonName, actionStr, lastAddedButton);

    if (lastAddedButton) {
        // m_ButtonsNum is used to record the number of QPushButton* , to calculate m_ButtonColumnBottomDistance.
        m_ButtonsNum = m_pushButtonList->m_ActionList.length();
        // 35 is offset to make m_ButtonColumnItem's bottom-margin satisfy the design requirment.
        m_ButtonColumnBottomDistance = m_ButtonsNum * BUTTON_HEIGHT + 35;
        // 30 is offset to make m_ButtonColumnItem's right-margin satisfy the design requirment.
        m_ButtonColumnRightDistance = m_pushButtonList->m_MaxTextWidth + 30;

        rePositionButtonColumn(m_ButtonColumnBottomDistance, m_ButtonColumnRightDistance);

        reSetAnimation();
    }
}

void HotZone::reSetAnimation()
{
    m_endPoint = m_ButtonColumnItem->pos();

    m_startPoint.setX(m_endPoint.x());
    m_startPoint.setY(m_endPoint.y() + ((int)m_isBottom * 2 - 1)*ANIMATION_MOVE_DISTANCE);
    m_animation->setStartValue(m_startPoint);
    m_animation->setEndValue(m_endPoint);
}

void HotZone::rePositionButtonColumn(int BottomOffset, int RightOffset)
{
    switch (corner) {
    case TopLeft:
        m_ButtonColumnItem->move(BUTTON_COLUMN_LEFT_DISTANCE, BUTTON_COLUMN_TOP_DISTANCE);
        break;
    case TopRight:
        m_ButtonColumnItem->move(WRAPPER_SIZE - RightOffset, BUTTON_COLUMN_TOP_DISTANCE);
        break;
    case BottomLeft:
        m_ButtonColumnItem->move(BUTTON_COLUMN_LEFT_DISTANCE, WRAPPER_SIZE - BottomOffset);
        break;
    case BottomRight:
        m_ButtonColumnItem->move(WRAPPER_SIZE - RightOffset, WRAPPER_SIZE - BottomOffset);
        break;
    }
}

void HotZone::initAnimationCorrectly()
{
    m_endPoint = m_ButtonColumnItem->pos();

    m_startPoint.setX(m_endPoint.x());
    m_startPoint.setY(m_endPoint.y() + ((int)m_isBottom * 2 - 1)*ANIMATION_MOVE_DISTANCE);

    m_animation = new QPropertyAnimation(m_ButtonColumnItem, "pos", m_mainItem);
    m_animation->setDuration(100);
    m_animation->setStartValue(m_startPoint);
    m_animation->setEndValue(m_endPoint);

    m_ButtonColumnItem->setVisible(false);
}

void HotZone::initConnects()
{
    connect(m_hoverItem, SIGNAL(mouseHasEntered()), this, SLOT(onMouseHasEntered()));
    connect(m_hoverItem, SIGNAL(mouseHasLeaved()), this, SLOT(onMouseHasLeaved()));

    connect(m_ButtonColumnItem, SIGNAL(mouseHasLeaved()), this, SLOT(onMouseHasLeavedTheColumnItem()));
}

void HotZone::onMouseHasEntered()
{
    m_pixmap.load(":/images/hover.png");
    m_imageLabel->setPixmap(m_pixmap.transformed(*m_matrix));
    if (!m_mouseIsEnterFromButtonColumnAfterButtonClick) {
        if (m_hasAnimationRun) {
            m_ButtonColumnItem->setHidden(false);
            m_ButtonColumnItem->raise();
            m_animation->start();
        } else {
            m_ButtonColumnItem->setHidden(false);
            m_ButtonColumnItem->raise();
        }
    }
}

void HotZone::onMouseHasLeaved()
{
    QPoint p = m_ButtonColumnItem->mapFromGlobal(QCursor::pos());
    if (m_ButtonColumnItem->rect().contains(p)) {
        if (m_pushButtonList->buttonClickStatus()) {
            m_pixmap.load(":/images/normal.png");
            m_imageLabel->setPixmap(m_pixmap.transformed(*m_matrix));
            m_pushButtonList->setButtonClickStatus(false);
        } else {
            m_hasAnimationRun = false;
            m_pixmap.load(":/images/hover.png");
            m_imageLabel->setPixmap(m_pixmap.transformed(*m_matrix));
        }
    } else {
        m_ButtonColumnItem->setHidden(true);
        m_hasAnimationRun = true;
        m_pushButtonList->setButtonClickStatus(false);
        m_pixmap.load(":/images/normal.png");
        m_imageLabel->setPixmap(m_pixmap.transformed(*m_matrix));
    }
    m_mouseIsEnterFromButtonColumnAfterButtonClick = false;
}

void HotZone::onMouseHasLeavedTheColumnItem()
{
    QPoint p = m_hoverItem->mapFromGlobal(QCursor::pos());
    if (m_hoverItem->rect().contains(p)) {
        m_hasAnimationRun = false;
        m_mouseIsEnterFromButtonColumnAfterButtonClick = true;
        m_pixmap.load(":/images/hover.png");
        m_imageLabel->setPixmap(m_pixmap.transformed(*m_matrix));
    } else {
        m_ButtonColumnItem->setVisible(false);
        m_hasAnimationRun = true;
        m_pushButtonList->setButtonClickStatus(false);
        m_mouseIsEnterFromButtonColumnAfterButtonClick = false;
        m_pixmap.load(":/images/normal.png");
        m_imageLabel->setPixmap(m_pixmap.transformed(*m_matrix));
    }
}

