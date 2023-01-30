// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DLNFSMOUNTHELPER_H
#define DLNFSMOUNTHELPER_H

#include "abstractmounthelper.h"

DAEMONPMOUNTCONTROL_BEGIN_NAMESPACE
class DlnfsMountHelper : public AbstractMountHelper
{
public:
    explicit DlnfsMountHelper(QDBusContext *context)
        : AbstractMountHelper(context) { }

    virtual QVariantMap mount(const QString &path, const QVariantMap &opts) override;
    virtual QVariantMap unmount(const QString &path, const QVariantMap &opts) override;

private:
    bool checkDlnfsExist(const QString &path);
};
DAEMONPMOUNTCONTROL_END_NAMESPACE

#endif   // DLNFSMOUNTHELPER_H
