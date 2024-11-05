// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "traversprehandler.h"
#include "utils/smbbrowserutils.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"
#include "displaycontrol/protocoldevicedisplaymanager.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-framework/event/event.h>

#include <QTimer>
#include <QDBusInterface>
#include <QNetworkInterface>

#include <unistd.h>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void travers_prehandler::networkAccessPrehandler(quint64 winId, const QUrl &url, std::function<void()> after)
{
    const auto &&scheme = url.scheme();
    static const QStringList &kSupportedSchemes { Global::Scheme::kSmb,
                                                  Global::Scheme::kFtp,
                                                  Global::Scheme::kSFtp,
                                                  Global::Scheme::kNfs,
                                                  Global::Scheme::kDav,
                                                  Global::Scheme::kDavs };
    if (!kSupportedSchemes.contains(scheme))
        return;

    // the mounted source must be the TOP dir.
    // if smb://1.2.3.4/top/subdir is passed in, the param passed to mount function must be smb://1.2.3.4/top
    // and only deal smb scheme now.
    QString mountSource = url.toString();
    QString subPath;
    bool isSmb = false;
    if (scheme == Global::Scheme::kSmb) {
        mountSource = prehandler_utils::splitMountSource(url.toString(), &subPath);
        isSmb = true;
    }

    // fix 237183
    QString origUrl;
    auto host = url.host();
    if (host.contains("xn--")) {   // xn--0zwm56d (测试)
        int dotAppended = 0;
        if (!host.endsWith("."))
            host += ".", dotAppended = 1;   // without this dot, cannot decoded by punycode. qurlidna.cpp::qt_ACE_do()
        QUrl u = QUrl::fromUserInput(host);   // 'http://' is auto prepended. (http://xn--0zwm56d.)
        auto punyDecodedHost = u.host().chopped(dotAppended);
        origUrl = url.toString().replace(url.host(), punyDecodedHost);   // got the original url (smb://测试/)
    }

    auto onMountFailed = [url, origUrl](const dfmmount::OperationErrorInfo &err) {
        DialogManager::instance()->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err);
        // remove from history when mount failed.
        dpfSlotChannel->push("dfmplugin_titlebar", "slot_ServerDialog_RemoveHistory", url.toString());
        if (!origUrl.isEmpty())
            dpfSlotChannel->push("dfmplugin_titlebar", "slot_ServerDialog_RemoveHistory", origUrl);
    };

    static QString kRecordFilePath = QString("/tmp/dfm_smb_mount_%1.ini").arg(getuid());
    static QString kRecordGroup = "defaultSmbPath";
    static QRegularExpression kRegx { "/|\\.|:" };
    auto recordSubPath = [](const QString &smbRoot, const QString &subPath) {
        QFile record(kRecordFilePath);
        if (!record.exists() && record.open(QIODevice::NewOnly))
            record.close();
        auto key(smbRoot);
        key = key.replace(kRegx, "_");
        QSettings sets(kRecordFilePath, QSettings::IniFormat);
        sets.setValue(QString("%1/%2").arg(kRecordGroup).arg(key), subPath);
    };
    auto readSubPath = [](const QString &smbRoot) {
        auto key(smbRoot);
        key = key.replace(kRegx, "_");
        QSettings sets(kRecordFilePath, QSettings::IniFormat);
        return sets.value(QString("%1/%2").arg(kRecordGroup).arg(key), "").toString();
    };

    DevMngIns->mountNetworkDeviceAsync(mountSource, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err, const QString &mpt) {
        fmInfo() << "mount done: " << url << ok << err.code << err.message << mpt;
        if (!mpt.isEmpty()) {
            if (err.code == DFMMOUNT::DeviceError::kNoError)
                recordSubPath(mountSource, subPath);
            QString jumpTo(subPath);
            if (jumpTo.isEmpty())
                jumpTo = readSubPath(mountSource);
            doChangeCurrentUrl(winId, mpt, jumpTo, url);
        } else if (ok || err.code == DFMMOUNT::DeviceError::kGIOErrorAlreadyMounted) {
            if (isSmb) onSmbRootMounted(mountSource, after);
        } else {
            onMountFailed(err);
        }
    });
}

void travers_prehandler::smbAccessPrehandler(quint64 winId, const QUrl &url, std::function<void()> after)
{
    QStringList localHosts;
    auto addrs = QNetworkInterface::allAddresses();
    for (const auto &addr : addrs)
        localHosts << addr.toString();
    localHosts << "localhost";

    QString targetHost = url.host();
    if (localHosts.contains(targetHost)) {   // only check service when access local shares
        QStringList validateService { "smb", "nmb" };
        for (const auto &serv : validateService) {
            if (smb_browser_utils::checkAndEnableService(serv))
                continue;
            dpfSlotChannel->push("dfmplugin_titlebar", "slot_Navigator_Backward", winId);   // if failed/cancelled, back to previous page.
            return;
        }
    }

    QTimer::singleShot(100, qApp, [=] { networkAccessPrehandler(winId, url, after); });
}

QString prehandler_utils::splitMountSource(const QString &source, QString *subPath)
{
    static const QRegularExpression regx(R"((^smb://[^/]*/[^/]*))");
    auto match = regx.match(source);
    if (match.hasMatch()) {
        QString top = match.captured(1);
        if (subPath)
            *subPath = source.mid(top.length() + 1);   // skip the '/'
        while (top.endsWith("/"))
            top.chop(1);
        return top;
    }

    return source;
}

void travers_prehandler::doChangeCurrentUrl(quint64 winId, const QString &mpt, const QString &subPath, const QUrl &sourceUrl)
{
    QString targetPath = mpt;
    if (!subPath.isEmpty() && !targetPath.endsWith("/"))
        targetPath.append("/");
    targetPath.append(subPath);

    QUrl url = QUrl::fromLocalFile(targetPath);
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(url);
    if (fileInfo && fileInfo->isAttributes(FileInfo::FileIsType::kIsFile))
        url = fileInfo->urlOf(FileInfo::FileUrlInfoType::kParentUrl);
    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, url);

    // remove sourceUrl from history stack.
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Navigator_Remove", winId, sourceUrl);
}

void travers_prehandler::onSmbRootMounted(const QString &mountSource, Handler after)
{
    if (after)
        after();

    if (!ProtocolDeviceDisplayManager::instance()->isShowOfflineItem())
        return;
    if (ProtocolDeviceDisplayManager::instance()->displayMode() != SmbDisplayMode::kAggregation)
        return;

    if (QUrl(mountSource).host().isEmpty())
        return;

    fmDebug() << "do cache root entry" << mountSource;
    VirtualEntryDbHandler::instance()->saveData(VirtualEntryData(mountSource));

    fmDebug() << "add virtual entry for smb root" << mountSource;
    const QUrl &entryUrl = protocol_display_utilities::makeVEntryUrl(mountSource);
    computer_sidebar_event_calls::callItemAdd(entryUrl);
}
