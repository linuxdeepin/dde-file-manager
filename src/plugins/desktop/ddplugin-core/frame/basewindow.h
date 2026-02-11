// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include "ddplugin_core_global.h"

#include <QWidget>

DDPCORE_BEGIN_NAMESPACE

class BaseWindow : public QWidget
{
    Q_OBJECT
public:
    explicit BaseWindow(QWidget *parent = nullptr);
    void init();
signals:

public slots:
};

typedef QSharedPointer<BaseWindow> BaseWindowPointer;

DDPCORE_END_NAMESPACE

#endif // BASEWINDOW_H
