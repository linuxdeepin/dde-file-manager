// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mimesappsmanager.h"

#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/base/standardpaths.h>

#include <QDir>
#include <QSettings>
#include <QMimeType>
#include <QDirIterator>
#include <QDateTime>
#include <QThread>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>
#include <QDebug>
#include <QUrl>
#include <QStandardPaths>

#undef signals
extern "C" {
#include <gio/gio.h>
#include <gio/gappinfo.h>
#include <gio-unix-2.0/gio/gdesktopappinfo.h>
}
#define signals public

using namespace dfmbase;

QStringList MimesAppsManager::DesktopFiles = {};
QMap<QString, QStringList> MimesAppsManager::MimeApps = {};
QMap<QString, QStringList> MimesAppsManager::DDE_MimeTypes = {};
QMap<QString, DesktopFile> MimesAppsManager::VideoMimeApps = {};
QMap<QString, DesktopFile> MimesAppsManager::ImageMimeApps = {};
QMap<QString, DesktopFile> MimesAppsManager::TextMimeApps = {};
QMap<QString, DesktopFile> MimesAppsManager::AudioMimeApps = {};
QMap<QString, DesktopFile> MimesAppsManager::DesktopObjs = {};

MimeAppsWorker::MimeAppsWorker(QObject *parent)
    : QObject(parent)
{
    updateCacheTimer = new QTimer(this);
    updateCacheTimer->setInterval(2000);
    updateCacheTimer->setSingleShot(true);
    startWatch();
    initConnect();
}

MimeAppsWorker::~MimeAppsWorker()
{
}

void MimeAppsWorker::initConnect()
{
    connect(updateCacheTimer, &QTimer::timeout, this, &MimeAppsWorker::updateCache);
}

void MimeAppsWorker::startWatch()
{
    const QStringList &paths { MimesAppsManager::getApplicationsFolders() };
    std::for_each(paths.begin(), paths.end(), [this](const QString &path) {
        AbstractFileWatcherPointer watcher { WatcherFactory::create<AbstractFileWatcher>(QUrl::fromLocalFile(path)) };
        watcherGroup.append(watcher);
        if (watcher) {
            connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this, [this]() {
                updateCacheTimer->start();
            });
            watcher->startWatcher();
        }
    });
}

void MimeAppsWorker::updateCache()
{
    MimesAppsManager::initMimeTypeApps();
}

void MimeAppsWorker::writeData(const QString &path, const QByteArray &content)
{
    qCDebug(logDFMBase) << path;
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        file.write(content);
    }
    file.close();
}

QByteArray MimeAppsWorker::readData(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(logDFMBase) << path << "isn't exists!";
        return {};
    }
    QByteArray content = file.readAll();
    file.close();
    return content;
}

MimesAppsManager *MimesAppsManager::instance()
{
    static MimesAppsManager manager;
    return &manager;
}

MimesAppsManager::MimesAppsManager(QObject *parent)
    : QObject(parent)
{
    mimeAppsWorker = new MimeAppsWorker;
    connect(this, &MimesAppsManager::requestUpdateCache, mimeAppsWorker, &MimeAppsWorker::updateCache);
    mimeAppsWorker->moveToThread(&mimeAppsThread);
    connect(&mimeAppsThread, &QThread::finished, mimeAppsWorker, &QObject::deleteLater);
    mimeAppsThread.start();
}

MimesAppsManager::~MimesAppsManager()
{
    mimeAppsThread.quit();
    mimeAppsThread.wait();
}

QMimeType MimesAppsManager::getMimeType(const QString &fileName)
{
    const QMimeType &mimeType = DMimeDatabase().mimeTypeForFile(fileName);
    return mimeType;
}

QString MimesAppsManager::getMimeTypeByFileName(const QString &fileName)
{
    const QMimeType &mimeType = DMimeDatabase().mimeTypeForFile(fileName);
    return mimeType.name();
}

QString MimesAppsManager::getDefaultAppByFileName(const QString &fileName)
{
    const QMimeType &mimeType = DMimeDatabase().mimeTypeForFile(fileName);
    return getDefaultAppByMimeType(mimeType);
}

QString MimesAppsManager::getDefaultAppByMimeType(const QMimeType &mimeType)
{
    return getDefaultAppByMimeType(mimeType.name());
}

