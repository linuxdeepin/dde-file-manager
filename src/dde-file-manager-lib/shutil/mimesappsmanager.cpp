/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include "mimesappsmanager.h"
#include "dmimedatabase.h"

#include "app/define.h"

#include "singleton.h"
#include "desktopfile.h"
#include "interfaces/dfmstandardpaths.h"
#include "interfaces/dfmglobal.h"
#include "mimetypedisplaymanager.h"

#include <QDir>
#include <QSettings>
#include <QMimeType>
#include <QDirIterator>
#include <QDateTime>
#include <QThread>
#include <QStandardPaths>
#include <QDebug>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>

#include "interfaces/durl.h"
#include "interfaces/dfileinfo.h"
#include "interfaces/dfileservices.h"
#include "fileutils.h"

#undef signals
extern "C" {
    #include <gio/gio.h>
    #include <gio/gdesktopappinfo.h>
    #include <gio/gappinfo.h>
}
#define signals public

DFM_USE_NAMESPACE

QStringList MimesAppsManager::DesktopFiles = {};
QMap<QString, QStringList> MimesAppsManager::MimeApps = {};
QMap<QString, QStringList> MimesAppsManager::DDE_MimeTypes = {};
QMap<QString, DesktopFile> MimesAppsManager::VideoMimeApps = {};
QMap<QString, DesktopFile> MimesAppsManager::ImageMimeApps = {};
QMap<QString, DesktopFile> MimesAppsManager::TextMimeApps = {};
QMap<QString, DesktopFile> MimesAppsManager::AudioMimeApps = {};

QMap<QString, DesktopFile> MimesAppsManager::DesktopObjs = {};

MimeAppsWorker::MimeAppsWorker(QObject *parent): QObject(parent)
{
    m_fileSystemWatcher = new QFileSystemWatcher;
    m_updateCacheTimer = new QTimer(this);
    m_updateCacheTimer->setInterval(2000);
    m_updateCacheTimer->setSingleShot(true);
    startWatch();
    initConnect();
}

MimeAppsWorker::~MimeAppsWorker()
{

}

void MimeAppsWorker::initConnect()
{
    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &MimeAppsWorker::handleDirectoryChanged);
    connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &MimeAppsWorker::handleFileChanged);
    connect(m_updateCacheTimer, &QTimer::timeout, this, &MimeAppsWorker::updateCache);
}

void MimeAppsWorker::startWatch()
{
    m_fileSystemWatcher->addPaths(MimesAppsManager::getDesktopFiles());
    m_fileSystemWatcher->addPaths(MimesAppsManager::getApplicationsFolders());
}

void MimeAppsWorker::handleDirectoryChanged(const QString &filePath)
{
    Q_UNUSED(filePath)

    //for 1.4
//    if(QFile::exists(filePath)){
//        m_fileSystemWatcher->addPath(filePath);

//        QMap<QString, QSet<QString>> mimeAppsSet;

//        DesktopFile desktopFile(filePath);
//        MimesAppsManager::DesktopFiles.append(filePath);
//        MimesAppsManager::DesktopObjs.insert(filePath, desktopFile);
//        QStringList mimeTypes = desktopFile.getMimeType();
//        foreach (QString mimeType, mimeTypes) {
//            if (!mimeType.isEmpty()){
//                QSet<QString> apps;
//                if (mimeAppsSet.contains(mimeType)){
//                    apps = mimeAppsSet.value(mimeType);
//                    apps.insert(filePath);
//                }else{
//                    apps.insert(filePath);
//                }
//                mimeAppsSet.insert(mimeType, apps);
//            }
//        }

//        foreach (QString key, mimeAppsSet.keys()) {
//            QSet<QString> apps = mimeAppsSet.value(key);
//            QStringList orderApps;
//            if (apps.count() > 1){
//                QFileInfoList fileInfos;
//                foreach (QString app, apps) {
//                    QFileInfo info(app);
//                    fileInfos.append(info);
//                }

//                std::sort(fileInfos.begin(), fileInfos.end(), MimesAppsManager::lessByDateTime);

//                foreach (QFileInfo info, fileInfos) {
//                    orderApps.append(info.absoluteFilePath());
//                }
//            }else{
//                orderApps.append(apps.toList());
//            }
//            MimesAppsManager::MimeApps.insert(key, orderApps);
//        }

//        foreach (QString desktopFile, MimesAppsManager::DesktopObjs.keys()) {
//            QString iconName = MimesAppsManager::DesktopObjs.value(desktopFile).getIcon();
//            fileIconProvider->getDesktopIcon(iconName, 48);
//        }

//    }else{
//        m_fileSystemWatcher->removePath(filePath);
//        MimesAppsManager::DesktopFiles.removeOne(filePath);
//        MimesAppsManager::DesktopObjs.remove(filePath);
//    }
//    updateCache();
    m_updateCacheTimer->start();
}

