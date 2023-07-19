// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usersharehelper.h"
#include "sharewatchermanager.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/event/event.h>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QtConcurrent>
#include <QNetworkInterface>
#include <QSettings>

#include <pwd.h>
#include <unistd.h>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_dirshare {

namespace DaemonServiceIFace {
static constexpr char kInterfaceService[] { "com.deepin.filemanager.daemon" };
static constexpr char kInterfacePath[] { "/com/deepin/filemanager/daemon/UserShareManager" };
static constexpr char kInterfaceInterface[] { "com.deepin.filemanager.daemon.UserShareManager" };

static constexpr char kFuncIsPasswordSet[] { "IsUserSharePasswordSet" };
static constexpr char kFuncSetPasswd[] { "SetUserSharePassword" };
static constexpr char kFuncCloseShare[] { "CloseSmbShareByShareName" };
static constexpr char kFuncEnableSmbServices[] { "EnableSmbServices" };
}   // namespace DBusINterfaceInfo

namespace ShareConfig {
static constexpr char kShareConfigPath[] { "/var/lib/samba/usershares" };
static constexpr char kShareName[] { "sharename" };
static constexpr char kShareAcl[] { "usershare_acl" };
static constexpr char kSharePath[] { "path" };
static constexpr char kShareComment[] { "comment" };
static constexpr char kGuestOk[] { "guest_ok" };
}   // namespace ShareConfig

namespace SambaServiceIFace {
static constexpr char kService[] { "org.freedesktop.systemd1" };
static constexpr char kPath[] { "/org/freedesktop/systemd1/unit/smbd_2eservice" };
static constexpr char kInterface[] { "org.freedesktop.systemd1.Unit" };

static constexpr char kPropertySubState[] { "SubState" };
static constexpr char kExpectedSubState[] { "running" };

static constexpr char kFuncStart[] { "Start" };
static constexpr char kParamReplace[] { "replace" };
}

UserShareHelper *UserShareHelper::instance()
{
    static UserShareHelper helper;
    return &helper;
}

bool UserShareHelper::share(const ShareInfo &info)
{
    if (!isSambaServiceRunning()) {
        startSambaServiceAsync([this, info](bool ret, const QString &errMsg) {
            if (ret)
                share(info);
            else
                qWarning() << "start samba service failed: " << errMsg;
        });
        return false;
    }

    // check if `net` is installed.
    QString sambaNetPath = QStandardPaths::findExecutable("net");
    if (sambaNetPath.isEmpty()) {
        DialogManagerInstance->showErrorDialog(tr("Kindly Reminder"), tr("Please firstly install samba to continue"));
        return false;
    }

    ShareInfo oldShare = getOldShareByNewShare(info);
    qDebug() << "OldShare: " << oldShare << "\nNewShare: " << info;

    if (isValidShare(info)) {
        const auto &&name = info.value(ShareInfoKeys::kName).toString();
        if (name.startsWith("-") || name.endsWith(" ")) {
            DialogManagerInstance->showErrorDialog(tr("The share name must not contain %1, and cannot start with a dash (-) or whitespace, or end with whitespace.").arg("%<>*?|/\\+=;:,\""), "");
            return false;
        }

        QStringList netArgs;
        auto param = info;
        if (param.value(ShareInfoKeys::kWritable).toBool())
            param.insert(ShareInfoKeys::kAcl, "Everyone:F");
        else
            param.insert(ShareInfoKeys::kAcl, "Everyone:R");

        if (param.value(ShareInfoKeys::kAnonymous).toBool())
            param.insert(ShareInfoKeys::kGuestEnable, "guest_ok=y");
        else
            param.insert(ShareInfoKeys::kGuestEnable, "guest_ok=n");

        netArgs << "usershare"
                << "add" << param.value(ShareInfoKeys::kName).toString()
                << param.value(ShareInfoKeys::kPath).toString()
                << param.value(ShareInfoKeys::kComment).toString()
                << param.value(ShareInfoKeys::kAcl).toString()
                << param.value(ShareInfoKeys::kGuestEnable).toString();

        int port = readPort();
        if (port != -1)
            netArgs << "-p" << QString::number(port);

        QString err;
        int code = runNetCmd(netArgs, -1, &err);
        if (code != 0) {
            handleErrorWhenShareFailed(code, err);
            return false;
        }
    }

    if (isValidShare(oldShare))
        removeShareByPath(oldShare.value(ShareInfoKeys::kPath).toString());

    return true;
}

