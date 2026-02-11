// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPGRADELOCKER_H
#define UPGRADELOCKER_H

#include <QString>
#include <QSharedMemory>

namespace dfm_upgrade {

class UpgradeLocker
{
public:
    explicit UpgradeLocker();
    ~UpgradeLocker();
    bool isLock();
protected:
    QSharedMemory sharedMemory;
};

}

#endif // UPGRADELOCKER_H
