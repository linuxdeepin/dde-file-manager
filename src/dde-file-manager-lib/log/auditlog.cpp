// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auditlog.h"
#include "shutil/fileutils.h"
#include "interfaces/dfileservices.h"

#include <QDebug>
#include <QObject>
#include <QDateTime>
#include <QDBusInterface>
#include <QFileInfoList>


void AuditLog::doCopyFromDiscAuditLog(const DUrlList &srcList, const DUrlList &destList)
{
    Q_ASSERT(srcList.size() == destList.size());

    qInfo() << "Create D-Bus Auditd interface object start";
    QDBusInterface auditd("org.deepin.PermissionManager.Auditd",
                          "/org/deepin/PermissionManager/Auditd",
                          "org.deepin.PermissionManager.Auditd",
                          QDBusConnection::systemBus());
    auditd.setTimeout(500);
    if (!auditd.isValid()) {
        qWarning() << "Invalid Auditd D-Bus interface";
        return;
    }
    qInfo() << "Create D-Bus Auditd interface object end";

    const QString &userName { DFMGlobal::getUser()} ;
    const QString &hostName { DFMGlobal::hostName() };
    QString dateTime { QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") };

    static int kCount { 1 };
    static QString kLogTemplate {"[%1] %2 %3: file_count=%4, src_file=%5, target_file=%6, file_type=%7, file_size=%8"};
    static QString kLogKey { "file_copy" };

    qInfo() << "Call D-Bus WriteLog start";
    for (int i = 0; i != srcList.size(); ++i) {
        const QString &srcPath { srcList.at(i).toLocalFile() };
        const QString &destPath { destList.at(i).toLocalFile() };
        auto response { auditd.call("NeedAuditForCopy", srcPath) };
        if (response.type() != QDBusMessage::ReplyMessage || response.arguments().isEmpty()) {
            qWarning() << "Call NeedAuditForCopy Failed";
            continue;
        }

        if (!response.arguments().takeFirst().toBool())
            continue;
        qInfo() << "Current env auditlog allowed: " << srcPath;

        QString curLog;
        QFileInfo srcInfo { srcPath };
        if (srcInfo.isDir()) {
            QFileInfoList &&dirFileInfoGroup { FileUtils::fileInfoListRecursive(srcPath) };
            for (const QFileInfo &subInfo : dirFileInfoGroup) {
                QString &&subFilePath { subInfo.absoluteFilePath() };
                auto fmInfo {DFileService::instance()->createFileInfo(nullptr, DUrl::fromLocalFile(subFilePath))};
                QString &&fileType { fmInfo ? fmInfo->mimeTypeDisplayName() : "" };
                curLog = kLogTemplate.arg(dateTime).arg(hostName).arg(userName)
                        .arg(kCount).arg(subFilePath).arg(destPath).arg(fileType)
                        .arg(FileUtils::formatSize(subInfo.size()));
                auditd.call("WriteLog", kLogKey, curLog);
            }
        } else {
            auto fmInfo {DFileService::instance()->createFileInfo(nullptr, DUrl::fromLocalFile(srcPath))};
            QString &&fileType { fmInfo ? fmInfo->mimeTypeDisplayName() : "" };
            curLog = kLogTemplate.arg(dateTime).arg(hostName).arg(userName)
                    .arg(kCount).arg(srcPath).arg(destPath).arg(fileType)
                    .arg(FileUtils::formatSize(srcInfo.size()));
            auditd.call("WriteLog", kLogKey, curLog);
        }
    }
    qInfo() << "Call D-Bus WriteLog end";
}
