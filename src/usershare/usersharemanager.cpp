/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "usersharemanager.h"
#include <QProcess>
#include <QStandardPaths>
#include <QFile>
#include <QApplication>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDir>
#include <QDebug>

#include <unistd.h>
#include <pwd.h>

#include "shareinfo.h"
#include "../dde-file-manager-daemon/dbusservice/dbusinterface/usershare_interface.h"

#include "dfilewatchermanager.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "dabstractfilewatcher.h"
#include "utils/singleton.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dialogs/dialogmanager.h"
#include "shutil/fileutils.h"
#include "ddialog.h"

DWIDGET_USE_NAMESPACE
QString UserShareManager::CurrentUser = "";

UserShareManager::UserShareManager(QObject *parent) : QObject(parent)
{
    m_fileMonitor = new DFileWatcherManager(this);
    m_fileMonitor->add(UserSharePath());
    m_shareInfosChangedTimer = new QTimer(this);
    m_shareInfosChangedTimer->setSingleShot(true);
    m_shareInfosChangedTimer->setInterval(300);

    m_userShareInterface = new UserShareInterface("com.deepin.filemanager.daemon",
                                                  "/com/deepin/filemanager/daemon/UserShareManager",
                                                  QDBusConnection::systemBus(),
                                                  this);
    initConnect();
    updateUserShareInfo();
    initMonitorPath();

    connect(this, &UserShareManager::userShareAdded, this, &UserShareManager::updateFileAttributeInfo);
    connect(this, &UserShareManager::userShareDeleted, this, &UserShareManager::updateFileAttributeInfo);
}

UserShareManager::~UserShareManager()
{

}

void UserShareManager::initMonitorPath()
{
    const ShareInfoList &infoList = shareInfoList();
    for (auto info : infoList) {
        m_fileMonitor->add(info.path());
    }
}

void UserShareManager::initConnect()
{
    connect(m_fileMonitor, &DFileWatcherManager::fileDeleted, this, &UserShareManager::onFileDeleted);
    connect(m_fileMonitor, &DFileWatcherManager::subfileCreated, this, &UserShareManager::handleShareChanged);
    connect(m_fileMonitor, &DFileWatcherManager::fileMoved, this, [this](const QString & from, const QString & to) {
        onFileDeleted(from);
        handleShareChanged(to);
    });
    connect(m_shareInfosChangedTimer, &QTimer::timeout, this, [this]() {emit updateUserShareInfo(true);});
}

QString UserShareManager::getCacehPath()
{
    return QString("%1/.cache/%2/usershare.json").arg(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0),
                                                      qApp->applicationName());
}

ShareInfo UserShareManager::getOldShareInfoByNewInfo(const ShareInfo &newInfo) const
{
    QStringList shareNames = m_sharePathToNames.value(newInfo.path());
    shareNames.removeOne(newInfo.shareName());
    if (shareNames.count() > 0)
        return getsShareInfoByShareName(shareNames.last());
    return ShareInfo();
}

ShareInfo UserShareManager::getShareInfoByPath(const QString &path) const
{
    QString shareName = getShareNameByPath(path);
    if (!shareName.isEmpty()) {
        if (m_shareInfos.contains(shareName)) {
            return m_shareInfos.value(shareName);
        }
    }
    return ShareInfo();
}

ShareInfo UserShareManager::getsShareInfoByShareName(const QString &shareName) const
{
    std::string stdStr = shareName.toStdString();
    return m_shareInfos.value(QUrl::fromPercentEncoding(stdStr.data()));
}

QString UserShareManager::getShareNameByPath(const QString &path) const
{
    QString shareName;
    if (m_sharePathToNames.contains(path)) {
        QStringList shareNames = m_sharePathToNames.value(path);
        if (shareNames.count() > 0) {
            shareName = shareNames.last();
        }
    }
    return shareName;
}

uint UserShareManager::getCreatorUidByShareName(const QString &shareName) const
{
    QFileInfo info(UserSharePath() + "/" + shareName);
    return info.ownerId();
}

void UserShareManager::loadUserShareInfoPathNames()
{
    QString cache = readCacheFromFile(getCacehPath());
    if (!cache.isEmpty()) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(cache.toLocal8Bit(), &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonObject obj = doc.object();
            foreach (QString key, obj.keys()) {
                m_sharePathByFilePath.insert(key, obj.value(key).toString());
            }
        } else {
            qDebug() << "load cache file: " << getCacehPath() << error.errorString();
        }
    }
}

