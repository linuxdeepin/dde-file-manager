/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AUDITLOGJOB_H
#define AUDITLOGJOB_H

#include "dfmplugin_burn_global.h"

#include <QUrl>
#include <QThread>
#include <QDBusInterface>
#include <QFileInfo>

namespace dfmplugin_burn {

class AbstractAuditLogJob : public QThread
{
    Q_OBJECT
public:
    explicit AbstractAuditLogJob(QObject *parent = nullptr);
    virtual ~AbstractAuditLogJob() override {}

protected:
    void run() override;
    virtual void doLog(QDBusInterface &interface) = 0;

    QFileInfoList localFileInfoList(const QString &path) const;
    QFileInfoList localFileInfoListRecursive(const QString &path) const;
};

class CopyFromDiscAuditLog : public AbstractAuditLogJob
{
public:
    CopyFromDiscAuditLog(const QList<QUrl> &srcList, const QList<QUrl> &destList, QObject *parent = nullptr);

protected:
    void doLog(QDBusInterface &interface) override;
    void writeLog(QDBusInterface &interface, const QString &srcPath, const QString &destPath);

private:
    QList<QUrl> urlsOfDisc;
    QList<QUrl> urlsOfDest;
};

class BurnFilesAuditLogJob : public AbstractAuditLogJob
{
public:
    BurnFilesAuditLogJob(const QVariantMap &info, const QUrl &stagingUrl, bool result, QObject *parent = nullptr);

protected:
    void doLog(QDBusInterface &interface) override;
    void writeLog(QDBusInterface &interface, const QString &discPath, const QString &nativePath, qint64 size);

private:
    QFileInfoList burnedFileInfoList() const;
    QString bunner() const;
    QString opticalMedia() const;

private:
    QVariantMap discDeviceInfo;
    QUrl localStagingUrl;
    bool burnedSuccess;
};

}   // namespace dfmplugin_burn

#endif   // AUDITLOGJOB_H