QString MimesAppsManager::getDefaultAppByMimeType(const QString &mimeType)
{
    g_autoptr(GAppInfo) defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    QString url = "";
    if (defaultApp) {
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

    g_autoptr(GAppInfo) defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    QString appDisplayName = "";
    if (defaultApp) {
        appDisplayName = g_app_info_get_name(defaultApp);
    }
    return appDisplayName;
}

QString MimesAppsManager::getDefaultAppDesktopFileByMimeType(const QString &mimeType)
{
    g_autoptr(GAppInfo) defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    if (!defaultApp)
        return "";

    const char *desktopId = g_app_info_get_id(defaultApp);
    g_autoptr(GDesktopAppInfo) desktopAppInfo = g_desktop_app_info_new(desktopId);
    if (!desktopAppInfo)
        return "";
    QString desktopFile = g_desktop_app_info_get_filename(desktopAppInfo);

    return desktopFile;
}

bool MimesAppsManager::setDefautlAppForTypeByGio(const QString &mimeType, const QString &appPath)
{
    GAppInfo *app = nullptr;
    GList *apps = nullptr;
    apps = g_app_info_get_all();

    GList *iterator = apps;

    while (iterator) {
        const char *desktopId = g_app_info_get_id((GAppInfo *)iterator->data);
        GDesktopAppInfo *desktopAppInfo = g_desktop_app_info_new(desktopId);

        if (desktopAppInfo) {
            if (appPath == g_desktop_app_info_get_filename(desktopAppInfo)) {
                app = (GAppInfo *)iterator->data;
                g_object_unref(desktopAppInfo);
                break;
            }

            g_object_unref(desktopAppInfo);
        }

        if (appPath.endsWith("/" + QString::fromLocal8Bit(desktopId))) {
            app = (GAppInfo *)iterator->data;
            break;
        }

        iterator = iterator->next;
    }

    g_list_free(apps);

    if (!app) {
        qCWarning(logDFMBase) << "no app found name as:" << appPath;
        return false;
    }

    g_autoptr(GError) error = nullptr;
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
        qCWarning(logDFMBase) << "fail to set default app for type:" << error->message;
        return false;
    }

    return true;
}

QStringList MimesAppsManager::getRecommendedApps(const QUrl &url)
{
    if (!url.isValid()) {
        return QStringList();
    }
    QStringList recommendedApps;
    QString mimeType;

    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    mimeType = info->fileMimeType().name();

    DFMBASE_NAMESPACE::DMimeDatabase db;

    recommendedApps = getRecommendedAppsByQio(db.mimeTypeForName(mimeType));

    //use mime white list to find apps first of all
    //    if(recommendedApps.isEmpty() && info) {
    //        recommendedApps = getrecommendedAppsFromMimeWhiteList(info->fileUrl());
    //    }
    QString customApp("%1/%2-custom-open-%3.desktop");
    QString defaultApp = getDefaultAppByMimeType(mimeType);

    customApp = customApp.arg(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)).arg(qApp->applicationName()).arg(mimeType.replace("/", "-"));

    if (QFile::exists(customApp)) {
        MimesAppsManager::removeOneDupFromList(recommendedApps, customApp);
        recommendedApps.append(customApp);
    }

    GDesktopAppInfo *desktopAppInfo = g_desktop_app_info_new(defaultApp.toLocal8Bit().constData());

    if (desktopAppInfo) {
        defaultApp = QString::fromLocal8Bit(g_desktop_app_info_get_filename(desktopAppInfo));
        g_object_unref(desktopAppInfo);
        MimesAppsManager::removeOneDupFromList(recommendedApps, defaultApp);
        recommendedApps.prepend(defaultApp);
    }

    return recommendedApps;
}