void UserShareManager::saveUserShareInfoPathNames()
{
    QVariantMap cache;
    foreach (const QString &path, m_sharePathByFilePath.keys()) {
        cache.insert(path, m_sharePathByFilePath.value(path));
    }

    QJsonDocument doc(QJsonObject::fromVariantMap(cache));
    writeCacheToFile(getCacehPath(), doc.toJson());
}

void UserShareManager::updateFileAttributeInfo(const QString &filePath) const
{
    const DUrl &fileUrl = DUrl::fromLocalFile(filePath);
    qDebug() << fileUrl;
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, fileUrl);

    if (!fileInfo)
        return;
    qDebug() << fileInfo->parentUrl();
    DAbstractFileWatcher::ghostSignal(fileInfo->parentUrl(), &DAbstractFileWatcher::fileAttributeChanged, fileUrl);
}

void UserShareManager::startSambaServiceAsync()
{
    QDBusInterface interface("org.freedesktop.systemd1",
                             "/org/freedesktop/systemd1/unit/smbd_2eservice",
                             "org.freedesktop.systemd1.Unit",
                             QDBusConnection::systemBus());

    QDBusPendingCall pcall = interface.asyncCall(QLatin1String("Start"), QString("replace"));

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, &UserShareManager::callFinishedSlot);
}

void UserShareManager::writeCacheToFile(const QString &path, const QString &content)
{
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        file.write(content.toLocal8Bit());
    }
    file.close();
}

QString UserShareManager::readCacheFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << path << "isn't exists!";
        return QString();
    }
    QByteArray content = file.readAll();
    file.close();
    return QString(content);
}

QString UserShareManager::getCurrentUserName()
{
    CurrentUser = getpwuid(getuid())->pw_name; //getpwuid get password uid，pw_name password name，这个用来获取uid对应的用户名
    return CurrentUser;
}

void UserShareManager::initSamaServiceSettings()
{
    //    addCurrentUserToSambashareGroup();
    //    restartSambaService();
}

ShareInfoList UserShareManager::shareInfoList() const
{
    ShareInfoList shareList;
    QList<QString> keys = m_shareInfos.keys();
    foreach (QString key, keys) {
        shareList << m_shareInfos[key];
    }

    return shareList;
}

int UserShareManager::validShareInfoCount() const
{
    int counter = 0;
    for (auto info : shareInfoList()) {
        if (info.isValid())
            counter ++;
    }
    return counter;
}

bool UserShareManager::hasValidShareFolders() const
{
    foreach (const ShareInfo &info, shareInfoList()) {
        if (QFile::exists(info.path()))
            return true;
    }
    return false;
}

bool UserShareManager::isShareFile(const QString &filePath) const
{
    return m_sharePathToNames.contains(filePath);
}

void UserShareManager::handleShareChanged(const QString &filePath)
{
    if (filePath.contains(":tmp"))
        return;
    m_shareInfosChangedTimer->start();
    QTimer::singleShot(1000, this, [ = ]() {
        emit fileSignalManager->requestRefreshFileModel(DUrl::fromUserShareFile("/"));
    });
}