void UserShareHelper::setSambaPasswd(const QString &userName, const QString &passwd)
{
    QDBusReply<bool> reply = userShareInter->call(DaemonServiceIFace::kFuncSetPasswd, userName, passwd);
    bool result = reply.isValid() && reply.error().message().isEmpty();
    qInfo() << "Samba password set result :" << result << ",error msg:" << reply.error().message();

    Q_EMIT sambaPasswordSet(result);
}

void UserShareHelper::removeShareByPath(const QString &path)
{
    const QString &&shareName = shareNameByPath(path);
    if (!shareName.isEmpty())
        removeShareByShareName(shareName);
}

int UserShareHelper::readPort()
{
    QSettings smbConf("/etc/samba/smb.conf", QSettings::IniFormat);
    static constexpr char kPortKey[] { "global/smb ports" };
    return smbConf.value(kPortKey, -1).toInt();
}

ShareInfoList UserShareHelper::shareInfos()
{
    return sharedInfos.values();
}

ShareInfo UserShareHelper::shareInfoByPath(const QString &path)
{
    const QString &&shareName = shareNameByPath(path);
    return shareInfoByShareName(shareName);
}

ShareInfo UserShareHelper::shareInfoByShareName(const QString &name)
{
    if (!name.isEmpty() && sharedInfos.contains(name))
        return sharedInfos.value(name);
    return {};
}

QString UserShareHelper::shareNameByPath(const QString &path)
{
    if (sharePathToShareName.contains(path)) {
        const auto &&names = sharePathToShareName.value(path);
        if (names.count() > 0)
            return names.last();
    }
    return "";
}

uint UserShareHelper::whoShared(const QString &name)
{
    QFileInfo info(QString("%1/%2").arg(ShareConfig::kShareConfigPath).arg(name));
    return info.ownerId();
}

bool UserShareHelper::isShared(const QString &path)
{
    return sharePathToShareName.contains(path);
}

QString UserShareHelper::currentUserName()
{
    return getpwuid(getuid())->pw_name;
}

void UserShareHelper::handleSetPassword(const QString &newPassword)
{
    setSambaPasswd(currentUserName(), newPassword);
}

bool UserShareHelper::isSambaServiceRunning()
{
    QDBusInterface iface(SambaServiceIFace::kService, SambaServiceIFace::kPath, SambaServiceIFace::kInterface, QDBusConnection::systemBus());

    if (iface.isValid()) {
        const QVariant &variantStatus = iface.property(SambaServiceIFace::kPropertySubState);   // 获取属性 SubState，等同于 systemctl status smbd 结果 Active 值
        if (variantStatus.isValid())
            return SambaServiceIFace::kExpectedSubState == variantStatus.toString();
    }
    return false;
}

void UserShareHelper::startSambaServiceAsync(StartSambaFinished onFinished)
{
    auto watcher = new QFutureWatcher<QPair<bool, QString>>();
    connect(watcher, &QFutureWatcher<QPair<bool, QString>>::finished, this, [onFinished, watcher]() {
        auto result = watcher->result();
        if (onFinished)
            onFinished(result.first, result.second);
        watcher->deleteLater();
    });
    watcher->setFuture(QtConcurrent::run([this] { return startSmbService(); }));
}

QString UserShareHelper::sharedIP() const
{
    QString selfIp;
    QStringList validIpList;
    foreach (QNetworkInterface netInterface, QNetworkInterface::allInterfaces()) {
        if (!netInterface.isValid())
            continue;
        QNetworkInterface::InterfaceFlags flags = netInterface.flags();
        if (!(flags.testFlag(QNetworkInterface::IsRunning) && !flags.testFlag(QNetworkInterface::IsLoopBack)))
            continue;
        QList<QNetworkAddressEntry> entryList = netInterface.addressEntries();
        foreach (QNetworkAddressEntry entry, entryList) {
            if (!entry.ip().toString().isEmpty() && entry.ip().toString() != "0.0.0.0" && entry.ip().toIPv4Address()) {
                validIpList << entry.ip().toString();
            }
        }
    }
    // If multiple IPs are got, just take the first one.
    // There is not a list control on the UI for the IP.
    // TODO(zhuangshu):discuss this issue with product manager, or reference the code of dde-network-core plugin.
    if (validIpList.count() > 0)
        selfIp = validIpList.first();

    return selfIp;
}