void MimeAppsWorker::handleFileChanged(const QString &filePath)
{
    Q_UNUSED(filePath)
//    updateCache();
    m_updateCacheTimer->start();
    //for 1.4
//    DesktopFile desktopFile(filePath);
//    MimesAppsManager::DesktopObjs.remove(filePath);
//    MimesAppsManager::DesktopObjs.insert(filePath, desktopFile);

//    QMap<QString, QSet<QString>> mimeAppsSet;

//    QStringList mimeTypes = desktopFile.getMimeType();
//    foreach (QString mimeType, mimeTypes) {
//        if (!mimeType.isEmpty()){
//            QSet<QString> apps;
//            if (mimeAppsSet.contains(mimeType)){
//                apps = mimeAppsSet.value(mimeType);
//                apps.insert(filePath);
//            }else{
//                apps.insert(filePath);
//            }
//            mimeAppsSet.insert(mimeType, apps);
//        }
//    }

//    foreach (QString key, mimeAppsSet.keys()) {
//        QSet<QString> apps = mimeAppsSet.value(key);
//        QStringList orderApps;
//        if (apps.count() > 1){
//            QFileInfoList fileInfos;
//            foreach (QString app, apps) {
//                QFileInfo info(app);
//                fileInfos.append(info);
//            }

//            std::sort(fileInfos.begin(), fileInfos.end(), MimesAppsManager::lessByDateTime);

//            foreach (QFileInfo info, fileInfos) {
//                orderApps.append(info.absoluteFilePath());
//            }
//        }else{
//            orderApps.append(apps.toList());
//        }
//        MimesAppsManager::MimeApps.insert(key, orderApps);
//    }

//    foreach (QString desktopFile, MimesAppsManager::DesktopObjs.keys()) {
//        QString iconName = MimesAppsManager::DesktopObjs.value(desktopFile).getIcon();
//        fileIconProvider->getDesktopIcon(iconName, 48);
//    }

}

void MimeAppsWorker::updateCache()
{
    MimesAppsManager::initMimeTypeApps();
}

void MimeAppsWorker::writeData(const QString &path, const QByteArray &content)
{
    qDebug() << path;
    QFile file(path);
    if (file.open(QFile::WriteOnly)){
        file.write(content);
    }
    file.close();
}

QByteArray MimeAppsWorker::readData(const QString &path)
{
    QFile file(path);
    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << path << "isn't exists!";
    }
    QByteArray content = file.readAll();
    file.close();
    return content;
}


MimesAppsManager::MimesAppsManager(QObject *parent): QObject(parent)
{
    m_mimeAppsWorker = new MimeAppsWorker;
    connect(this, &MimesAppsManager::requestUpdateCache, m_mimeAppsWorker, &MimeAppsWorker::updateCache);
    QThread* mimeAppsThread = new QThread;
    m_mimeAppsWorker->moveToThread(mimeAppsThread);
    mimeAppsThread->start();
}

MimesAppsManager::~MimesAppsManager()
{

}

QMimeType MimesAppsManager::getMimeType(const QString &fileName)
{
    DMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(fileName);
    return mimeType;
}

QString MimesAppsManager::getMimeTypeByFileName(const QString &fileName)
{
    DMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(fileName);
    return mimeType.name();
}


QString MimesAppsManager::getDefaultAppByFileName(const QString& fileName){
    DMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(fileName);
    return getDefaultAppByMimeType(mimeType);
}

QString MimesAppsManager::getDefaultAppByMimeType(const QMimeType &mimeType)
{
    return getDefaultAppByMimeType(mimeType.name());
}

