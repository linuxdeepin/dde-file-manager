/*
 * Copyright (C) 2016 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
*/

#ifndef MOVETOTRASHCONFLICTDIALOG_H
#define MOVETOTRASHCONFLICTDIALOG_H

#include <QWidget>
#include "ddialog.h"
#include "interfaces/durl.h"

DWIDGET_USE_NAMESPACE

class MoveToTrashConflictDialog : public DDialog
{
    Q_OBJECT
public:
    explicit MoveToTrashConflictDialog(QWidget *parent = 0,
                                       const DUrlList& urls = DUrlList());

signals:

public slots:
};

#endif // MOVETOTRASHCONFLICTDIALOG_H
