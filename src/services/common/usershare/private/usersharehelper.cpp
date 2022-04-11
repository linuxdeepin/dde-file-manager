/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "usersharehelper.h"
#include "sharewatchermanager.h"

#include "dfm-base/utils/dialogmanager.h"

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

#include <pwd.h>
#include <unistd.h>

DSC_BEGIN_NAMESPACE

// TODO(xust) this maybe changed later, use new interface instead
namespace DaemonServiceIFace {
static constexpr char kInterfaceService[] { "com.deepin.filemanager.daemon" };
static constexpr char kInterfacePath[] { "/com/deepin/filemanager/daemon/UserShareManager" };
static constexpr char kInterfaceInterface[] { "com.deepin.filemanager.daemon.UserShareManager" };

static constexpr char kFuncSetPasswd[] { "SetUserSharePassword" };
static constexpr char kFuncCloseShare[] { "CloseSmbShareByShareName" };
static constexpr char kFuncCreateShareLinkFile[] { "CreateShareLinkFile" };
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

    if (info.isValid()) {
        if (info.getShareName().startsWith("-") || info.getShareName().endsWith(" ")) {
            DialogManagerInstance->showErrorDialog(tr("The share name must not contain %1, and cannot start with a dash (-) or whitespace, or end with whitespace.").arg("%<>*?|/\\+=;:,\""), "");
            return false;
        }

        QStringList netArgs;
        auto _info = info;
        if (_info.getWritable())
            _info.setUserShareAcl("Everyone:f");
        else
            _info.setUserShareAcl("Everyone:R");

        netArgs << "usershare"
                << "add" << _info.getShareName() << _info.getPath() << _info.getComment()
                << _info.getUserShareAcl() << _info.getGuestEnable();

        QString err;
        int code = runNetCmd(netArgs, -1, &err);
        if (code != 0) {
            handleErrorWhenShareFailed(code, err);
            return false;
        }
    }

    if (oldShare.isValid())
        removeShareByPath(oldShare.getPath());

    return true;
}

void UserShareHelper::setSambaPasswd(const QString &userName, const QString &passwd)
{
    userShareInter->asyncCall(DaemonServiceIFace::kFuncSetPasswd, userName, passwd);
}

void UserShareHelper::removeShareByPath(const QString &path)
{
    const QString &&shareName = getShareNameByPath(path);
    if (!shareName.isEmpty())
        removeShareByShareName(shareName);
}

ShareInfoList UserShareHelper::shareInfos() const
{
    return sharedInfos.values();
}

ShareInfo UserShareHelper::getShareInfoByPath(const QString &path) const
{
    const QString &&shareName = getShareNameByPath(path);
    return getShareInfoByShareName(shareName);
}

ShareInfo UserShareHelper::getShareInfoByShareName(const QString &name) const
{
    if (!name.isEmpty() && sharedInfos.contains(name))
        return sharedInfos.value(name);
    return ShareInfo();
}

QString UserShareHelper::getShareNameByPath(const QString &path) const
{
    if (sharePathToShareName.contains(path)) {
        const auto &&names = sharePathToShareName.value(path);
        if (names.count() > 0)
            return names.last();
    }
    return "";
}

uint UserShareHelper::getUidByShareName(const QString &name) const
{
    QFileInfo info(QString("%1/%2").arg(ShareConfig::kShareConfigPath).arg(name));
    return info.ownerId();
}

bool UserShareHelper::isShared(const QString &path) const
{
    return sharePathToShareName.contains(path);
}

QString UserShareHelper::getCurrentUserName() const
{
    return getpwuid(getuid())->pw_name;
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

UserShareHelper::~UserShareHelper()
{
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
        if (shareInfo.isValid()) {
            sharedInfos.insert(shareInfo.getShareName(), shareInfo);

            QStringList names = sharePathToShareName.value(shareInfo.getPath(), {});
            names.append(shareInfo.getShareName());
            sharePathToShareName.insert(shareInfo.getPath(), names);
        }
    }

    ShareInfoList newShares;
    for (const auto &info : sharedInfos.values()) {
        if (info.isValid()) {
            if (!oldSharedInfoKeys.contains(info.getShareName()))
                newShares << info;
            else
                oldSharedInfoKeys.removeOne(info.getShareName());
        }
    }

    // broadcast deleted shares
    for (const QString &shareName : oldSharedInfoKeys) {
        const QString &filePath = shareInfoCache.value(shareName).getPath();
        Q_EMIT shareRemoved(filePath);
        watcherManager->remove(filePath);
    }

    // broadcast new shares
    for (const ShareInfo &info : newShares) {
        Q_EMIT shareAdded(info.getPath());
        watcherManager->add(info.getPath());
    }

    if (!sendSignal)
        return;

    int count = validShareInfoCount();
    if (count == 0)
        Q_EMIT shareRemoved("/");

    Q_EMIT shareCountChanged(count);
}

void UserShareHelper::onShareChanged(const QString &path)
{
    if (path.contains(":tmp"))
        return;

    pollingSharesTimer->start();
    QTimer::singleShot(1000, this, [=] { /*TODO(xust) TODO(liuyangming) request to refresh file view*/ });
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
        watcherManager->add(info.getPath());
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
    const QString &&shareName = getShareNameByPath(deletedPath);
    if (shareName.isEmpty())
        return;

    removeShareByShareName(shareName);
}

ShareInfo UserShareHelper::getOldShareByNewShare(const ShareInfo &newShare)
{
    QStringList shareNames = sharePathToShareName.value(newShare.getPath());
    shareNames.removeOne(newShare.getShareName());
    if (shareNames.count() > 0)
        return getShareInfoByShareName(shareNames.last());
    return ShareInfo();
}

int UserShareHelper::runNetCmd(const QStringList &args, int wait, QString *err)
{
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
    if (err.contains("is already a valid system user name")) {
        //        emit fileSignalManager->requestShowAddUserShareFailedDialog(_info.path());
        return;
    }

    //root权限文件分享会报这个错误信息
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
        DDialog dialog;
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
        info.setShareName(shareName.toLower());
        info.setPath(sharePath);
        info.setComment(contents.value(ShareConfig::kShareComment));
        info.setGuestEnable(contents.value(ShareConfig::kGuestOk));
        info.setUserShareAcl(shareAcl);

        if (shareAcl.toUpper().contains("R"))
            info.setWritable(false);
        else if (shareAcl.toUpper().contains("F"))
            info.setWritable(true);
    }
    return info;
}

int UserShareHelper::validShareInfoCount() const
{
    int count = 0;
    for (const auto &info : sharedInfos)
        count += (info.isValid() ? 1 : 0);
    return count;
}

QPair<bool, QString> UserShareHelper::startSmbService()
{
    QDBusInterface iface(SambaServiceIFace::kService, SambaServiceIFace::kPath, SambaServiceIFace::kInterface, QDBusConnection::systemBus());
    QDBusPendingReply<QString> reply = iface.asyncCall(SambaServiceIFace::kFuncStart, SambaServiceIFace::kParamReplace);
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
    QDBusReply<bool> reply = userShareInter->call(DaemonServiceIFace::kFuncCreateShareLinkFile);
    return reply.value();
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

DSC_END_NAMESPACE
