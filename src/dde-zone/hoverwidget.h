/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
