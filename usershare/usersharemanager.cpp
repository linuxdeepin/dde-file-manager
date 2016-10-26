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

#include <simpleini/SimpleIni.h>
#include "shareinfo.h"

UserShareManager::UserShareManager(QObject *parent) : QObject(parent)
{
    m_fileMonitor = new FileMonitor(this);
    m_fileMonitor->addMonitorPath(UserSharePath());
    m_shareInfosChangedTimer = new QTimer(this);
    m_shareInfosChangedTimer->setSingleShot(true);
    m_shareInfosChangedTimer->setInterval(300);
    initConnect();
    updateUserShareInfo();
    initMonitorPath();
}

UserShareManager::~UserShareManager()
{

}

void UserShareManager::initMonitorPath()
{
    const ShareInfoList& infoList = shareInfoList();
    for(auto info : infoList){
        m_fileMonitor->addMonitorPath(info.path());
    }
}

void UserShareManager::initConnect()
{
    connect(m_fileMonitor, &FileMonitor::fileDeleted, this, &UserShareManager::onFileDeleted);
    connect(m_fileMonitor, &FileMonitor::fileCreated, this, &UserShareManager::handleShareChanged);
    connect(m_shareInfosChangedTimer, &QTimer::timeout, this, &UserShareManager::updateUserShareInfo);
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
    if (!shareName.isEmpty()){
        if (m_shareInfos.contains(shareName)){
            return m_shareInfos.value(shareName);
        }
    }
    return ShareInfo();
}

ShareInfo UserShareManager::getsShareInfoByShareName(const QString &shareName) const
{
    return m_shareInfos.value(shareName);
}

QString UserShareManager::getShareNameByPath(const QString &path) const
{
    QString shareName;
    if (m_sharePathToNames.contains(path)){
        QStringList shareNames = m_sharePathToNames.value(path);
        if (shareNames.count() > 0){
            shareName = shareNames.last();
        }
    }
    return shareName;
}

void UserShareManager::loadUserShareInfoPathNames()
{
    QString cache = readCacheFromFile(getCacehPath());
    if (!cache.isEmpty()){
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(cache.toLocal8Bit(),&error);
        if (error.error == QJsonParseError::NoError){
            QJsonObject obj = doc.object();
            foreach (QString key, obj.keys()) {
                m_sharePathByFilePath.insert(key, obj.value(key).toString());
            }
        }else{
            qDebug() << "load cache file: " << getCacehPath() << error.errorString();
        }
    }
}

void UserShareManager::saveUserShareInfoPathNames()
{
    QVariantMap cache;
    foreach (const QString& path, m_sharePathByFilePath.keys()) {
        cache.insert(path, m_sharePathByFilePath.value(path));
    }

    QJsonDocument doc(QJsonObject::fromVariantMap(cache));
    writeCacheToFile(getCacehPath(), doc.toJson());
}

void UserShareManager::writeCacheToFile(const QString &path, const QString &content)
{
    QFile file(path);
    if (file.open(QFile::WriteOnly)){
        file.write(content.toLocal8Bit());
    }
    file.close();
}

QString UserShareManager::readCacheFromFile(const QString &path)
{
    QFile file(path);
    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << path << "isn't exists!";
        return QString();
    }
    QByteArray content = file.readAll();
    file.close();
    return QString(content);
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
    for (auto info: shareInfoList()){
        if(info.isValid())
            counter ++;
    }
    return counter;
}

