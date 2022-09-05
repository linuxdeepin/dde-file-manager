// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HOVERWIDGET_H
#define HOVERWIDGET_H

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>

class HoverWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HoverWidget(QWidget *parent = nullptr);
    ~HoverWidget();

    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
signals:
    void mouseHasEntered();
    void mouseHasLeaved();
};

#endif // HOVERWIDGET_H