UserShareHelper::~UserShareHelper()
{
}

bool UserShareHelper::canShare(FileInfoPointer info)
{
    if (!info || !info->isAttributes(OptInfoType::kIsDir) || !info->isAttributes(OptInfoType::kIsReadable))
        return false;

    if (DevProxyMng->isFileOfProtocolMounts(info->pathOf(PathInfoType::kFilePath)))
        return false;

    if (info->urlOf(UrlInfoType::kUrl).scheme() == Global::Scheme::kBurn || DevProxyMng->isFileFromOptical(info->pathOf(PathInfoType::kFilePath)))
        return false;

    return true;
}

bool UserShareHelper::needDisableShareWidget(FileInfoPointer info)
{
    return (info && info->extendAttributes(ExtInfoType::kOwnerId).toUInt() != static_cast<uint>(SysInfoUtils::getUserId()) && !SysInfoUtils::isRootUser());
}

bool UserShareHelper::isUserSharePasswordSet(const QString &username)
{
    QDBusReply<bool> reply = userShareInter->call(DaemonServiceIFace::kFuncIsPasswordSet, username);
    bool result = reply.isValid() ? reply.value() : false;
    qDebug() << "isSharePasswordSet result: " << result << ", error: " << reply.error();

    return result;
}

void UserShareHelper::readShareInfos(bool sendSignal)
{
    QStringList oldSharedInfoKeys = sharedInfos.keys();
    auto shareInfoCache = sharedInfos;

    sharedInfos.clear();
    sharePathToShareName.clear();

    QDir d(ShareConfig::kShareConfigPath);
    QFileInfoList shareList = d.entryInfoList(QDir::Files | QDir::Hidden);
    for (const auto &fileInfo : shareList) {
        QString filePath = fileInfo.absoluteFilePath();
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "open share file failed: " << filePath;
            return;
        }

        QMap<QString, QString> info;
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            if (!line.isEmpty() && line.contains("=")) {
                int idx = line.indexOf("=");
                QString key = line.mid(0, idx);
                QString value = line.mid(idx + 1);
                info.insert(key, value);
            }
        }
        file.close();

        auto shareInfo = makeInfoByFileContent(info);
        if (isValidShare(shareInfo)) {
            const auto &&name = shareInfo.value(ShareInfoKeys::kName).toString();
            const auto &&path = shareInfo.value(ShareInfoKeys::kPath).toString();

            sharedInfos.insert(name, shareInfo);
            QStringList names = sharePathToShareName.value(path, {});
            names.append(name);
            sharePathToShareName.insert(path, names);
        }
    }

    ShareInfoList newShares;
    for (const auto &info : sharedInfos.values()) {
        if (isValidShare(info)) {
            if (!oldSharedInfoKeys.contains(info.value(ShareInfoKeys::kName).toString()))
                newShares << info;
            else
                oldSharedInfoKeys.removeOne(info.value(ShareInfoKeys::kName).toString());
        }
    }

    // broadcast deleted shares
    for (const QString &shareName : oldSharedInfoKeys) {
        const QString &filePath = shareInfoCache.value(shareName).value(ShareInfoKeys::kPath).toString();
        emitShareRemoved(filePath);
        watcherManager->remove(filePath);
    }

    // broadcast new shares
    for (const ShareInfo &info : newShares) {
        const auto &&path = info.value(ShareInfoKeys::kPath).toString();
        emitShareAdded(path);
        watcherManager->add(path);
    }

    if (!sendSignal)
        return;

    int count = validShareInfoCount();
    if (count == 0)
        emitShareRemoved("/");

    emitShareCountChanged(count);
}

void UserShareHelper::onShareChanged(const QString &path)
{
    if (path.contains(":tmp"))
        return;

    pollingSharesTimer->start();
    //    QTimer::singleShot(1000, this, [=] { /*TODO(xust) TODO(liuyangming) request to refresh file view*/ });
}

