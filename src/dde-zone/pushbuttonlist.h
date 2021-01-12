/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PUSHBUTTONLISTITEM_H
#define PUSHBUTTONLISTITEM_H

#include "hoverwidget.h"
#include "dbus/dbuszone.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QList>
#include <QLabel>

class PushButtonList : public QPushButton
{
    Q_OBJECT
public:
    PushButtonList(HoverWidget *parent, const bool isRight, const bool isBottom, QLabel *selectedText);
    ~PushButtonList();

    Q_PROPERTY(bool buttonClickStatus READ buttonClickStatus WRITE setButtonClickStatus)
    bool buttonClickStatus() const
    {
        return buttonHasClicked;
    }
    void setButtonClickStatus(bool btnClicked)
    {
        buttonHasClicked = btnClicked;
    }

    void addButton(QString buttonName, QString actionStr, bool lastAddedButton);

    ZoneInterface *m_zoneInterface;
    QStringList m_ActionList;
    int m_MaxTextWidth;
private slots:
    void onButtonClicked(QString theAction, int index);

private:
    void alignTheButtonAccordingToTopOrBottom();
    void highlightTheButtonTextWhenAppInit();
    void handleSignalAndSlots();

    HoverWidget *m_ParentItem;
    QVBoxLayout *m_VLayout;
    QLabel *m_SelectedText;

    QList < QPushButton * > m_ButtonList;

    enum Corner{
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    } corner;

    bool m_IsRight;
    bool m_IsBottom;

    // this variable is used to help mouse area work properly
    bool buttonHasClicked;

    // fixed button height
    const int BUTTON_HEIGHT = 24;
    const int BUTTON_PADDINGS = 12;

    const int BUTTON_TEXT_SIZE = 12;
};

#endif // PUSHBUTTONLISTITEM_H
