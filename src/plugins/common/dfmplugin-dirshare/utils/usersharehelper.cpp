// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usersharehelper.h"
#include "sharewatchermanager.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systemservicemanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/event/event.h>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QtConcurrent>
#include <QNetworkInterface>
#include <QSettings>
#include <QTemporaryFile>

#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_dirshare {

namespace DaemonServiceIFace {
static constexpr char kInterfaceService[] { "org.deepin.Filemanager.UserShareManager" };
static constexpr char kInterfacePath[] { "/org/deepin/Filemanager/UserShareManager" };
static constexpr char kInterfaceInterface[] { "org.deepin.Filemanager.UserShareManager" };

static constexpr char kFuncIsPasswordSet[] { "IsUserSharePasswordSet" };
static constexpr char kFuncSetPasswd[] { "SetUserSharePassword" };
static constexpr char kFuncCloseShare[] { "CloseSmbShareByShareName" };
}   // namespace DaemonServiceIFace

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
                fmWarning() << "start samba service failed: " << errMsg;
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
    fmDebug() << "OldShare: " << oldShare << "\nNewShare: " << info;

    if (isValidShare(info)) {
        const auto &&name = info.value(ShareInfoKeys::kName).toString();
        // 是否包含了非法字符：%<>*?|/\\+=;:,\"，且不能以 "-" 和空格开头，或者空格结尾
        QRegularExpression regex(R"(^(?![ -])[^%<>*?|/\\+=;:,"]*$(?<! ))");
        if (!regex.match(name).hasMatch()) {
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
            fmWarning() << "Share dir and run net cmd error: " << err;
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
    // Create anonymous pipe for secure credential transmission
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        fmInfo() << "Failed to create anonymous pipe for credentials";
        Q_EMIT sambaPasswordSet(false);
        return;
    }

    // Prepare credentials data using QDataStream for reliable serialization
    QByteArray credentials;
    QDataStream stream(&credentials, QIODevice::WriteOnly);
    stream << userName << FileUtils::encryptString(passwd);

    // Write credentials to pipe and close write end immediately
    ssize_t written = write(pipefd[1], credentials.constData(), credentials.size());
    close(pipefd[1]);   // Close write end immediately after writing

    if (written != credentials.size()) {
        fmInfo() << "Failed to write credentials to pipe, written:" << written << "expected:" << credentials.size();
        close(pipefd[0]);
        Q_EMIT sambaPasswordSet(false);
        return;
    }

    // Create file descriptor for D-Bus transmission
    QDBusUnixFileDescriptor fd(pipefd[0]);
    if (!fd.isValid()) {
        fmInfo() << "Failed to create valid file descriptor from pipe";
        close(pipefd[0]);
        Q_EMIT sambaPasswordSet(false);
        return;
    }

    // Call D-Bus interface with pipe file descriptor
    QDBusInterface *interface = getUserShareInterface();
    QDBusReply<bool> reply = interface->call(DaemonServiceIFace::kFuncSetPasswd, QVariant::fromValue(fd));
    bool success = reply.isValid() && reply.value();
    fmInfo() << "Samba password set result:" << success
             << ", error msg:" << (reply.isValid() ? "none" : reply.error().message());

    // Close read end (D-Bus service will have its own copy)
    close(pipefd[0]);

    Q_EMIT sambaPasswordSet(success);
}

