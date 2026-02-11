// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BURNSIGNALMANAGER_H
#define BURNSIGNALMANAGER_H

#include "dfmplugin_burn_global.h"

#include <QObject>

DPBURN_BEGIN_NAMESPACE

class BurnSignalManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BurnSignalManager)

public:
    static BurnSignalManager *instance();

signals:
    void activeTaskDialog();

private:
    explicit BurnSignalManager(QObject *parent = nullptr);
};

DPBURN_END_NAMESPACE

#endif   // BURNSIGNALMANAGER_H
