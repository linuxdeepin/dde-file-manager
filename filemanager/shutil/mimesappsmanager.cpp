#include "mimesappsmanager.h"
#include <QDir>
#include <QSettings>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDirIterator>
#include <QDateTime>
#include <QThread>
#include <QStandardPaths>
#include <QDebug>
#include "desktopfile.h"
#include "standardpath.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>
#include "../app/global.h"
#include "iconprovider.h"


QStringList MimesAppsManager::DesktopFiles = {};
QMap<QString, QStringList> MimesAppsManager::MimeApps = {};
QMap<QString, DesktopFile> MimesAppsManager::DesktopObjs = {};

MimeAppsWorker::MimeAppsWorker(QObject *parent): QObject(parent)
{
    m_fileSystemWatcher = new QFileSystemWatcher;
    startWatch();
    initConnect();
}

MimeAppsWorker::~MimeAppsWorker()
{

}

void MimeAppsWorker::initConnect()
{
    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &MimeAppsWorker::handleDirectoryChanged);
    connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &MimeAppsWorker::handleDirectoryChanged);
}

void MimeAppsWorker::startWatch()
{
    m_fileSystemWatcher->addPaths(MimesAppsManager::getDesktopFiles());
}

void MimeAppsWorker::handleDirectoryChanged()
{
    updateCache();
}

void MimeAppsWorker::handleFileChanged()
{
    updateCache();
}

void MimeAppsWorker::updateCache()
{
    if (QFile(MimesAppsManager::getMimeAppsCacheFile()).exists() &&
            QFile(MimesAppsManager::getDesktopFilesCacheFile()).exists() &&
            QFile(MimesAppsManager::getDesktopIconsCacheFile()).exists()){
        loadCache();
    }else{
        MimesAppsManager::getMimeTypeApps();
        saveCache();
    }
}

