// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dlocalfilehandler.h"
#include "private/dfilehandler_p.h"
#include "../controllers/vaultcontroller.h"
#include "dfmstandardpaths.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "utils/singleton.h"

#include <QObject>
#include <QDir>
#include <QDateTime>
#include <QTemporaryFile>

#include <unistd.h>
#include <utime.h>
#include <cstdio>

DFM_BEGIN_NAMESPACE

DLocalFileHandler::DLocalFileHandler()
{

}

bool DLocalFileHandler::exists(const DUrl &url)
{
    Q_ASSERT(url.isLocalFile());

    QFileInfo info(url.toLocalFile());

    return info.exists() || info.isSymLink();
}

bool DLocalFileHandler::touch(const DUrl &url)
{
    Q_ASSERT(url.isLocalFile());

    QFile file(url.toLocalFile());

    if (file.exists())
        return false;

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.close();
        return true;
    }

    Q_D(DFileHandler);

    d->setErrorString(file.errorString());

    return false;
}

bool DLocalFileHandler::mkdir(const DUrl &url)
{
    Q_ASSERT(url.isLocalFile());

    Q_D(DFileHandler);

    if (QDir::current().mkdir(url.toLocalFile())) {
        return true;
    }

    d->setErrorString(QString::fromLocal8Bit(strerror(errno)));

    return false;
}

bool DLocalFileHandler::mkpath(const DUrl &url)
{
    Q_ASSERT(url.isLocalFile());

    if (QDir::current().mkpath(url.toLocalFile())) {
        return true;
    }

    Q_D(DFileHandler);

    d->setErrorString(QString::fromLocal8Bit(strerror(errno)));

    return false;
}

bool DLocalFileHandler::link(const QString &path, const DUrl &linkUrl)
{
    Q_ASSERT(linkUrl.isLocalFile());

    if (::symlink(path.toLocal8Bit().constData(), linkUrl.toLocalFile().toLocal8Bit().constData()) == 0)
        return true;

    Q_D(DFileHandler);

    d->setErrorString(QString::fromLocal8Bit(strerror(errno)));

    return false;
}

bool DLocalFileHandler::remove(const DUrl &url)
{
    Q_ASSERT(url.isLocalFile());

    if (url.path().contains(MTP_STAGING) && url.path().startsWith(MOBILE_ROOT_PATH))
        fileSignalManager->requestCloseMediaInfo(url.path());

    if (::remove(url.toLocalFile().toLocal8Bit()) == 0)
        return true;

    Q_D(DFileHandler);

    d->setErrorString(QString::fromLocal8Bit(strerror(errno)));

    return false;
}

bool DLocalFileHandler::rmdir(const DUrl &url)
{
    Q_ASSERT(url.isLocalFile());

    if (::rmdir(url.toLocalFile().toLocal8Bit()) == 0)
        return true;

    Q_D(DFileHandler);

    d->setErrorString(QString::fromLocal8Bit(strerror(errno)));

    return false;
}

bool DLocalFileHandler::rename(const DUrl &url, const DUrl &newUrl)
{
    Q_ASSERT(url.isLocalFile());
    Q_ASSERT(newUrl.isLocalFile());
    Q_D(DFileHandler);

    const QByteArray &source_file = url.toLocalFile().toLocal8Bit();
    const QByteArray &target_file = newUrl.toLocalFile().toLocal8Bit();

    if (::rename(source_file.constData(), target_file.constData()) == 0)
        return true;

    d->setErrorString(QString::fromLocal8Bit(strerror(errno)));

    return false;
}

bool DLocalFileHandler::setPermissions(const DUrl &url, QFileDevice::Permissions permissions)
{
    Q_ASSERT(url.isLocalFile());

    QFile file(url.toLocalFile());

    if (file.setPermissions(permissions))
        return true;

    Q_D(DFileHandler);

    d->setErrorString(file.errorString());

    return false;
}

bool DLocalFileHandler::setFileTime(const DUrl &url, const QDateTime &accessDateTime, const QDateTime &lastModifiedTime)
{
    Q_ASSERT(url.isLocalFile());

    utimbuf buf = {
        .actime = accessDateTime.toTime_t(),
        .modtime = lastModifiedTime.toTime_t()
    };

    if (::utime(url.toLocalFile().toLocal8Bit(), &buf) == 0) {
        return true;
    }

    Q_D(DFileHandler);

    d->setErrorString(QString::fromLocal8Bit(strerror(errno)));

    return false;
}

DFM_END_NAMESPACE