QString MimesAppsManager::getDefaultAppByMimeType(const QString &mimeType)
{
    GAppInfo* defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    QString url = "";
    if(defaultApp){
        url = g_app_info_get_id(defaultApp);
    }
    return url;
}

QString MimesAppsManager::getDefaultAppDisplayNameByMimeType(const QMimeType &mimeType)
{
    return getDefaultAppDisplayNameByGio(mimeType.name());
}

QString MimesAppsManager::getDefaultAppDisplayNameByGio(const QString &mimeType)
{
    /*
        *
        * We have the appsForMimeList. Now we need to filter some applications out as per user's choice and get the default value
        * First check mimeapps.list/[Default Associations], then mimeapps.list/[Added Associations]. The entry corresponding to the mimetype in
        * the first case and the first entry in the second case are the user defaults.
        * If the mimetype is not listed, then check mimeinfo.cache
        * Do the same for /usr/local/share/applications and /usr/share/applications
        *
    */

    GAppInfo* defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    QString appDisplayName = "";
    if(defaultApp){
        appDisplayName = g_app_info_get_name(defaultApp);
    }
    g_object_unref(defaultApp);
    return appDisplayName;
}

QString MimesAppsManager::getDefaultAppDesktopFileByMimeType(const QString &mimeType)
{
    GAppInfo* defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    if(!defaultApp)
        return "";

    const char* desktop_id = g_app_info_get_id(defaultApp);
    GDesktopAppInfo* desktopAppInfo = g_desktop_app_info_new(desktop_id);
    if(!desktopAppInfo)
        return "";
    QString desktopFile = g_desktop_app_info_get_filename(desktopAppInfo);

    g_object_unref(defaultApp);
    g_object_unref(desktopAppInfo);

    return desktopFile;
}

bool MimesAppsManager::setDefautlAppForTypeByGio(const QString &mimeType, const QString &appPath)
{
    GAppInfo* app = NULL;
    GList* apps = NULL;
    apps = g_app_info_get_all();

    GList* iterator = apps;

    while (iterator) {
        const char* desktop_id = g_app_info_get_id((GAppInfo*)iterator->data);
        GDesktopAppInfo* desktopAppInfo = g_desktop_app_info_new(desktop_id);

        if (desktopAppInfo) {
            if (appPath == g_desktop_app_info_get_filename(desktopAppInfo)) {
                app = (GAppInfo*)iterator->data;
                g_object_unref(desktopAppInfo);
                break;
            }

            g_object_unref(desktopAppInfo);
        }

        if (appPath.endsWith("/" + QString::fromLocal8Bit(desktop_id))) {
            app = (GAppInfo*)iterator->data;
            break;
        }

        iterator = iterator->next;
    }

    g_list_free(apps);

    if (!app) {
        qWarning() << "no app found name as:" << appPath;
        return false;
    }

    GError* error = NULL;
    //如果是xml类型的文件，需要同时设置application/xml和text/xml字段
    if (mimeType == "application/xml") {
        QString spMimeType = "text/xml";
        g_app_info_set_as_default_for_type(app,
                                           spMimeType.toLocal8Bit().constData(),
                                           &error);
    }
    g_app_info_set_as_default_for_type(app,
                                       mimeType.toLocal8Bit().constData(),
                                       &error);
    if (error) {
        qDebug () << "fail to set default app for type:" << error->message;
        g_free(error);
        return false;
    }

    return true;
}

QStringList MimesAppsManager::getRecommendedApps(const DUrl &url)
{
    if (!url.isValid()) {
        return QStringList();
    }
    QStringList recommendedApps;
    QString mimeType;

    DAbstractFileInfoPointer info = fileService->createFileInfo(nullptr, url);
    mimeType = info->mimeType().name();

    QMimeDatabase db;

    recommendedApps = getRecommendedAppsByQio(db.mimeTypeForName(mimeType));

    //use mime white list to find apps first of all
//    if(recommendedApps.isEmpty() && info) {
//        recommendedApps = getrecommendedAppsFromMimeWhiteList(info->fileUrl());
//    }
    QString custom_app("%1/%2-custom-open-%3.desktop");
    QString default_app = getDefaultAppByMimeType(mimeType);

    custom_app = custom_app.arg(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)).arg(qApp->applicationName()).arg(mimeType.replace("/", "-"));

    if (QFile::exists(custom_app)) {
        MimesAppsManager::removeOneDupFromList(recommendedApps, custom_app);
        recommendedApps.append(custom_app);
    }

    GDesktopAppInfo *desktopAppInfo = g_desktop_app_info_new(default_app.toLocal8Bit().constData());

    if (desktopAppInfo) {
        default_app = QString::fromLocal8Bit(g_desktop_app_info_get_filename(desktopAppInfo));
        g_object_unref(desktopAppInfo);
        MimesAppsManager::removeOneDupFromList(recommendedApps, default_app);
        recommendedApps.prepend(default_app);
    }

    return recommendedApps;
}