void UserShareManager::updateUserShareInfo(bool sendSignal)
{
    //cache
    QStringList oldShareInfos = m_shareInfos.keys();
    QMap<QString, ShareInfo> shareInfoCache = m_shareInfos;
    ShareInfoList newInfos;

    m_shareInfos.clear();
    m_sharePathToNames.clear();

    QDir d(UserSharePath());
    // 修复BUG-46217 增加筛选条件，将以"."开头的文件筛选出来
    QFileInfoList infolist = d.entryInfoList(QDir::Files | QDir::Hidden);
    foreach (const QFileInfo &f, infolist) {
        ShareInfo shareInfo;
        QMap<QString, QString> info;
        QString fpath = f.absoluteFilePath();
        QFile file(fpath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Readonly" << fpath << "failed";
            return;
        }
        QTextStream in(&file);
        while (!in.atEnd()) {
            // Read new line
            QString line = in.readLine();
            // Skip empty line or line with invalid format
            if (line.trimmed().isEmpty()) {
                continue;
            }
            if (line.contains("=")) {
                int index = line.indexOf("=");
                QString key = line.mid(0, index);
                QString value = line.mid(index + 1);
                info.insert(key, value);
            }
        }
        QString shareName = info.value("sharename").toLower();
        QString sharePath = info.value("path");
        QString share_acl = info.value("usershare_acl");
        if (!shareName.isEmpty() &&
                !sharePath.isEmpty() &&
                QFile(sharePath).exists() &&
                !share_acl.isEmpty()) {
            shareInfo.setShareName(shareName);
            shareInfo.setPath(sharePath);
            shareInfo.setComment(info.value("comment"));
            shareInfo.setGuest_ok(info.value("guest_ok"));
            shareInfo.setUsershare_acl(info.value("usershare_acl"));
            if (share_acl.contains("r") || share_acl.contains("R")) {
                shareInfo.setIsWritable(false);
            } else if (share_acl.contains("f") || share_acl.contains("F")) {
                shareInfo.setIsWritable(true);
            }
            m_shareInfos.insert(shareInfo.shareName(), shareInfo);

            if (m_sharePathToNames.contains(shareInfo.path())) {
                QStringList names = m_sharePathToNames.value(shareInfo.path());
                names.append(shareInfo.shareName());
                m_sharePathToNames.insert(shareInfo.path(), names);
            } else {
                QStringList names;
                names.append(shareInfo.shareName());
                m_sharePathToNames.insert(shareInfo.path(), names);
            }
        }
    }

    foreach (ShareInfo info, m_shareInfos.values()) {
        if (info.isValid() && !oldShareInfos.contains(info.shareName())) {
            newInfos << info;
        } else if (info.isValid() && oldShareInfos.contains(info.shareName())) {
            oldShareInfos.removeOne(info.shareName());
        }
    }

    // emit deleted usershare
    for (const QString &shareName : oldShareInfos) {
        const QString &filePath = shareInfoCache.value(shareName).path();
        emit userShareDeleted(filePath);
        m_fileMonitor->remove(filePath);
    }

    //emit new encoming shared info
    foreach (const ShareInfo &info, newInfos) {
        emit userShareAdded(info.path());
        m_fileMonitor->add(info.path());
    }

    if (!sendSignal) {
        return;
    }

    // send signal.
    if (validShareInfoCount() <= 0) {
        emit userShareDeleted("/");
    }
    usershareCountchanged();
}

void UserShareManager::setSambaPassword(const QString &userName, const QString &password)
{
    QDBusReply<bool> reply = m_userShareInterface->setUserSharePassword(userName, password);
    if (reply.isValid()) {
        qDebug() << "set usershare password:" << reply.value();
    } else {
        qDebug() << "set usershare password:" << reply.error();
    }
}

bool UserShareManager::addUserShare(const ShareInfo &info)
{
    if (!FileUtils::isSambaServiceRunning()) {
        m_currentInfo = info;
        startSambaServiceAsync();

        return false;
    }

    // check if we got `net` (in `samba-common-bin` package) installed
    QString samba_common_bin_path = QStandardPaths::findExecutable("net");
    if (samba_common_bin_path.isEmpty()) {
        dialogManager->showErrorDialog(tr("Kindly Reminder"), tr("Please firstly install samba to continue"));
        return false;
    }

    // handle old info
    ShareInfo oldInfo = getOldShareInfoByNewInfo(info);
    qDebug() << oldInfo << info;
    if (!info.shareName().isEmpty() && QFile(info.path()).exists()) {
        // 共享文件的共享名不能以-开头
        if (info.shareName().startsWith("-") || info.shareName().endsWith(" ")) {
            dialogManager->showErrorDialog(tr("The share name must not contain %<>*?|/\\+=;:,\" and should not start with %1").arg("-"), "");
            return false;
        }
        QString cmd = "net";
        QStringList args;
        ShareInfo _info = info;
        if (_info.isWritable()) {
            _info.setUsershare_acl("Everyone:f");
        } else {
            _info.setUsershare_acl("Everyone:R");
        }
        args << "usershare" << "add"
             << _info.shareName() << _info.path()
             << _info.comment() << _info.usershare_acl()
             << _info.guest_ok();


        QProcess process;
        process.start(cmd, args);
        // Wait for process to finish without timeout.
        process.waitForFinished(-1);

        if (process.exitCode() != 0) {
            QString err = process.readAllStandardError();

            if (err.contains("is already a valid system user name")) {
                emit fileSignalManager->requestShowAddUserShareFailedDialog(_info.path());
                return false;
            }

            //root权限文件分享会报这个错误信息
            if (err.contains("as we are restricted to only sharing directories we own.")) {
                dialogManager->showErrorDialog(tr("To protect the files, you cannot share this folder."), "");
                return false;
            }

            // 共享文件的共享名输入特殊字符会报这个错误信息
            if (err.contains("contains invalid characters")) {
                dialogManager->showErrorDialog(tr("The share name must not contain %<>*?|/\\+=;:,\" and should not start with %1").arg("-"), "");
                return false;
            }

            // net usershare add: failed to add share sharename. Error was 文件名过长
            // 共享文件的共享名太长，会报上面这个错误信息，最后居然还是中文
            // another fix: 有多种问题会报上面的错误信息，该错误信息最后的错误描述是系统翻译后的文本，所以这里改成直接显示命令返回的错误描述。
            if (err.contains("net usershare add: failed to add share") && err.contains("Error was ")) {
                DDialog dialog;
                QString errorDisc = err.split("Error was ").last();
                errorDisc = errorDisc.remove("\n");
                dialogManager->showErrorDialog(errorDisc, "");
                return false;
            }

            //计算机名称过长会报错
            if (err.contains("gethostname failed") && err.contains("net usershare add: cannot convert name")) {
                dialogManager->showErrorDialog(tr("Sharing failed"), tr("The computer name is too long"));
                return false;
            }

            qWarning() << err << "err code = " << QString::number(process.exitCode());

            dialogManager->showErrorDialog(QString(), err);
            return false;
        }
        if (oldInfo.isValid()) {
            deleteUserShareByPath(oldInfo.path());
        }

        return true;
    }
    return false;
}