bool UserShareManager::hasValidShareFolders() const
{
    foreach (const ShareInfo& info, shareInfoList()) {
        if(QFile::exists(info.path()))
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
}

void UserShareManager::updateUserShareInfo()
{
    //cache
    QStringList oldShareInfos = m_shareInfos.keys();
    QMap<QString,ShareInfo> shareInfoCache = m_shareInfos;

    m_shareInfos.clear();
    m_sharePathToNames.clear();

    QDir d(UserSharePath());
    QFileInfoList infolist = d.entryInfoList(QDir::Files);
    foreach (const QFileInfo& f, infolist) {
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
            if (line.contains("=")){
                int index = line.indexOf("=");
                QString key = line.mid(0, index);
                QString value = line.mid(index+1);
                info.insert(key, value);
            }
        }
        QString shareName = info.value("sharename");
        QString sharePath = info.value("path");
        QString share_acl = info.value("usershare_acl");
        if (!shareName.isEmpty() &&
                !sharePath.isEmpty() &&
                QFile(sharePath).exists() &&
                !share_acl.isEmpty()){
            shareInfo.setShareName(shareName);
            shareInfo.setPath(sharePath);
            shareInfo.setComment(info.value("comment"));
            shareInfo.setGuest_ok(info.value("guest_ok"));
            shareInfo.setUsershare_acl(info.value("usershare_acl"));
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
            emit userShareAdded(info.path());
            m_fileMonitor->addMonitorPath(info.path());
        }else if(info.isValid() && oldShareInfos.contains(info.shareName())){
            oldShareInfos.removeOne(info.shareName());
        }

    }

    // emit deleted usershare
    for (const QString &shareName : oldShareInfos){
        const QString& filePath = shareInfoCache.value(shareName).path();
        emit userShareDeleted(filePath);
        m_fileMonitor->removeMonitorPath(filePath);
    }
    usershareCountchanged();
}

void UserShareManager::testUpdateUserShareInfo()
{
    QProcess net_usershare_info;
    net_usershare_info.start("net usershare info");
    if (net_usershare_info.waitForFinished()){
        QString content(net_usershare_info.readAll());
        writeCacheToFile(getCacehPath(), content);
        qDebug() << content;
        QSettings settings(getCacehPath(), QSettings::IniFormat);
        settings.setIniCodec("utf-8");
        qDebug() << settings.childGroups();
        foreach (QString group, settings.childGroups()) {
            settings.beginGroup(group);
            qDebug() << settings.value("path").toString();
            settings.endGroup();
        }
    }
}

void UserShareManager::addUserShare(const ShareInfo &info)
{
    //handle old info
    ShareInfo oldInfo = getOldShareInfoByNewInfo(info);
    qDebug() << oldInfo << info;
    if(oldInfo.isValid()){
        deleteUserShare(oldInfo);
    }
    if (!info.shareName().isEmpty() && QFile(info.path()).exists()){
        QString cmd = "net";
        QStringList args;
        args << "usershare" << "add"
             << info.shareName() << info.path()
             << info.comment() << info.usershare_acl()
             << info.guest_ok();
        bool ret = QProcess::startDetached(cmd, args);

        if(info.isWritable()){
            QString cmd = "chmod";
            QStringList args;
            args << "-R"<<"777"<<info.path();
            ret = QProcess::startDetached(cmd, args);
        }
        else {
            QString cmd = "chmod";
            QStringList args;
            args << "-R"<<"755"<<info.path();
            ret = QProcess::startDetached(cmd, args);
        }

        if (ret){
            qDebug() << info.path();
        }
    }
}

void UserShareManager::deleteUserShareByShareName(const QString &shareName)
{
    QString cmd = "net";
    QStringList args;
    args << "usershare" << "delete"
         << shareName;
    QProcess p;
    p.start(cmd, args);
    p.waitForFinished();
}

void UserShareManager::deleteUserShare(const ShareInfo &info)
{
    if (!info.shareName().isEmpty()){
        QStringList names = m_sharePathToNames.value(info.path());
        names.removeOne(info.shareName());
        m_sharePathToNames.insert(info.path(), names);
        deleteUserShareByShareName(info.shareName());
    }
}

void UserShareManager::deleteUserShareByPath(const QString &path)
{
    QString shareName = getShareNameByPath(path);
    if (!shareName.isEmpty()){
        deleteUserShareByShareName(shareName);
    }
}

void UserShareManager::onFileDeleted(const QString &filePath)
{
    if(filePath.contains(UserSharePath()))
        handleShareChanged(filePath);
    else
        deleteUserShareByPath(filePath);
}

void UserShareManager::usershareCountchanged()
{
    int count = validShareInfoCount();
    emit userShareCountChanged(count);
}

