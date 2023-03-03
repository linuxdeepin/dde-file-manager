// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