void UserShareManager::deleteUserShareByPath(const QString &path)
{
    QString shareName = getShareNameByPath(path);
    if (!shareName.isEmpty()) {
        deleteUserShareByShareName(shareName);
    }
}

void UserShareManager::removeFiledeleteUserShareByPath(const QString &path)
{
    QString shareName = getShareNameByPath(path);
    if (shareName.isEmpty()) {
        return;
    }
    QString cmd = "net";
    QStringList args;
    args << "usershare" << "delete"
         << shareName;
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished();
}

void UserShareManager::onFileDeleted(const QString &filePath)
{
    if (filePath.contains(UserSharePath()))
        handleShareChanged(filePath);
    else
        removeFiledeleteUserShareByPath(filePath);
}

void UserShareManager::usershareCountchanged()
{
    int count = validShareInfoCount();
    emit userShareCountChanged(count);
}

void UserShareManager::callFinishedSlot(QDBusPendingCallWatcher *watcher)
{
    QObject::disconnect(watcher, &QDBusPendingCallWatcher::finished, this, &UserShareManager::callFinishedSlot);

    QDBusPendingReply<> reply = *watcher;
    watcher->deleteLater();
    if (reply.isValid()) {
        const QString &errorMsg = reply.reply().errorMessage();
        if (errorMsg.isEmpty()) {
            qDebug() << "smbd service start success";

            // 自启动
            // 这里创建链接文件 以便下次开机自启
            QDBusReply<bool> reply = m_userShareInterface->createShareLinkFile();
            if (reply.isValid()) {
                qDebug() << "set usershare password:" << reply.value();
                if(reply.value())
                    addUserShare(m_currentInfo);
            } else {
                qDebug() << "set usershare password:" << reply.error();
            }
        }
    } else {
        dialogManager->showErrorDialog(QString(), QObject::tr("Failed to start Samba services"));
    }
}

void UserShareManager::deleteUserShareByShareName(const QString &shareName)
{
    QDBusReply<bool> reply = m_userShareInterface->closeSmbShareByShareName(shareName, true);
    if (reply.isValid() && reply.value()) {
        qDebug() << "closeSmbShareByShareName:" << reply.value();
    } else {
        qDebug() << "closeSmbShareByShareName:" << reply.error();
        QMap<QString, ShareInfo> shareInfoCache = m_shareInfos;
        if (shareInfoCache.contains(shareName)) {
            /*fix 64070 root用户共享的文件，普通用户去取消该共享的时候需要做弹窗处理*/
            QString filename = shareName.toLower(); //文件名小写
            auto getShareUid = getCreatorUidByShareName("/var/lib/samba/usershares/" + filename);
            if (DFMGlobal::getUserId() != getShareUid) { //对比文件属主与共享属主
                if (!DFMGlobal::isRootUser())
                    dialogManager->showErrorDialog(tr("You do not have permission to operate file/folder!"), QString());
            }
            const QString &filePath = shareInfoCache.value(shareName).path();
            emit userShareDeletedFailed(filePath);
        }
        return;
    }

    QString cmd = "net";
    QStringList args;
    args << "usershare" << "delete"
         << shareName;
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished();
}
