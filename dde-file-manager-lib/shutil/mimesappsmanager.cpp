#include "mimesappsmanager.h"
#include "dmimedatabase.h"

#include "app/define.h"

#include "widgets/singleton.h"

#include <QDir>
#include <QSettings>
#include <QMimeType>
#include <QDirIterator>
#include <QDateTime>
#include <QThread>
#include <QStandardPaths>
#include <QDebug>
#include "desktopfile.h"
#include "interfaces/dfmstandardpaths.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>

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
    MimesAppsManager::getMimeTypeApps();
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
    /*
        *
        * We have the appsForMimeList. Now we need to filter some applications out as per user's choice and get the default value
        * First check mimeapps.list/[Default Associations], then mimeapps.list/[Added Associations]. The entry corresponding to the mimetype in
        * the first case and the first entry in the second case are the user defaults.
        * If the mimetype is not listed, then check mimeinfo.cache
        * Do the same for /usr/local/share/applications and /usr/share/applications
        *
    */

    GAppInfo* defaultApp = g_app_info_get_default_for_type(mimeType.toStdString().c_str(), FALSE);
    QString appDisplayName = "";
    if(defaultApp){
        appDisplayName = g_app_info_get_name(defaultApp);
    }
    return appDisplayName;
}

void MimesAppsManager::setDefautlAppForType(const QString &mimeType,
                                            const QString &targetAppName)
{
    GAppInfo* app;
    GList* apps = g_app_info_get_all();
    GList* iterator = apps;

    while(iterator){
        QString appName = g_app_info_get_name((GAppInfo*)iterator->data);
        if(appName == targetAppName){
            app = (GAppInfo*)iterator->data;
            break;
        }
        iterator = iterator->next;
    }

    if(!app)
        return;

    g_app_info_set_as_default_for_type(app,
                                       mimeType.toStdString().c_str(),
                                       NULL);
}

QList<AppContext> MimesAppsManager::getRecommendedAppsForType(const QString &mimeType)
{
    QList<AppContext> appContexts;
    GList* apps = g_app_info_get_recommended_for_type(mimeType.toStdString().c_str());
    GList* iterator = apps;
    while (iterator) {
        GAppInfo* appInfo = (GAppInfo*)iterator->data;
        AppContext appContext;
        const QString& appName = g_app_info_get_name(appInfo);
        const QString& commandLine = g_app_info_get_commandline(appInfo);
        const char* desktopId = g_app_info_get_id(appInfo);
        GDesktopAppInfo* dekstopAppInfo = g_desktop_app_info_new(desktopId);
        QString iconName = g_desktop_app_info_get_string(dekstopAppInfo, "Icon");
        QIcon icon(QIcon::fromTheme(iconName));

        appContext.appName = appName;
        appContext.commandLine = commandLine;
        appContext.appIcon = icon;
        appContexts << appContext;
        iterator = iterator->next;
    }
    return appContexts;
}

QStringList MimesAppsManager::getApplicationsFolders()
{
    QStringList desktopFolders;
    desktopFolders << QString("/usr/share/applications/")
                   << QString("/usr/local/share/applications/)")
                   << QString("/usr/share/gnome/applications/")
                   << QDir::homePath() + QString( "/.local/share/applications" );
    return desktopFolders;
}

QString MimesAppsManager::getMimeAppsCacheFile()
{
    return QString("%1/%2").arg(DFMStandardPaths::standardLocation(DFMStandardPaths::CachePath), "MimeApps.json");
}

QString MimesAppsManager::getDesktopFilesCacheFile()
{
    return QString("%1/%2").arg(DFMStandardPaths::standardLocation(DFMStandardPaths::CachePath), "DesktopFiles.json");
}

QString MimesAppsManager::getDesktopIconsCacheFile()
{
    return QString("%1/%2").arg(DFMStandardPaths::standardLocation(DFMStandardPaths::CachePath), "DesktopIcons.json");
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

QMap<QString, DesktopFile> MimesAppsManager::getDesktopObjs()
{
    QMap<QString, DesktopFile> desktopObjs;
    foreach (QString f, getApplicationsFolders()) {
        desktopObjs.insert(f, DesktopFile(f));
    }
    return desktopObjs;
}

QMap<QString, QStringList> MimesAppsManager::getMimeTypeApps()
{
    qDebug() << "getMimeTypeApps in" << QThread::currentThread() << qApp->thread();
    DesktopFiles.clear();
    DesktopObjs.clear();

    QMap<QString, QSet<QString>> mimeAppsSet;

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

//    foreach (QString desktopFile, DesktopObjs.keys()) {
//        QString iconName = DesktopObjs.value(desktopFile).getIcon();
//    }

    return MimeApps;
}

bool MimesAppsManager::lessByDateTime(const QFileInfo &f1, const QFileInfo &f2)
{
    return f1.created() < f2.created();
}