QStringList MimesAppsManager::getRecommendedAppsByQio(const QMimeType &mimeType)
{
    QStringList recommendApps;
    QList<QMimeType> mimeTypeList;
    QMimeDatabase mimeDatabase;

    mimeTypeList.append(mimeType);

    while (recommendApps.isEmpty()) {
        for (const QMimeType &type : mimeTypeList) {
            QStringList type_name_list;

            type_name_list.append(type.name());
            type_name_list.append(type.aliases());

            foreach (const QString &name, type_name_list) {
                foreach (const QString &app, mimeAppsManager->MimeApps.value(name)) {
                    bool app_exist = false;

                    for (const QString &other : recommendApps) {
                        const DesktopFile &app_desktop = mimeAppsManager->DesktopObjs.value(app);
                        const DesktopFile &other_desktop = mimeAppsManager->DesktopObjs.value(other);

                        if (app_desktop.getExec() == other_desktop.getExec() && app_desktop.getLocalName() == other_desktop.getLocalName()) {
                            app_exist = true;
                            break;
                        }
                    }

                    // if desktop file was not existed do not recommend!!
                    if (!QFileInfo::exists(app)) {
                        qWarning() << app << "not exist anymore";
                        continue;
                    }

                    if (!app_exist)
                        recommendApps.append(app);
                }
            }
        }

        if (!recommendApps.isEmpty())
            break;

        QList<QMimeType> newMimeTypeList;

        for (const QMimeType &type : mimeTypeList) {
            for (const QString &name : type.parentMimeTypes())
                newMimeTypeList.append(mimeDatabase.mimeTypeForName(name));
        }

        mimeTypeList = newMimeTypeList;

        if (mimeTypeList.isEmpty())
            break;
    }

    return recommendApps;
}

QStringList MimesAppsManager::getRecommendedAppsByGio(const QString &mimeType)
{
    QStringList recommendApps;
    GList* recomendAppInfoList = g_app_info_get_recommended_for_type(mimeType.toLocal8Bit().constData());
    GList* iterator = recomendAppInfoList;

    while(iterator){
        GAppInfo* appInfo = (GAppInfo*)iterator->data;
        if(appInfo){
        const char* desktopId = g_app_info_get_id(appInfo);

        GDesktopAppInfo* dekstopAppInfo = g_desktop_app_info_new(desktopId);
        QString app = g_desktop_app_info_get_filename(dekstopAppInfo);

        recommendApps << app;
        g_object_unref(dekstopAppInfo);
        }
        iterator = iterator->next;
    }
    g_list_free(recomendAppInfoList);
    return recommendApps;
}

QStringList MimesAppsManager::getrecommendedAppsFromMimeWhiteList(const DUrl &url)
{
    const DAbstractFileInfoPointer& info = fileService->createFileInfo(Q_NULLPTR, url);
    QString aliasMimeType = info->mimeTypeName();
    QStringList recommendedApps;
    QString mimeAssociationsFile = QString("%1/%2/%3").arg(DFMStandardPaths::location(DFMStandardPaths::ApplicationSharePath),
                                                           "mimetypeassociations",
                                                           "mimetypeassociations.json");
    QFile file(mimeAssociationsFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug () << "could not open file :" << mimeAssociationsFile << ", error:" << file.errorString();
        return recommendedApps;
    }
    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError* jsonPaserError = NULL;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, jsonPaserError);
    if(jsonPaserError){
       qDebug () << "json file paser error:" << jsonPaserError->errorString();
       return recommendedApps;
    }
    QJsonObject obj = jsonDoc.object();

    if(obj.contains("associations")){
        QJsonArray mimesArr = obj.value("associations").toArray();
        foreach (const QJsonValue& mime, mimesArr) {
            if(mime.toObject().contains(info->mimeTypeName())){
                aliasMimeType = mime.toObject().value(info->mimeTypeName()).toString();
                recommendedApps = getRecommendedAppsByGio(aliasMimeType);
            }
        }
    }

    return recommendedApps;
}