void UserShareHelper::onShareFileDeleted(const QString &path)
{
    if (path.contains(ShareConfig::kShareConfigPath))
        onShareChanged(path);
    else
        removeShareWhenShareFolderDeleted(path);
}

void UserShareHelper::onShareMoved(const QString &from, const QString &to)
{
    onShareFileDeleted(from);
    onShareChanged(to);
}

void UserShareHelper::initConnect()
{
    pollingSharesTimer = new QTimer(this);
    pollingSharesTimer->setInterval(300);
    pollingSharesTimer->setSingleShot(true);

    connect(pollingSharesTimer, &QTimer::timeout, this, [this] { this->readShareInfos(); });

    connect(watcherManager, &ShareWatcherManager::fileMoved, this, &UserShareHelper::onShareMoved);
    connect(watcherManager, &ShareWatcherManager::fileDeleted, this, &UserShareHelper::onShareFileDeleted);
    connect(watcherManager, &ShareWatcherManager::subfileCreated, this, &UserShareHelper::onShareChanged);
}

void UserShareHelper::initMonitorPath()
{
    const auto lst = shareInfos();
    for (auto info : lst)
        watcherManager->add(info.value(ShareInfoKeys::kPath).toString());
}

void UserShareHelper::removeShareByShareName(const QString &name)
{
    QDBusReply<bool> reply = userShareInter->asyncCall(DaemonServiceIFace::kFuncCloseShare, name, true);
    if (reply.isValid() && reply.value()) {
        qDebug() << "share closed: " << name;
    } else {
        qWarning() << "share close failed: " << name << ", " << reply.error();
        // TODO(xust) regular user cannot remove the sharing which shared by root user. and should raise an error dialog to notify user.
    }

    runNetCmd(QStringList() << "usershare"
                            << "delete" << name);
}

void UserShareHelper::removeShareWhenShareFolderDeleted(const QString &deletedPath)
{
    const QString &&shareName = shareNameByPath(deletedPath);
    if (shareName.isEmpty())
        return;

    removeShareByShareName(shareName);
}

ShareInfo UserShareHelper::getOldShareByNewShare(const ShareInfo &newShare)
{
    QStringList shareNames = sharePathToShareName.value(newShare.value(ShareInfoKeys::kPath).toString());
    shareNames.removeOne(newShare.value(ShareInfoKeys::kName).toString());
    if (shareNames.count() > 0)
        return shareInfoByShareName(shareNames.last());
    return ShareInfo();
}

int UserShareHelper::runNetCmd(const QStringList &args, int wait, QString *err)
{
    qDebug() << "usershare params:" << args;
    QProcess p;
    p.start("net", args);
    p.waitForFinished(wait);

    int ret = p.exitCode();
    if (ret != 0 && err)
        *err = p.readAllStandardError();
    return ret;
}

void UserShareHelper::handleErrorWhenShareFailed(int code, const QString &err) const
{
    // when shared dir is sys、bin、the same as current user ..., show the notice
    if (err.contains("is already a valid system user name")) {
        DialogManagerInstance->showErrorDialog(tr("Share folder can't be named after the current username"), "");
        return;
    }

    // root权限文件分享会报这个错误信息
    if (err.contains("as we are restricted to only sharing directories we own.")) {
        DialogManagerInstance->showErrorDialog(tr("To protect the files, you cannot share this folder."), "");
        return;
    }

    // 共享文件的共享名输入特殊字符会报这个错误信息
    if (err.contains("contains invalid characters")) {
        DialogManagerInstance->showErrorDialog(tr("The share name must not contain %1, and cannot start with a dash (-) or whitespace, or end with whitespace.").arg("%<>*?|/\\+=;:,\""), "");
        return;
    }

    // net usershare add: failed to add share sharename. Error was 文件名过长
    // 共享文件的共享名太长，会报上面这个错误信息，最后居然还是中文
    // another fix: 有多种问题会报上面的错误信息，该错误信息最后的错误描述是系统翻译后的文本，所以这里改成直接显示命令返回的错误描述。
    if (err.contains("net usershare add: failed to add share") && err.contains("Error was ")) {
        QString errorDisc = err.split("Error was ").last();
        errorDisc = errorDisc.remove("\n");
        DialogManagerInstance->showErrorDialog(errorDisc, "");
        return;
    }

    //计算机名称过长会报错
    if (err.contains("gethostname failed") && err.contains("net usershare add: cannot convert name")) {
        DialogManagerInstance->showErrorDialog(tr("Sharing failed"), tr("The computer name is too long"));
        return;
    }

    DialogManagerInstance->showErrorDialog(QString(), err);
    qWarning() << "run net command failed: " << err << ", code is: " << code;
}

