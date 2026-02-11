// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCKDEVHELPER_H
#define BLOCKDEVHELPER_H

#include "diskencrypt_global.h"

#include <dfm-mount/dmount.h>

typedef QSharedPointer<dfmmount::DBlockDevice> DevPtr;

FILE_ENCRYPT_BEGIN_NS
namespace blockdev_helper {
quint64 devDeviceSize(const QString &phyDev);
quint64 devBlockSize(const QString &phyDev);
DevPtr createDevPtr(const QString &dev);
DevPtr createDevPtr2(const QString &objPath);
QString resolveDevObjPath(const QString &source);
int devCryptVersion(const QString &dev);
QString getUSecName(const QString &dmDev);

enum CryptVersion {
    kNotEncrypted,
    kVersionLUKS1,
    kVersionLUKS2,
    kVersionLUKSUnknown,
    kVersionUnknown = 10000,
};   // enum CryptVersion
}   // namespace blockdev_helper
FILE_ENCRYPT_END_NS

#endif   // BLOCKDEVHELPER_H
