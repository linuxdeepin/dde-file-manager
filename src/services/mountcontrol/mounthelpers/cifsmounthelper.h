// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CIFSMOUNTHELPER_H
#define CIFSMOUNTHELPER_H

#include "abstractmounthelper.h"

SERVICEMOUNTCONTROL_BEGIN_NAMESPACE
class CifsMountHelperPrivate;
class CifsMountHelper : public AbstractMountHelper
{
    enum MountStatus {
        kOkay,
        kAlreadyMounted = kOkay,
        kNotOwner,
        kNotCifs,
        kNotExist,
        kNotMountByDaemon,
    };

public:
    explicit CifsMountHelper(QDBusContext *context);

    virtual QVariantMap mount(const QString &path, const QVariantMap &opts) override;
    virtual QVariantMap unmount(const QString &path, const QVariantMap &opts) override;

    void cleanMountPoint();

private:
    MountStatus checkMount(const QString &path, QString &mpt);
    QString generateMountPath(const QString &address);
    QString mountRoot();
    QString decryptPasswd(const QString &passwd);
    uint invokerUid();
    std::string convertArgs(const QVariantMap &opts);
    bool checkAuth();
    bool mkdir(const QString &path);
    bool rmdir(const QString &path);
    bool mkdirMountRootPath();

    QScopedPointer<CifsMountHelperPrivate> d;
};
SERVICEMOUNTCONTROL_END_NAMESPACE

#endif   // CIFSMOUNTHELPER_H
