/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "pushbuttonlist.h"
#include <QFontMetrics>

PushButtonList::PushButtonList(HoverWidget *parent, const bool isRight, const bool isBottom, QLabel *selectedText)
{
    // this variable if so important that I have to new it before everything else
    m_zoneInterface = new ZoneInterface("com.deepin.daemon.Zone", "/com/deepin/daemon/Zone",
                                        QDBusConnection::sessionBus(), this);

    m_ParentItem = parent;
    m_ParentItem->setContentsMargins(0, 0, 0, 0);
    m_VLayout = new QVBoxLayout(m_ParentItem);
    m_VLayout->setSpacing(0);
    m_VLayout->setContentsMargins(0, 0, 0, 0);

    m_SelectedText = selectedText;

    m_IsRight = isRight;
    m_IsBottom = isBottom;

    // this variable is used to record where button has clicked to help change image correctly
    buttonHasClicked = false;

    // this variable is used to record the max width of QPushButton's text
    m_MaxTextWidth = 0;

    if (!isRight && !isBottom) {
        corner = TopLeft;
    } else if (isRight && !isBottom) {
        corner = TopRight;
    } else if (!isRight && isBottom) {
        corner = BottomLeft;
    } else {
        corner = BottomRight;
    }
}

PushButtonList::~PushButtonList()
{
}


void PushButtonList::addButton(QString buttonName, QString actionStr, bool lastAddedButton)
{
    QPushButton *theButton = new QPushButton(m_ParentItem);
    theButton->setText(buttonName);
    m_IsBottom ? m_ButtonList.prepend(theButton) : m_ButtonList.append(theButton);
    m_IsBottom ? m_ActionList.prepend(actionStr) : m_ActionList.append(actionStr);

    if (lastAddedButton) {
        alignTheButtonAccordingToTopOrBottom();

        highlightTheButtonTextWhenAppInit();

        handleSignalAndSlots();
    }
}

void PushButtonList::alignTheButtonAccordingToTopOrBottom()
{
    for (int i = 0, length = m_ButtonList.length(); i < length; i++) {
        QPushButton *tmpButton = m_ButtonList[i];
        QFont font(tmpButton->font());
        font.setPixelSize(BUTTON_TEXT_SIZE);
        tmpButton->setFont(font);
        QFontMetrics metrics(tmpButton->font());
        int pushButtonWidth = metrics.boundingRect(tmpButton->text()).width() + BUTTON_PADDINGS;
        m_MaxTextWidth = (pushButtonWidth > m_MaxTextWidth ? pushButtonWidth : m_MaxTextWidth);
        tmpButton->setFixedSize(pushButtonWidth, BUTTON_HEIGHT);
        tmpButton->setStyleSheet("QPushButton{color:rgba(255,255,255,0.5);background:transparent;border:none}"
                                 "QPushButton:hover{background:rgba(0,0,0,0.5);border:none;border-radius:4px}");
        tmpButton->setFocusPolicy(Qt::NoFocus);
        tmpButton->setCursor(Qt::PointingHandCursor);
        m_IsRight ? m_VLayout->addWidget(tmpButton, 0, Qt::AlignRight) : m_VLayout->addWidget(tmpButton, 0, Qt::AlignLeft);
    }
}

void PushButtonList::highlightTheButtonTextWhenAppInit()
{
    QString theAction;
    switch (corner) {
    case TopLeft:
        theAction = m_zoneInterface->TopLeftAction();
        break;
    case TopRight:
        theAction = m_zoneInterface->TopRightAction();
        break;
    case BottomLeft:
        theAction = m_zoneInterface->BottomLeftAction();
        break;
    case BottomRight:
        theAction = m_zoneInterface->BottomRightAction();
        break;
    }

    for (int i = 0, length = m_ActionList.length(); i < length; i++) {
        if (theAction == m_ActionList[i]) {
            m_SelectedText->setText(m_ButtonList[i]->text());
            m_ButtonList[i]->setStyleSheet("QPushButton{color:rgba(1,189,255,1);background:transparent;border:none}"
                                           "QPushButton:hover{background:rgba(0,0,0,0.5);border:none;border-radius:4px}");
        }
    }
}

void PushButtonList::handleSignalAndSlots()
{
    for (int i = 0, length = m_ButtonList.length(); i < length; i++) {
        connect(m_ButtonList[i], &QPushButton::clicked, [ = ] {
            this->onButtonClicked(m_ActionList[i], i);
        });
    }
}

void PushButtonList::onButtonClicked(QString theAction, int index)
{
    switch (corner) {
    case TopLeft:
        m_zoneInterface->SetTopLeft(theAction);
        break;
    case TopRight:
        m_zoneInterface->SetTopRight(theAction);
        break;
    case BottomLeft:
        m_zoneInterface->SetBottomLeft(theAction);
        break;
    case BottomRight:
        m_zoneInterface->SetBottomRight(theAction);
        break;
    }

    m_SelectedText->setText(m_ButtonList[index]->text());
    buttonHasClicked = true;

    for (int j = 0, length = m_ButtonList.length(); j < length; j++) {
        if (m_ActionList[j] == theAction)
            m_ButtonList[j]->setStyleSheet("QPushButton{color:rgba(1,189,255,1);background:transparent;border:none}"
                                           "QPushButton:hover{background:rgba(0,0,0,0.5);border:none;border-radius:4px}");
        else
            m_ButtonList[j]->setStyleSheet("QPushButton{color:rgba(255,255,255,0.5);background:transparent;border:none}"
                                           "QPushButton:hover{background:rgba(0,0,0,0.5);border:none;border-radius:4px}");
    }
    m_ParentItem->setVisible(false);

    qDebug() << m_zoneInterface->TopLeftAction();
    qDebug() << m_zoneInterface->TopRightAction();
    qDebug() << m_zoneInterface->BottomLeftAction();
    qDebug() << m_zoneInterface->BottomRightAction();
}
