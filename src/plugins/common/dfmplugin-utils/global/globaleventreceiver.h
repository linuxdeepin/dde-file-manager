// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALEVENTRECEIVER_H
#define GLOBALEVENTRECEIVER_H

#include "dfmplugin_utils_global.h"

#include <QObject>
#include <QUrl>

namespace dfmplugin_utils {

class GlobalEventReceiver : public QObject
{
    Q_OBJECT

public:
    explicit GlobalEventReceiver(QObject *parent = nullptr);
    ~GlobalEventReceiver();

    void initEventConnect();

public slots:
    void handleOpenAsAdmin(const QUrl &url);
};

}

#endif   // GLOBALEVENTRECEIVER_H
