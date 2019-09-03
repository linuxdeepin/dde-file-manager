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

#include <DIconButton>

DWIDGET_USE_NAMESPACE

class DFMActionButton : public DIconButton
{
    Q_OBJECT

public:
    explicit DFMActionButton(QWidget *parent = nullptr);

    void setAction(QAction *action);
    QAction *action() const;
};

#endif // DFMACTIONBUTTON_H
