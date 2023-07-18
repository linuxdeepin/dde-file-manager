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

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void travers_prehandler::networkAccessPrehandler(quint64 winId, const QUrl &url, std::function<void()> after)
{
    const auto &&scheme = url.scheme();
    static const QStringList &kSupportedSchemes { Global::Scheme::kSmb, Global::Scheme::kFtp, Global::Scheme::kSFtp };
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

    DevMngIns->mountNetworkDeviceAsync(mountSource, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err, const QString &mpt) {
        qInfo() << "mount done: " << url << ok << err.code << err.message << mpt;
        if (!mpt.isEmpty()) {
            doChangeCurrentUrl(winId, mpt, subPath, url);
        } else if (ok || err.code == DFMMOUNT::DeviceError::kGIOErrorAlreadyMounted) {
            if (isSmb) onSmbRootMounted(mountSource, after);
        } else {
            DialogManager::instance()->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err);
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

    pddmDbg << "do cache root entry" << mountSource;
    VirtualEntryDbHandler::instance()->saveData(VirtualEntryData(mountSource));

    pddmDbg << "add virtual entry for smb root" << mountSource;
    const QUrl &entryUrl = protocol_display_utilities::makeVEntryUrl(mountSource);
    computer_sidebar_event_calls::callItemAdd(entryUrl);
}