bool UserShareHelper::removeShareByPath(const QString &path)
{
    const QString &&shareName = shareNameByPath(path);
    if (!shareName.isEmpty())
        return removeShareByShareName(shareName);
    return false;
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
    // 使用 SystemServiceManager 统一检查服务状态（遵循 DRY 原则）
    return SystemServiceManager::instance().isServiceRunning("smbd.service");
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
    QString ip = "127.0.0.1";
    QDBusInterface networkIface("org.freedesktop.NetworkManager",
                                "/org/freedesktop/NetworkManager",
                                "org.freedesktop.NetworkManager",
                                QDBusConnection::systemBus());

    auto primaryConn = networkIface.property("PrimaryConnection").value<QDBusObjectPath>();
    QDBusInterface actIface("org.freedesktop.NetworkManager",
                            primaryConn.path(),
                            "org.freedesktop.NetworkManager.Connection.Active",
                            QDBusConnection::systemBus());

    auto ipv4CfgPath = actIface.property("Ip4Config").value<QDBusObjectPath>();
    if (ipv4CfgPath.path().isEmpty()) {
        fmInfo() << "got invalid ipv4config in" << primaryConn.path();
        return ip;
    }

    QDBusInterface ipConfigIface("org.freedesktop.NetworkManager",
                                 ipv4CfgPath.path(),
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = ipConfigIface.call("Get",
                                                    "org.freedesktop.NetworkManager.IP4Config",
                                                    "AddressData");
    if (!reply.isValid()) {
        fmWarning() << "Failed to get AddressData:" << reply.error();
        return ip;
    }

    // 解析 aa{sv} 类型
    const QDBusArgument arg = reply.value().value<QDBusArgument>();
    if (arg.currentType() != QDBusArgument::ArrayType) {
        return ip;
    }

    arg.beginArray();
    while (!arg.atEnd()) {
        // 开始处理内层数组元素
        QVariantMap item;
        arg >> item;
        if (!item.value("address", "").toString().isEmpty()) {
            ip = item.value("address").toString();
            break;
        }
    }
    arg.endArray();

    return ip;
}

int UserShareHelper::getSharePort() const
{
    QSettings smbConf("/etc/samba/smb.conf", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    auto ports = smbConf.value("global/smb ports").toString().split(" ", QString::SkipEmptyParts);
#else
    auto ports = smbConf.value("global/smb ports").toString().split(" ", Qt::SkipEmptyParts);
#endif
    return ports.isEmpty() ? -1 : ports.first().toInt();
}

UserShareHelper::~UserShareHelper()
{
}

bool UserShareHelper::canShare(FileInfoPointer info)
{
    if (!info || !info->isAttributes(OptInfoType::kIsDir) || !info->isAttributes(OptInfoType::kIsReadable))
        return false;

    if (DevProxyMng->isFileOfProtocolMounts(info->pathOf(PathInfoType::kCanonicalPath)))
        return false;

    if (info->urlOf(UrlInfoType::kUrl).scheme() == Global::Scheme::kBurn || DevProxyMng->isFileFromOptical(info->pathOf(PathInfoType::kCanonicalPath)))
        return false;

    return true;
}

bool UserShareHelper::needDisableShareWidget(FileInfoPointer info)
{
    return (info && info->extendAttributes(ExtInfoType::kOwnerId).toUInt() != static_cast<uint>(SysInfoUtils::getUserId()) && !SysInfoUtils::isRootUser());
}

bool UserShareHelper::isUserSharePasswordSet(const QString &username)
{
    QDBusInterface *interface = getUserShareInterface();
    QDBusReply<bool> reply = interface->call(DaemonServiceIFace::kFuncIsPasswordSet, username);
    bool result = reply.isValid() ? reply.value() : false;
    fmDebug() << "isSharePasswordSet result: " << result << ", error: " << reply.error();

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
        if (fileInfo.groupId() != SysInfoUtils::getUserId())
            continue;

        QString filePath = fileInfo.absoluteFilePath();
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            fmWarning() << "open share file failed: " << filePath;
            return;
        }

        QMap<QString, QString> info;
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
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

bool UserShareHelper::removeShareByShareName(const QString &name, bool silent)
{
    QDBusInterface *interface = getUserShareInterface();
    QDBusReply<bool> reply = interface->asyncCall(DaemonServiceIFace::kFuncCloseShare, name, !silent);
    if (reply.isValid() && reply.value()) {
        fmDebug() << "share closed: " << name;
        runNetCmd(QStringList() << "usershare"
                                << "delete" << name);
        return true;
    }

    fmWarning() << "share close failed: " << name << ", " << reply.error();
    // TODO(xust) regular user cannot remove the sharing which shared by root user. and should raise an error dialog to notify user.
    return false;
}

void UserShareHelper::removeShareWhenShareFolderDeleted(const QString &deletedPath)
{
    const QString &&shareName = shareNameByPath(deletedPath);
    if (shareName.isEmpty())
        return;

    removeShareByShareName(shareName, true);
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
    fmDebug() << "usershare params:" << args;
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

    // 端口被禁用
    if (err.contains("net usershare add: cannot convert name") && err.contains("{Device Timeout}")) {
        NetworkUtils::instance()->doAfterCheckNet(
                "127.0.0.1", { "139", "445" },
                [](bool result) {
                    if (result) {
                        DialogManagerInstance->showErrorDialog(tr("Sharing failed"), "");
                    } else {
                        DialogManagerInstance->showErrorDialog(tr("Sharing failed"),
                                                               tr("SMB port is banned, please check the firewall strategy."));
                    }
                },
                500);
        return;
    }

    // 计算机名称过长会报错
    if (err.contains("gethostname failed") && err.contains("net usershare add: cannot convert name")) {
        DialogManagerInstance->showErrorDialog(tr("Sharing failed"), tr("The computer name is too long"));
        return;
    }

    DialogManagerInstance->showErrorDialog(QString(), err);
    fmWarning() << "run net command failed: " << err << ", code is: " << code;
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
    QString errMsg;
    // 使用 SystemServiceManager 统一管理服务启动和启用
    bool smbdOk = SystemServiceManager::instance().enableServiceNow("smbd.service");
    if (!smbdOk) {
        errMsg = "Failed to start and enable smbd.service";
        return { false, errMsg };
    }

    bool nmbdOk = SystemServiceManager::instance().enableServiceNow("nmbd.service");
    if (!nmbdOk) {
        errMsg = "Failed to start and enable nmbd.service";
        return { false, errMsg };
    }

    return { true, "" };
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
    // D-Bus interface will be lazily initialized on first use to avoid unnecessary service activation
    watcherManager = new ShareWatcherManager(this);
    watcherManager->add(ShareConfig::kShareConfigPath);

    initConnect();
    readShareInfos();

    initMonitorPath();
}

QDBusInterface *UserShareHelper::getUserShareInterface()
{
    if (!userShareInter) {
        fmInfo() << "Initializing UserShare D-Bus interface on first use";
        userShareInter.reset(new QDBusInterface(
                DaemonServiceIFace::kInterfaceService,
                DaemonServiceIFace::kInterfacePath,
                DaemonServiceIFace::kInterfaceInterface,
                QDBusConnection::systemBus(),
                this));

        if (!userShareInter->isValid()) {
            fmWarning() << "Failed to create UserShare D-Bus interface:" << userShareInter->lastError().message();
        }
    }
    return userShareInter.data();
}
}
