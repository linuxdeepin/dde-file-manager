// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auditlogjob.h"
#include "utils/burnhelper.h"

#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QDebug>
#include <QDateTime>

namespace dfmplugin_burn {
DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

namespace AuditHelper {
QString bunner(const QVariant &value)
{
    const QStringList &splitedDrive { value.toString().split("/") };
    return splitedDrive.isEmpty() ? QString() : splitedDrive.last();
}

QString opticalMedia(const QVariant &value)
{
    const QString &media { value.toString() };
    return DeviceUtils::formatOpticalMediaType(media);
}

qint64 idGenerator()
{
    static qint64 index { 0 };
    static qint64 baseID { QDateTime::currentSecsSinceEpoch() };

    qint64 id { baseID + (index++) };
    return id;
}
}   // namespace AuditHelper

AbstractAuditLogJob::AbstractAuditLogJob(QObject *parent)
    : QThread(parent)
{
}

void AbstractAuditLogJob::run()
{
    fmInfo() << "Create D-Bus Auditd interface object start";
    QDBusInterface auditd("org.deepin.PermissionManager.Auditd",
                          "/org/deepin/PermissionManager/Auditd",
                          "org.deepin.PermissionManager.Auditd",
                          QDBusConnection::systemBus());
    auditd.setTimeout(500);
    if (!auditd.isValid()) {
        fmWarning() << "Invalid Auditd D-Bus interface";
        return;
    }
    fmInfo() << "Create D-Bus Auditd interface object end";

    fmInfo() << "Call D-Bus WriteAuditLog start";
    doLog(auditd);
    fmInfo() << "Call D-Bus WriteAuditLog end";
}

CopyFromDiscAuditLog::CopyFromDiscAuditLog(const QList<QUrl> &srcList, const QList<QUrl> &destList, QObject *parent)
    : AbstractAuditLogJob(parent),
      urlsOfDisc(srcList),
      urlsOfDest(destList)
{
}

void CopyFromDiscAuditLog::doLog(QDBusInterface &interface)
{
    for (int i = 0; i != urlsOfDisc.size(); ++i) {
        const QString &srcPath { urlsOfDisc.at(i).toLocalFile() };
        const QString &destPath { urlsOfDest.at(i).toLocalFile() };
        auto response { interface.call("NeedAuditForCopy", srcPath) };
        if (response.type() != QDBusMessage::ReplyMessage || response.arguments().isEmpty()) {
            fmWarning() << "Call NeedAuditForCopy Failed";
            continue;
        }
        if (!response.arguments().takeFirst().toBool())
            continue;
        fmWarning() << "Current env auditlog allowed: " << srcPath;

        auto fileInfo { InfoFactory::create<FileInfo>(QUrl::fromLocalFile(srcPath)) };
        if (fileInfo->isAttributes(OptInfoType::kIsDir)) {
            for (const QFileInfo &subInfo : BurnHelper::localFileInfoListRecursive(srcPath))
                writeLog(interface, subInfo.absoluteFilePath(), destPath);
        } else {
            writeLog(interface, srcPath, destPath);
        }
    }
}

void CopyFromDiscAuditLog::writeLog(QDBusInterface &interface, const QString &srcPath, const QString &destPath)
{
    QString dateTime { QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") };
    static constexpr int kCount { 1 };
    static const QString kLogTemplate { "[%1] %2 %3: file_count=%4, src_file=%5, target_file=%6, file_type=%7, file_size=%8" };
    static const QString kLogKey { "file_copy" };
    static const QString &kUserName { SysInfoUtils::getUser() };
    static const QString &kHostName { SysInfoUtils::getHostName() };

    auto fmInfo { InfoFactory::create<FileInfo>(QUrl::fromLocalFile(srcPath), Global::CreateFileInfoType::kCreateFileInfoSync) };
    const QString &fileType { fmInfo ? fmInfo->displayOf(DisPlayInfoType::kMimeTypeDisplayName) : "" };
    const QString &curLog { kLogTemplate.arg(dateTime).arg(kHostName).arg(kUserName).arg(kCount).arg(srcPath).arg(destPath).arg(fileType).arg(FileUtils::formatSize(fmInfo->size())) };
    interface.call("WriteLog", kLogKey, curLog);
}

BurnFilesAuditLogJob::BurnFilesAuditLogJob(const QUrl &stagingUrl, bool result, QObject *parent)
    : AbstractAuditLogJob(parent),
      localStagingUrl(stagingUrl),
      burnedSuccess(result)
{
}

void BurnFilesAuditLogJob::doLog(QDBusInterface &interface)
{
    QString device { property(DeviceProperty::kDevice).toString() };
    const auto &pathMap { Application::dataPersistence()->value("StagingMap", device).toMap() };

    for (const QFileInfo &info : burnedFileInfoList()) {
        if (!info.exists()) {
            fmWarning() << "File doesn't exitsts: " << info.absoluteFilePath();
            continue;
        }

        const QString &discPath { info.absoluteFilePath() };
        QString nativePath { pathMap.contains(discPath) ? pathMap.value(discPath).toString() : discPath };

        if (info.isDir()) {
            for (const QFileInfo &subInfo : BurnHelper::localFileInfoListRecursive(info.absoluteFilePath())) {
                QString subNativePath { subInfo.absoluteFilePath() };
                subNativePath = subNativePath.replace(discPath, nativePath);
                writeLog(interface, subInfo.absoluteFilePath(), subNativePath, subInfo.size());
            }
        } else {
            writeLog(interface, discPath, nativePath, info.size());
        }
    }
}

void BurnFilesAuditLogJob::writeLog(QDBusInterface &interface, const QString &discPath, const QString &nativePath, qint64 size)
{
    static const QString kLogKey { "cdrecord" };
    static const QString kLogTemplate { QObject::tr("ID=%1, DateTime=%2, Burner=%3, DiscType=%4, Result=%5, User=%6, FileName=%7, FileSize=%8, FileType=%9") };
    static const QString &kUserName { SysInfoUtils::getUser() };

    const QString &result { burnedSuccess ? QObject::tr("Success") : QObject::tr("Failed") };
    const QString &dateTime { QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") };
    const QString &burner { AuditHelper::bunner(property(DeviceProperty::kDrive)) };
    const QString &discType { AuditHelper::opticalMedia(property(DeviceProperty::kMedia)) };

    auto fmInfo { InfoFactory::create<FileInfo>(QUrl::fromLocalFile(discPath), Global::CreateFileInfoType::kCreateFileInfoSync) };
    const QString &fileType { fmInfo ? fmInfo->displayOf(DisPlayInfoType::kMimeTypeDisplayName) : "" };
    QString curLog { kLogTemplate.arg(AuditHelper::idGenerator()).arg(dateTime).arg(burner).arg(discType).arg(result).arg(kUserName).arg(nativePath).arg(FileUtils::formatSize(size)).arg(fileType) };
    interface.call("WriteLog", kLogKey, curLog);

    if (burnedSuccess) {
        QString device { property(DeviceProperty::kDevice).toString() };
        Application::dataPersistence()->remove("StagingMap", device);
        Application::dataPersistence()->sync();
    }
}

QFileInfoList BurnFilesAuditLogJob::burnedFileInfoList() const
{
    QFileInfoList burnedFileInfoGroup;
    QFileInfo info { localStagingUrl.toLocalFile() };
    if (info.isFile())   // image file
        burnedFileInfoGroup.append(info);

    if (info.isDir())   // data files
        burnedFileInfoGroup = BurnHelper::localFileInfoList(info.absoluteFilePath());

    return burnedFileInfoGroup;
}

EraseDiscAuditLogJob::EraseDiscAuditLogJob(bool result, QObject *parent)
    : AbstractAuditLogJob(parent),
      eraseSuccess(result)
{
}

void EraseDiscAuditLogJob::doLog(QDBusInterface &interface)
{
    static const QString kLogKey { "cdrecord" };
    static const QString kLogTemplate { "ID=%1, Type=%2, Burner=%3, DiscType=%4, User=%5, DateTime=%6, Result=%7" };
    static const QString &kUserName { SysInfoUtils::getUser() };

    const QString &result { eraseSuccess ? "Success" : "Failed" };
    const QString &dateTime { QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") };
    const QString &burner { AuditHelper::bunner(property(DeviceProperty::kDrive)) };
    const QString &discType { AuditHelper::opticalMedia(property(DeviceProperty::kMedia)) };

    QString curLog { kLogTemplate
                             .arg(AuditHelper::idGenerator())
                             .arg("Erase")
                             .arg(burner)
                             .arg(discType)
                             .arg(kUserName)
                             .arg(dateTime)
                             .arg(result) };
    interface.call("WriteLog", kLogKey, curLog);
}

}   // namespace dfmplugin_burn
