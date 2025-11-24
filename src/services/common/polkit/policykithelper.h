// SPDX-FileCopyrightText: 2022 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICECOMMON_POLICYKITHELPER_H
#define SERVICECOMMON_POLICYKITHELPER_H

#include <QObject>
#include <polkit-qt6-1/PolkitQt1/Authority>

namespace ServiceCommon {

class PolicyKitHelper
{
public:
    static PolicyKitHelper *instance();

    bool checkAuthorization(const QString &actionId, const QString &appBusName);

private:
    PolicyKitHelper();
    ~PolicyKitHelper();

    Q_DISABLE_COPY(PolicyKitHelper)
};

}  // namespace ServiceCommon

#endif   // SERVICECOMMON_POLICYKITHELPER_H
