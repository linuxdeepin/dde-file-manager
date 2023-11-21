// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef POLICYKITHELPER_H
#define POLICYKITHELPER_H

#include "daemonplugin_core_global.h"

#include <QObject>
#include <polkit-qt5-1/PolkitQt1/Authority>

DAEMONPCORE_BEGIN_NAMESPACE

class PolicyKitHelper : public QObject
{
    Q_OBJECT
public:
    static PolicyKitHelper *instance();

    bool checkAuthorization(const QString &actionId, const QString &appBusName);

private:
    explicit PolicyKitHelper(QObject *parent = nullptr);
    ~PolicyKitHelper();

    Q_DISABLE_COPY(PolicyKitHelper)
};

DAEMONPCORE_END_NAMESPACE

#endif   // POLICYKITHELPER_H
