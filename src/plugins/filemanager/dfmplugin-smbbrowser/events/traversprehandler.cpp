// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <dfm-base/utils/networkutils.h>

#include <dfm-framework/event/event.h>

#include <QTimer>
#include <QDBusInterface>
#include <QNetworkInterface>
#include <QRegularExpression>

#include <unistd.h>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace {
/*!
 * \brief gvfsMountRootOf
 *
 * GVFS exposes the remote absolute root ('/') at \c /run/user/<uid>/gvfs/<mount-spec>
 * (or the legacy \c /root/.gvfs/<mount-spec>), but the mount point reported by GIO
 * is the mount's *default location* — an arbitrary directory below that root (for
 * sftp/ftp it is the login home directory). Returns the mount root, or an empty
 * string when \a localPath is not a gvfs path.
 */
QString gvfsMountRootOf(const QString &localPath)
{
    static const QRegularExpression kGvfsRoot { R"(^(?:/run/user/\d+/gvfs/|/root/\.gvfs/)[^/]+)" };
    const auto match = kGvfsRoot.match(localPath);
    return match.hasMatch() ? match.captured(0) : QString();
}
}   // namespace

void travers_prehandler::networkAccessPrehandler(quint64 winId, const QUrl &url, std::function<void()> after)
{
    const auto &&scheme = url.scheme();
    static const QStringList &kSupportedSchemes { Global::Scheme::kSmb,
                                                  Global::Scheme::kFtp,
                                                  Global::Scheme::kSFtp,
                                                  Global::Scheme::kNfs,
                                                  Global::Scheme::kDav,
                                                  Global::Scheme::kDavs };
    if (!kSupportedSchemes.contains(scheme)) {
        fmDebug() << "Scheme not supported for network access:" << scheme;
        return;
    }

    QUrl netUrl = url;
    if (url.hasFragment()) {
        netUrl = url.adjusted(QUrl::RemoveFragment);
        // '#' 标识片段起始，后面的内容被分配到了fragment字段中。
        // 当 '#' 紧跟 host（如 smb://1.2.3.4#share/）时 url.path() 为空，
        // 此时需补前导 '/'，否则 QUrl::setPath 会生成无效 URL（toString 返回空串）。
        auto path = url.path();
        if (path.isEmpty())
            path = "/";
        path = path + '#' + url.fragment(QUrl::FullyDecoded);
        netUrl.setPath(path);
        fmInfo() << "networkAccessPrehandler: converted fragment into path."
                 << "oldUrl: " << url
                 << "newUrl" << netUrl;
    }

    // Probe the server before trying to mount: an unreachable host must fail
    // fast instead of leaving a stale "connecting" state in the view.
    if (!netUrl.host().isEmpty() && NetworkUtils::instance()->checkFtpOrSmbBusy(netUrl, 1000)) {
        QApplication::restoreOverrideCursor();
        fmWarning() << "Network is not reachable for URL:" << netUrl.toString();
        DialogManager::instance()->showUnableToVistDir(netUrl.toString());
        return;
    }

    // the mounted source must be the TOP dir.
    // if smb://1.2.3.4/top/subdir is passed in, the param passed to mount function must be smb://1.2.3.4/top
    // smb/ftp/sftp share the same split logic; other schemes return source unchanged.
    QString mountSource = netUrl.toString();
    QString subPath;
    bool isSmb = false;
    mountSource = prehandler_utils::splitMountSource(QUrl::fromPercentEncoding(netUrl.toString().toUtf8()), &subPath);
    if (scheme == Global::Scheme::kSmb)
        isSmb = true;

    // fix 237183
    QString origUrl;
    auto host = netUrl.host();
    if (host.contains("xn--")) {   // xn--0zwm56d (测试)
        int dotAppended = 0;
        if (!host.endsWith("."))
            host += ".", dotAppended = 1;   // without this dot, cannot decoded by punycode. qurlidna.cpp::qt_ACE_do()
        QUrl u = QUrl::fromUserInput(host);   // 'http://' is auto prepended. (http://xn--0zwm56d.)
        auto punyDecodedHost = u.host().chopped(dotAppended);
        origUrl = netUrl.toString().replace(netUrl.host(), punyDecodedHost);   // got the original url (smb://测试/)
    }

    auto onMountFailed = [netUrl, origUrl](const dfmmount::OperationErrorInfo &err) {
        fmCritical() << "Mount operation failed for URL:" << netUrl.toString()
                     << "error code:" << err.code << "error message:" << err.message;

        DialogManager::instance()->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err);
        // remove from history when mount failed.
        dpfSlotChannel->push("dfmplugin_titlebar", "slot_ServerDialog_RemoveHistory", netUrl.toString().toUtf8());
        if (!origUrl.isEmpty()) {
            dpfSlotChannel->push("dfmplugin_titlebar", "slot_ServerDialog_RemoveHistory", origUrl);
            fmDebug() << "Removed both original and decoded URLs from history";
        }
    };

    static QString kRecordFilePath = QString("/tmp/dfm_smb_mount_%1.ini").arg(getuid());
    static QString kRecordGroup = "defaultSmbPath";
    static QRegularExpression kRegx { "/|\\.|:" };
    DevMngIns->mountNetworkDeviceAsync(mountSource, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err, const QString &mpt) {
        if (!mpt.isEmpty()) {
            doChangeCurrentUrl(winId, mpt, subPath, netUrl);
        } else if (ok || err.code == DFMMOUNT::DeviceError::kGIOErrorAlreadyMounted) {
            if (isSmb)
                onSmbRootMounted(mountSource, after);
            else if (after)
                after();
        } else {
            fmWarning() << "mount done: " << netUrl << ok << err.code << err.message << mpt;
            QApplication::restoreOverrideCursor();
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
    } else {
        fmDebug() << "Target host is not local, skipping service validation:" << targetHost;
    }

    QTimer::singleShot(100, qApp, [=] { networkAccessPrehandler(winId, url, after); });
}