QStringList MimesAppsManager::getRecommendedAppsByQio(const QMimeType &mimeType)
{
    QStringList recommendApps;
    QList<QMimeType> mimeTypeList;
    DFMBASE_NAMESPACE::DMimeDatabase mimeDatabase;

    mimeTypeList.append(mimeType);

    while (recommendApps.isEmpty()) {
        for (const QMimeType &type : mimeTypeList) {
            QStringList typeNameList;

            typeNameList.append(type.name());
            typeNameList.append(type.aliases());

            foreach (const QString &name, typeNameList) {
                foreach (const QString &app, MimesAppsManager::MimeApps.value(name)) {
                    bool appExist = false;

                    for (const QString &other : recommendApps) {
                        const DesktopFile &appDesktop = MimesAppsManager::DesktopObjs.value(app);
                        const DesktopFile &otherDesktop = MimesAppsManager::DesktopObjs.value(other);

                        if (appDesktop.desktopExec() == otherDesktop.desktopExec() && appDesktop.desktopLocalName() == otherDesktop.desktopLocalName()) {
                            appExist = true;
                            break;
                        }
                    }

                    // if desktop file was not existed do not recommend!!
                    if (!QFileInfo::exists(app)) {
                        qCWarning(logDFMBase) << app << "not exist anymore";
                        continue;
                    }

                    if (!appExist)
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
    GList *recomendAppInfoList = g_app_info_get_recommended_for_type(mimeType.toLocal8Bit().constData());
    GList *iterator = recomendAppInfoList;

    while (iterator) {
        GAppInfo *appInfo = (GAppInfo *)iterator->data;
        if (appInfo) {
            const char *desktopId = g_app_info_get_id(appInfo);

            GDesktopAppInfo *dekstopAppInfo = g_desktop_app_info_new(desktopId);
            QString app = g_desktop_app_info_get_filename(dekstopAppInfo);

            recommendApps << app;
            g_object_unref(dekstopAppInfo);
        }
        iterator = iterator->next;
    }
    g_list_free(recomendAppInfoList);
    return recommendApps;
}

QStringList MimesAppsManager::getrecommendedAppsFromMimeWhiteList(const QUrl &url)
{
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    QString aliasMimeType = info->fileMimeType().name();
    QStringList recommendedApps;
    QString mimeAssociationsFile = QString("%1/%2/%3").arg(StandardPaths::location(StandardPaths::kApplicationSharePath), "mimetypeassociations", "mimetypeassociations.json");
    QFile file(mimeAssociationsFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(logDFMBase) << "could not open file :" << mimeAssociationsFile << ", error:" << file.errorString();
        return recommendedApps;
    }
    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError *jsonPaserError = nullptr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, jsonPaserError);
    if (jsonPaserError) {
        qCWarning(logDFMBase) << "json file paser error:" << jsonPaserError->errorString();
        return recommendedApps;
    }
    QJsonObject obj = jsonDoc.object();

    if (obj.contains("associations")) {
        QJsonArray mimesArr = obj.value("associations").toArray();
        foreach (const QJsonValue &mime, mimesArr) {
            if (mime.toObject().contains(info->fileMimeType().name())) {
                aliasMimeType = mime.toObject().value(info->fileMimeType().name()).toString();
                recommendedApps = getRecommendedAppsByGio(aliasMimeType);
            }
        }
    }

    return recommendedApps;
}

QStringList MimesAppsManager::getApplicationsFolders()
{
    QStringList desktopFolders;
    desktopFolders << QString("/usr/share/applications")
                   << QString("/usr/local/share/applications")
                   << QString("/usr/share/gnome/applications");
    // env for XDG_DATA_DIRS
    for (const QString &path : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation)) {
        if (desktopFolders.contains(path))
            continue;
        desktopFolders.append(path);
    }

    return desktopFolders;
}

QString MimesAppsManager::getMimeAppsCacheFile()
{
    return QString("%1/%2").arg(StandardPaths::location(StandardPaths::kCachePath), "MimeApps.json");
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
    return QString("%1/%2").arg(StandardPaths::location(StandardPaths::kCachePath), "DesktopFiles.json");
}

QString MimesAppsManager::getDesktopIconsCacheFile()
{
    return QString("%1/%2").arg(StandardPaths::location(StandardPaths::kCachePath), "DesktopIcons.json");
}

QString MimesAppsManager::getDDEMimeTypeFile()
{
    return QString("%1/%2/%3").arg(getMimeInfoCacheFileRootPath(), "deepin", "dde-mimetype.list");
}

QMap<QString, DesktopFile> MimesAppsManager::getDesktopObjs()
{
    QMap<QString, DesktopFile> desktopObjs;
    for (const QString &desktopFolder : getApplicationsFolders()) {
        desktopObjs.insert(desktopFolder, DesktopFile(desktopFolder));
    }
    return desktopObjs;
}

void MimesAppsManager::initMimeTypeApps()
{
    qCDebug(logDFMBase) << "getMimeTypeApps in" << QThread::currentThread() << qApp->thread();
    DesktopFiles.clear();
    DesktopObjs.clear();
    DDE_MimeTypes.clear();

    QMap<QString, QSet<QString>> mimeAppsSet;
    loadDDEMimeTypes();
    for (const QString &desktopFolder : getApplicationsFolders()) {
        QDirIterator it(desktopFolder, QStringList("*.desktop"), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            const QString &filePath = it.filePath();
            DesktopFile desktopFile(filePath);
            if (desktopFile.isNoShow())
                continue;

            DesktopFiles.append(filePath);
            DesktopObjs.insert(filePath, desktopFile);
            QStringList mimeTypes = desktopFile.desktopMimeType();
            QString fileName = QFileInfo(filePath).fileName();
            if (DDE_MimeTypes.contains(fileName)) {
                mimeTypes.append(DDE_MimeTypes.value(fileName));
            }

            for (const QString &mimeType : mimeTypes) {
                if (!mimeType.isEmpty()) {
                    QSet<QString> apps;
                    if (mimeAppsSet.contains(mimeType)) {
                        apps = mimeAppsSet.value(mimeType);
                        apps.insert(filePath);
                    } else {
                        apps.insert(filePath);
                    }
                    mimeAppsSet.insert(mimeType, apps);
                }
            }
        }
    }

    for (const QString &key : mimeAppsSet.keys()) {
        QSet<QString> apps = mimeAppsSet.value(key);
        QStringList orderApps;
        if (apps.count() > 1) {
            QFileInfoList fileInfos;
            for (const QString &app : apps) {
                QFileInfo info(app);
                fileInfos.append(info);
            }

            std::sort(fileInfos.begin(), fileInfos.end(), MimesAppsManager::lessByDateTime);

            for (const QFileInfo &info : fileInfos) {
                orderApps.append(info.absoluteFilePath());
            }
        } else {
            orderApps.append(apps.values());
        }
        MimeApps.insert(key, orderApps);
    }

    //check mime apps from cache
    QFile f(getMimeInfoCacheFilePath());
    if (!f.open(QIODevice::ReadOnly)) {
        qCWarning(logDFMBase) << "failed to read mime info cache file:" << f.errorString();
        return;
    }

    QStringList audioDesktopList;
    QStringList imageDeksopList;
    QStringList textDekstopList;
    QStringList videoDesktopList;

    while (!f.atEnd()) {
        QString data = f.readLine();
        QString desktop = data.split("=").last();
        QString mimeType = data.split("=").first();
        QStringList desktops = desktop.split(";");

        for (const QString &desktop : desktops) {
            if (desktop.isEmpty() || audioDesktopList.contains(desktop))
                continue;

            if (mimeType.startsWith("audio")) {
                if (!audioDesktopList.contains(desktop))
                    audioDesktopList << desktop;
            } else if (mimeType.startsWith("image")) {
                if (!imageDeksopList.contains(desktop))
                    imageDeksopList << desktop;
            } else if (mimeType.startsWith("text")) {
                if (!textDekstopList.contains(desktop))
                    textDekstopList << desktop;
            } else if (mimeType.startsWith("video")) {
                if (!videoDesktopList.contains(desktop))
                    videoDesktopList << desktop;
            }
        }
    }
    f.close();

    const QString &mimeInfoCacheRootPath = getMimeInfoCacheFileRootPath();
    for (const QString &desktop : audioDesktopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path))
            continue;
        DesktopFile df(path);
        AudioMimeApps.insert(path, df);
    }

    for (const QString &desktop : imageDeksopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path))
            continue;
        DesktopFile df(path);
        ImageMimeApps.insert(path, df);
    }

    for (const QString &desktop : textDekstopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path))
            continue;
        DesktopFile df(path);
        TextMimeApps.insert(path, df);
    }

    for (const QString &desktop : videoDesktopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path))
            continue;
        DesktopFile df(path);
        VideoMimeApps.insert(path, df);
    }

    return;
}

void MimesAppsManager::loadDDEMimeTypes()
{
    QSettings settings(getDDEMimeTypeFile(), QSettings::IniFormat);
    qCDebug(logDFMBase) << settings.childGroups();

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
    return f1.birthTime() < f2.birthTime();
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

        if (source.desktopExec() == target.desktopExec() && source.desktopLocalName() == target.desktopLocalName()) {
            iter.remove();
            return true;
        }
    }

    return false;
}