ShareInfo UserShareHelper::makeInfoByFileContent(const QMap<QString, QString> &contents)
{
    QString shareName = contents.value(ShareConfig::kShareName);
    QString sharePath = contents.value(ShareConfig::kSharePath);
    QString shareAcl = contents.value(ShareConfig::kShareAcl);

    ShareInfo info;
    if (!shareName.isEmpty() && !sharePath.isEmpty()
        && QFile(sharePath).exists() && !shareAcl.isEmpty()) {
        info = {
            { ShareInfoKeys::kName, shareName.toLower() },
            { ShareInfoKeys::kPath, sharePath },
            { ShareInfoKeys::kComment, contents.value(ShareConfig::kShareComment) },
            { ShareInfoKeys::kAcl, shareAcl },
            { ShareInfoKeys::kGuestEnable, contents.value(ShareConfig::kGuestOk) },
            { ShareInfoKeys::kAnonymous, contents.value(ShareConfig::kGuestOk) == "y" },
            { ShareInfoKeys::kWritable, shareAcl.toUpper() == "S-1-1-0:F" }   // just for current needs.
        };
    }
    return info;
}

int UserShareHelper::validShareInfoCount() const
{
    return std::accumulate(sharedInfos.begin(), sharedInfos.end(),
                           0, [this](int sum, const ShareInfo &info) {
                               return sum += isValidShare(info) ? 1 : 0;
                           });
}

QPair<bool, QString> UserShareHelper::startSmbService()
{
    QDBusInterface iface(SambaServiceIFace::kService, SambaServiceIFace::kPath, SambaServiceIFace::kInterface, QDBusConnection::systemBus());
    QDBusPendingReply<QDBusObjectPath> reply = iface.asyncCall(SambaServiceIFace::kFuncStart, SambaServiceIFace::kParamReplace);
    reply.waitForFinished();
    if (reply.isValid()) {
        const QString &errMsg = reply.error().message();
        if (errMsg.isEmpty()) {
            if (!setSmbdAutoStart())
                qWarning() << "auto start smbd failed.";
            return { true, "" };
        }
        return { false, errMsg };
    }
    return { false, "restart smbd failed" };
}

bool UserShareHelper::setSmbdAutoStart()
{
    QDBusReply<bool> reply = userShareInter->call(DaemonServiceIFace::kFuncEnableSmbServices);
    return reply.value();
}

bool UserShareHelper::isValidShare(const QVariantMap &info) const
{
    auto name = info.value(ShareInfoKeys::kName).toString();
    auto path = info.value(ShareInfoKeys::kPath).toString();
    return !name.isEmpty() && QFile(path).exists();
}

void UserShareHelper::emitShareCountChanged(int count)
{
    Q_EMIT shareCountChanged(count);
    dpfSignalDispatcher->publish(kEventSpace, "signal_Share_ShareCountChanged", count);
}

void UserShareHelper::emitShareAdded(const QString &path)
{
    Q_EMIT shareAdded(path);
    dpfSignalDispatcher->publish(kEventSpace, "signal_Share_ShareAdded", path);
}

void UserShareHelper::emitShareRemoved(const QString &path)
{
    Q_EMIT shareRemoved(path);
    dpfSignalDispatcher->publish(kEventSpace, "signal_Share_ShareRemoved", path);
}

void UserShareHelper::emitShareRemoveFailed(const QString &path)
{
}

UserShareHelper::UserShareHelper(QObject *parent)
    : QObject(parent)
{
    userShareInter.reset(new QDBusInterface(DaemonServiceIFace::kInterfaceService, DaemonServiceIFace::kInterfacePath, DaemonServiceIFace::kInterfaceInterface, QDBusConnection::systemBus(), this));

    watcherManager = new ShareWatcherManager(this);
    watcherManager->add(ShareConfig::kShareConfigPath);

    initConnect();
    readShareInfos();

    initMonitorPath();
}
}