QStringList MimesAppsManager::getApplicationsFolders()
{
    QStringList desktopFolders;
    desktopFolders << QString("/usr/share/applications/")
                   << QString("/usr/local/share/applications/")
                   << QString("/usr/share/gnome/applications/")
                   << QString("/var/lib/flatpak/exports/share/applications")
                   << QDir::homePath() + QString("/.local/share/flatpak/exports/share/applications")
                   << QDir::homePath() + QString( "/.local/share/applications" );
    return desktopFolders;
}

QString MimesAppsManager::getMimeAppsCacheFile()
{
    return QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::CachePath), "MimeApps.json");
}

QString MimesAppsManager::getMimeInfoCacheFilePath()
{
    return "/usr/share/applications/mimeinfo.cache";
}

QString MimesAppsManager::getMimeInfoCacheFileRootPath()
{
    return "/usr/share/applications";
}

QString MimesAppsManager::getDesktopFilesCacheFile()
{
    return QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::CachePath), "DesktopFiles.json");
}

QString MimesAppsManager::getDesktopIconsCacheFile()
{
    return QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::CachePath), "DesktopIcons.json");
}

QStringList MimesAppsManager::getDesktopFiles()
{
      QStringList desktopFiles;

      foreach (QString desktopFolder, getApplicationsFolders()) {
          QDirIterator it(desktopFolder, QStringList("*.desktop"),
                          QDir::Files | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories);
          while (it.hasNext()) {
            it.next();
            desktopFiles.append(it.filePath());
          }
      }
      return desktopFiles;
}

QString MimesAppsManager::getDDEMimeTypeFile()
{
    return QString("%1/%2/%3").arg(getMimeInfoCacheFileRootPath(), "deepin", "dde-mimetype.list");
}

QMap<QString, DesktopFile> MimesAppsManager::getDesktopObjs()
{
    QMap<QString, DesktopFile> desktopObjs;
    foreach (QString f, getApplicationsFolders()) {
        desktopObjs.insert(f, DesktopFile(f));
    }
    return desktopObjs;
}