void MimeAppsWorker::saveCache()
{
    QVariantMap ma;
    foreach (QString key, MimesAppsManager::MimeApps.keys()) {
        ma.insert(key, MimesAppsManager::MimeApps.value(key));
    }
    QJsonDocument doc(QJsonObject::fromVariantMap(ma));
    writeData(MimesAppsManager::getMimeAppsCacheFile(), doc.toJson());

    QJsonDocument desktopFileDoc(QJsonArray::fromStringList(MimesAppsManager::DesktopFiles));
    writeData(MimesAppsManager::getDesktopFilesCacheFile(), desktopFileDoc.toJson());


    QVariantMap icons;
    foreach (QString key, fileIconProvider->getDesktopIconPaths().keys()) {
        icons.insert(key, fileIconProvider->getDesktopIconPaths().value(key));
    }

    QJsonDocument desktopFileIconDoc(QJsonObject::fromVariantMap(icons));
    writeData(MimesAppsManager::getDesktopIconsCacheFile(), desktopFileIconDoc.toJson());

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

void MimeAppsWorker::loadCache()
{
    loadMimeAppsCache();
    loadDesktopFilesCache();
    loadDesktopIconsCache();
}

void MimeAppsWorker::loadMimeAppsCache()
{
    if (QFile(MimesAppsManager::getMimeAppsCacheFile()).exists()){
        qDebug() << "load mime apps cache from:" << MimesAppsManager::getMimeAppsCacheFile();
        QByteArray mimeAppsContent = readData(MimesAppsManager::getMimeAppsCacheFile());
        QJsonParseError error;
        QJsonDocument mimeAppsdoc=QJsonDocument::fromJson(mimeAppsContent,&error);
        if (error.error == QJsonParseError::NoError){
            QJsonObject obj = mimeAppsdoc.object();
            foreach (QString key, obj.keys()) {
                QVariantList appVariants = obj.value(key).toArray().toVariantList();
                QStringList apps;
                foreach(QVariant appVariant, appVariants){
                    apps.append(appVariant.toString());
                }
                MimesAppsManager::MimeApps.insert(key, apps);
            }
        }else{
            qDebug() << "load cache file: " << MimesAppsManager::getMimeAppsCacheFile() << error.errorString();
        }
    }else{
        qDebug() << MimesAppsManager::getMimeAppsCacheFile() << "isn't exists";
    }
}

void MimeAppsWorker::loadDesktopFilesCache()
{
    if (QFile(MimesAppsManager::getDesktopFilesCacheFile()).exists()){
        qDebug() << "load desktop files cache from:" << MimesAppsManager::getDesktopFilesCacheFile();
        QByteArray desktopFilesContent = readData(MimesAppsManager::getDesktopFilesCacheFile());
        QJsonParseError error;
        QJsonDocument desktopFiledoc=QJsonDocument::fromJson(desktopFilesContent,&error);
        if (error.error == QJsonParseError::NoError){
            QJsonArray array = desktopFiledoc.array();
            QVariantList desktopFileVariants =  array.toVariantList();
            QStringList desktopFiles;
            foreach(QVariant desktopFileVariant, desktopFileVariants){
                desktopFiles.append(desktopFileVariant.toString());
            }

            foreach(QString f, desktopFiles){
                MimesAppsManager::DesktopObjs.insert(f, DesktopFile(f));
            }
            MimesAppsManager::DesktopFiles = desktopFiles;
        }else{
            qDebug() << "load cache file: " << MimesAppsManager::getDesktopFilesCacheFile() << error.errorString();
        }
    }else{
        qDebug() << MimesAppsManager::getDesktopFilesCacheFile() << "isn't exists";
    }

}

void MimeAppsWorker::loadDesktopIconsCache()
{
    if (QFile(MimesAppsManager::getDesktopIconsCacheFile()).exists()){
        qDebug() << "load desktop icons cache from:" << MimesAppsManager::getDesktopFilesCacheFile();
        QByteArray content = readData(MimesAppsManager::getDesktopIconsCacheFile());
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(content,&error);
        if (error.error == QJsonParseError::NoError){
            QMap<QString, QString> iconPaths;
            QJsonObject obj = doc.object();
            foreach (QString key, obj.keys()) {
                QString path = obj.value(key).toString();
                iconPaths.insert(key, path);
            }
            fileIconProvider->setDesktopIconPaths(iconPaths);
        }else{
            qDebug() << "load cache file: " << MimesAppsManager::getDesktopIconsCacheFile() << error.errorString();
        }
    }else{
        qDebug() << MimesAppsManager::getDesktopIconsCacheFile() << "isn't exists";
    }

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
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(fileName);
    return mimeType;
}

QString MimesAppsManager::getMimeTypeByFileName(const QString &fileName)
{
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(fileName);
    return mimeType.name();
}


QString MimesAppsManager::getDefaultAppByFileName(const QString& fileName){
    QMimeDatabase db;
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

    QStringList files;
    files << QDir::homePath() + QString( "/.config/mimeapps.list" );
    files << QDir::homePath() + QString( "/.local/share/applications/mimeapps.list" );
    files << QDir::homePath() + QString( "/.local/share/applications/defaults.list" );
    files << QDir::homePath() + QString( "/.local/share/applications/mimeinfo.cache" );

    files << QString( "/usr/local/share/applications/mimeapps.list" );
    files << QString( "/usr/local/share/applications/defaults.list" );
    files << QString( "/usr/local/share/applications/mimeinfo.cache" );

    files << QString( "/usr/share/applications/mimeapps.list" );
    files << QString( "/usr/share/applications/defaults.list" );
    files << QString( "/usr/share/applications/mimeinfo.cache" );

    QString app = "";
    foreach( QString file, files) {
        if (QFile(file).exists()){
            QSettings defaults(file, QSettings::IniFormat);
            QString app;

            app = defaults.value(QString("Added Associations/%1" ).arg(mimeType)).toString();
            if (app.length() > 0){
                return app;
            }

            app = defaults.value(QString("Default Applications/%1" ).arg(mimeType)).toString();
            if (app.length() > 0){
                return app;
            }

            app = defaults.value(QString("MIME Cache/%1" ).arg(mimeType)).toString();
            if (app.length() > 0){
                return app;
            }
        }
    }
    return app;
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
    return QString("%1/%2").arg(StandardPath::getCachePath(), "MimeApps.json");
}

QString MimesAppsManager::getDesktopFilesCacheFile()
{
    return QString("%1/%2").arg(StandardPath::getCachePath(), "DesktopFiles.json");
}

QString MimesAppsManager::getDesktopIconsCacheFile()
{
    return QString("%1/%2").arg(StandardPath::getCachePath(), "DesktopIcons.json");
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

    foreach (QString desktopFile, DesktopObjs.keys()) {
        QString iconName = DesktopObjs.value(desktopFile).getIcon();
        fileIconProvider->getDesktopIcon(iconName, 48);
    }

    return MimeApps;
}

bool MimesAppsManager::lessByDateTime(const QFileInfo &f1, const QFileInfo &f2)
{
    return f1.created() < f2.created();
}



