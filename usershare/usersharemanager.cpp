#include "usersharemanager.h"
#include <QProcess>
#include <QStandardPaths>
#include <QFile>
#include <QApplication>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include <simpleini/SimpleIni.h>
#include "shareinfo.h"

UserShareManager::UserShareManager(QObject *parent) : QObject(parent)
{
    m_fileSystemWatcher = new QFileSystemWatcher(this);
    m_fileSystemWatcher->addPath(UserSharePath());
    initConnect();
    updateUserShareInfo();
    loadUserShareInfoPathNames();

//    ShareInfo info;
//    info.setShareName("11111");
//    info.setPath("/home/djf/Desktop");
//    addUserShare(info);
//    deleteUserShareByPath("/home/djf/Desktop");
}

UserShareManager::~UserShareManager()
{

}

void UserShareManager::initConnect()
{
    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &UserShareManager::handleShareChanged);
}

QString UserShareManager::getCacehPath()
{
    return QString("%1/.cache/%2/usershare.json").arg(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0),
                                                      qApp->applicationName());
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

    if (m_sharePathByFilePath.contains(path)){
        shareName = m_sharePathByFilePath.value(path);
    }else{
        if (m_sharePathToNames.contains(path)){
            QStringList shareNames = m_sharePathToNames.value(path);
            if (shareNames.count() > 0){
                shareName = shareNames.last();
            }
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
        shareList << m_shareInfos.value(key);
    }

    return shareList;
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

void UserShareManager::handleShareChanged()
{
    updateUserShareInfo();
    emit userShareChanged(m_shareInfos.count());
}

void UserShareManager::updateUserShareInfo()
{
    QProcess net_usershare_info;
    net_usershare_info.start("net usershare info");
    if (net_usershare_info.waitForFinished()){

        CSimpleIniA settings;
        settings.SetUnicode(true);
        QString content(net_usershare_info.readAll());
        settings.LoadData(content.toStdString().c_str(), content.length());

        CSimpleIniA::TNamesDepend sections;
        settings.GetAllSections(sections);
        CSimpleIniA::TNamesDepend::iterator i;

        QStringList oldShareInfos = m_shareInfos.keys();

        m_shareInfos.clear();
        m_sharePathToNames.clear();

        for (i = sections.begin(); i != sections.end(); ++i){
            CSimpleIniA::Entry sectionEntry = *i;
            CSimpleIniA::TNamesDepend keys;
            settings.GetAllKeys(sectionEntry.pItem, keys);
            CSimpleIniA::TNamesDepend::iterator j;

            ShareInfo info;
            info.setShareName(sectionEntry.pItem);
            for (j = keys.begin(); j != keys.end(); ++j){
                CSimpleIniA::Entry keyEntry = *j;
                const char * value = settings.GetValue(sectionEntry.pItem, keyEntry.pItem);
                if (QString(keyEntry.pItem) == "comment"){
                    info.setComment(value);
                }else if (QString(keyEntry.pItem) == "path"){
                    info.setPath(value);
                }else if (QString(keyEntry.pItem) == "usershare_acl"){
                    info.setUsershare_acl(value);
                }else if (QString(keyEntry.pItem) == "guest_ok"){
                    info.setGuest_ok(value);
                }
            }

            if (info.isValid() && !oldShareInfos.contains(info.path())) {
                oldShareInfos.removeOne(info.path());

                emit userShareAdded(info.path());
            }

            m_shareInfos.insert(info.shareName(), info);

            if (m_sharePathToNames.contains(info.path())) {
                QStringList names = m_sharePathToNames.value(info.path());
                names.append(info.shareName());
                m_sharePathToNames.insert(info.path(), names);
            } else {
                QStringList names;
                names.append(info.shareName());
                m_sharePathToNames.insert(info.path(), names);
            }
        }

        for (const QString &path : oldShareInfos)
            emit userShareDeleted(path);
    }
}

void UserShareManager::addUserShare(const ShareInfo &info)
{
    //handle old info
    ShareInfo oldInfo = getShareInfoByPath(info.path());
    if(oldInfo.isValid())
        deleteUserShare(oldInfo);

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
            m_sharePathByFilePath.insert(info.path(), info.shareName());
            saveUserShareInfoPathNames();
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
        deleteUserShareByShareName(info.shareName());
    }
}

void UserShareManager::deleteUserShareByPath(const QString &path)
{
    if (m_sharePathByFilePath.contains(path)){
        deleteUserShareByShareName(m_sharePathByFilePath.value(path));
        m_sharePathByFilePath.remove(path);
        saveUserShareInfoPathNames();
    }else{
        QString shareName = getShareNameByPath(path);
        if (!shareName.isEmpty()){
            deleteUserShareByShareName(shareName);
        }
    }
}

