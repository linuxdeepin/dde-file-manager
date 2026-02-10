// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALSIGNALMANAGER_H
#define OPTICALSIGNALMANAGER_H

#include "dfmplugin_optical_global.h"

#include <QObject>

namespace dfmplugin_optical {

class OpticalSignalManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OpticalSignalManager)

public:
    static OpticalSignalManager *instance();

signals:
    void discUnmounted(const QUrl &url);

private:
    explicit OpticalSignalManager(QObject *parent = nullptr);
};

}

#endif   // OPTICALSIGNALMANAGER_H
