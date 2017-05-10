/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef DFMACTIONBUTTON_H
#define DFMACTIONBUTTON_H

#include <QAbstractButton>

class DFMActionButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit DFMActionButton(QWidget *parent = 0);

    void setAction(QAction *action);
    QAction *action() const;

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
};

#endif // DFMACTIONBUTTON_H
