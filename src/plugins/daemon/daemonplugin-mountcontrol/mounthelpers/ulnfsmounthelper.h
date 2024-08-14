// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UlnfsMountHelper_H
#define UlnfsMountHelper_H

#include "abstractmounthelper.h"

DAEMONPMOUNTCONTROL_BEGIN_NAMESPACE
class UlnfsMountHelper : public AbstractMountHelper
{
public:
    explicit UlnfsMountHelper(QDBusContext *context)
        : AbstractMountHelper(context) {}

    virtual QVariantMap mount(const QString &path, const QVariantMap &opts) override;
    virtual QVariantMap unmount(const QString &path, const QVariantMap &opts) override;

private:
    bool loadKernelModule(const QString &moduleName);
    bool checkLnfsExist(const QString &path);
    bool isModuleLoaded(const QString &path);
    int parseErrorCodeByMsg(int res);
};
DAEMONPMOUNTCONTROL_END_NAMESPACE

#endif   // UlnfsMountHelper_H