QString prehandler_utils::splitMountSource(const QString &source, QString *subPath)
{
    // smb/ftp/sftp 的 query（如 ?charset=gbk）是挂载参数，要随挂载地址保留；
    // 它在首个 '/' 之后会被下面的截断规则当成子目录，先摘出、最后拼回（fad1a3b8b 引入）
    QString addr = source;
    QString query;
    const int queryPos = source.indexOf('?');
    const QString &scheme = QUrl(source).scheme();
    if (queryPos > 0 && (scheme == Global::Scheme::kSmb || scheme == Global::Scheme::kFtp || scheme == Global::Scheme::kSFtp)) {
        addr = source.left(queryPos);
        query = source.mid(queryPos);
    }

    static const QRegularExpression regx(R"((^smb://[^/]*/[^/]*|^s?ftp://[^/]*))",
                                         QRegularExpression::CaseInsensitiveOption);
    auto match = regx.match(addr);
    if (match.hasMatch()) {
        QString top = match.captured(1);
        if (subPath)
            *subPath = addr.mid(top.length() + 1);   // skip the '/'
        while (top.endsWith("/"))
            top.chop(1);
        return top + query;
    }

    return source;
}

/*!
 * \brief prehandler_utils::joinMountedPath
 *
 * Builds the local path to navigate to from the mount point reported by the mount
 * backend and the remote sub path parsed by \a splitMountSource.
 *
 * The mount point of a GVFS mount is NOT the mount root but its *default location*
 * (see gvfs backend: sftp/ftp report the login home directory via PWD, smb reports
 * the top-most existing directory of the share). The sub path however is always
 * relative to the remote root, so a plain concatenation duplicates the prefix:
 *
 *   mount point : /run/user/1000/gvfs/sftp:host=1.2.3.4,user=bob/home/bob
 *   sub path    : home/bob/Documents
 *   concat      : .../home/bob/home/bob/Documents   <- wrong
 *
 * For GVFS mounts the base therefore must be the mount root, i.e. the path up to
 * the mount spec (/run/user/<uid>/gvfs/<spec> or /root/.gvfs/<spec>). Non-GVFS
 * mounts (cifs/smbmounts, nfs, ...) are rooted at their mount point, so the plain
 * concatenation is kept there.
 */
QString prehandler_utils::joinMountedPath(const QString &mountPoint, const QString &subPath)
{
    if (subPath.isEmpty())
        return mountPoint;

    QString base = mountPoint;
    const QString gvfsRoot = gvfsMountRootOf(mountPoint);
    if (!gvfsRoot.isEmpty()) {
        base = gvfsRoot;
        fmDebug() << "joinMountedPath: gvfs default location replaced by mount root."
                  << "mountPoint:" << mountPoint
                  << "mountRoot:" << base
                  << "subPath:" << subPath;
    }

    while (base.endsWith("/"))
        base.chop(1);

    QString path = base + "/" + subPath;
    while (path.size() > 1 && path.endsWith("/"))
        path.chop(1);
    return path;
}

void travers_prehandler::doChangeCurrentUrl(quint64 winId, const QString &mpt, const QString &subPath, const QUrl &sourceUrl)
{
    QString targetPath = prehandler_utils::joinMountedPath(mpt, subPath);

    QUrl url = QUrl::fromLocalFile(targetPath);
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(url, Global::kCreateFileInfoSync);
    if (!fileInfo || !fileInfo->exists()) {
        QApplication::restoreOverrideCursor();
        fmWarning() << "Mounted path does not exist:" << targetPath;
        DialogManager::instance()->showUnableToVistDir(targetPath);
        return;
    }
    if (fileInfo->isAttributes(FileInfo::FileIsType::kIsFile)) {
        dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, winId, QList<QUrl> { url });

        QUrl parentUrl = fileInfo->urlOf(UrlInfoType::kParentUrl);
        parentUrl.setQuery("selectUrl=" + url.toString());
        url = parentUrl;
    }
    dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, url);

    // remove sourceUrl from history stack.
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Navigator_Remove", winId, sourceUrl);
}

void travers_prehandler::onSmbRootMounted(const QString &mountSource, Handler after)
{
    if (after)
        after();

    if (!ProtocolDeviceDisplayManager::instance()->isShowOfflineItem()) {
        fmDebug() << "Show offline items is disabled, skipping virtual entry processing";
        return;
    }

    if (ProtocolDeviceDisplayManager::instance()->displayMode() != SmbDisplayMode::kAggregation) {
        fmDebug() << "Display mode is not aggregation, skipping virtual entry processing";
        return;
    }

    if (QUrl(mountSource).host().isEmpty()) {
        fmWarning() << "Mount source has empty host, cannot process virtual entry:" << mountSource;
        return;
    }

    fmDebug() << "do cache root entry" << mountSource;
    VirtualEntryDbHandler::instance()->saveData(VirtualEntryData(mountSource));

    fmDebug() << "add virtual entry for smb root" << mountSource;
    const QUrl &entryUrl = protocol_display_utilities::makeVEntryUrl(mountSource);
    computer_sidebar_event_calls::callItemAdd(entryUrl);
}