void MimesAppsManager::initMimeTypeApps()
{
    qDebug() << "getMimeTypeApps in" << QThread::currentThread() << qApp->thread();
    DesktopFiles.clear();
    DesktopObjs.clear();
    DDE_MimeTypes.clear();

    QMap<QString, QSet<QString>> mimeAppsSet;
    loadDDEMimeTypes();
    foreach (QString desktopFolder, getApplicationsFolders()) {
        QDirIterator it(desktopFolder, QStringList("*.desktop"),
                        QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
          it.next();
          QString filePath = it.filePath();
          DesktopFile desktopFile(filePath);
          DesktopFiles.append(filePath);
          DesktopObjs.insert(filePath, desktopFile);
          QStringList mimeTypes = desktopFile.getMimeType();
          QString fileName = QFileInfo(filePath).fileName();
          if (DDE_MimeTypes.contains(fileName)){
              mimeTypes.append(DDE_MimeTypes.value(fileName));
          }

          foreach (QString mimeType, mimeTypes) {
              if (!mimeType.isEmpty()){
                  QSet<QString> apps;
                  if (mimeAppsSet.contains(mimeType)){
                      apps = mimeAppsSet.value(mimeType);
                      apps.insert(filePath);
                  }else{
                      apps.insert(filePath);
                  }
                  mimeAppsSet.insert(mimeType, apps);
              }
          }
        }
    }

    foreach (QString key, mimeAppsSet.keys()) {
        QSet<QString> apps = mimeAppsSet.value(key);
        QStringList orderApps;
        if (apps.count() > 1){
            QFileInfoList fileInfos;
            foreach (QString app, apps) {
                QFileInfo info(app);
                fileInfos.append(info);
            }

            std::sort(fileInfos.begin(), fileInfos.end(), MimesAppsManager::lessByDateTime);

            foreach (QFileInfo info, fileInfos) {
                orderApps.append(info.absoluteFilePath());
            }
        }else{
            orderApps.append(apps.toList());
        }
        MimeApps.insert(key, orderApps);
    }

    //check mime apps from cache
    QFile f(getMimeInfoCacheFilePath());
    if(!f.open(QIODevice::ReadOnly)){
        qDebug () << "failed to read mime info cache file:" << f.errorString();
        return;
    }

    QStringList audioDesktopList;
    QStringList imageDeksopList;
    QStringList textDekstopList;
    QStringList videoDesktopList;

    while (!f.atEnd()) {
        QString data = f.readLine();
        QString _desktops = data.split("=").last();
        QString mimeType = data.split("=").first();
        QStringList desktops = _desktops.split(";");

        foreach (const QString desktop, desktops) {
            if(desktop.isEmpty() || audioDesktopList.contains(desktop))
                continue;

            if(mimeType.startsWith("audio")){
                if(!audioDesktopList.contains(desktop))
                    audioDesktopList << desktop;
            } else if(mimeType.startsWith("image")){
                if(!imageDeksopList.contains(desktop))
                    imageDeksopList << desktop;
            } else if(mimeType.startsWith("text")){
                if(!textDekstopList.contains(desktop))
                    textDekstopList << desktop;
            } else if(mimeType.startsWith("video")){
                if(!videoDesktopList.contains(desktop))
                    videoDesktopList << desktop;
            }
        }
    }
    f.close();

    const QString mimeInfoCacheRootPath = getMimeInfoCacheFileRootPath();
    foreach (QString desktop, audioDesktopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath,desktop);
        if(!QFile::exists(path))
            continue;
        DesktopFile df(path);
        AudioMimeApps.insert(path, df);
    }

    foreach (QString desktop, imageDeksopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath,desktop);
        if(!QFile::exists(path))
            continue;
        DesktopFile df(path);
        ImageMimeApps.insert(path, df);
    }

    foreach (QString desktop, textDekstopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath,desktop);
        if(!QFile::exists(path))
            continue;
        DesktopFile df(path);
        TextMimeApps.insert(path, df);
    }

    foreach (QString desktop, videoDesktopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath,desktop);
        if(!QFile::exists(path))
            continue;
        DesktopFile df(path);
        VideoMimeApps.insert(path, df);
    }

    return;
}

void MimesAppsManager::loadDDEMimeTypes()
{
    QSettings settings(getDDEMimeTypeFile(), QSettings::IniFormat);
    qDebug() << settings.childGroups();

    QFile file(getDDEMimeTypeFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    // Read propeties
    QTextStream in(&file);
    QString desktopKey;
    while (!in.atEnd()) {

      // Read new line
      QString line = in.readLine();

      // Skip empty line or line with invalid format
      if (line.trimmed().isEmpty()) {
        continue;
      }

      // Read group
      // NOTE: symbols '[' and ']' can be found not only in group names, but
      // only group can start with '['

      if (line.trimmed().startsWith("[") && line.trimmed().endsWith("]")) {
            QString tmp = line.trimmed().replace("[", "").replace("]", "");
            desktopKey = tmp;
            continue;
      }

      // If we are in correct group and line contains assignment then read data
      int first_equal = line.indexOf('=');
      if (!desktopKey.isEmpty() && first_equal >= 0) {
            QString value = line.mid(first_equal + 1);
            QStringList mimetypes = value.split(";");
            DDE_MimeTypes.insert(desktopKey, mimetypes);
            desktopKey.clear();
        }
    }
    file.close();
}

bool MimesAppsManager::lessByDateTime(const QFileInfo &f1, const QFileInfo &f2)
{
    return f1.created() < f2.created();
}

bool MimesAppsManager::removeOneDupFromList(QStringList &list, const QString desktopFilePath)
{
    if (list.removeOne(desktopFilePath)) {
        return true;
    }

    const DesktopFile target(desktopFilePath);

    QMutableStringListIterator iter(list);
    while (iter.hasNext()) {
        const DesktopFile source(iter.next());

        if (source.getExec() == target.getExec() && source.getLocalName() == target.getLocalName()) {
            iter.remove();
            return true;
        }
    }

    return false;
}



