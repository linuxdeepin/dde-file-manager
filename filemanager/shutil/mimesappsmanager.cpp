#include "mimesappsmanager.h"
#include <QDir>
#include <QSettings>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDirIterator>
#include <QDateTime>
#include <QDebug>
#include "desktopfile.h"


MimesAppsManager::MimesAppsManager()
{

}

MimesAppsManager::~MimesAppsManager()
{

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
    return app;
}

QStringList MimesAppsManager::getDesktopFiles()
{
      QStringList desktopFolders;
      desktopFolders << QString("/usr/share/applications")
                     << QDir::homePath() + QString( "/.local/share/applications" );
      QStringList desktopFiles;

      foreach (QString desktopFolder, desktopFolders) {
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

QMap<QString, QStringList> MimesAppsManager::getMimeTypeApps()
{
    QMap<QString, QSet<QString>> mimeAppsSet;
    foreach (QString f, MimesAppsManager::getDesktopFiles()) {
        DesktopFile desktopFile(f);
        QStringList mimeTypes = desktopFile.getMimeType();
        foreach (QString mimeType, mimeTypes) {
            if (!mimeType.isEmpty()){
                QSet<QString> apps;
                if (mimeAppsSet.contains(mimeType)){
                    apps = mimeAppsSet.value(mimeType);
                    apps.insert(f);
                }else{
                    apps.insert(f);
                }
                mimeAppsSet.insert(mimeType, apps);
            }
        }
    }
    QMap<QString, QStringList> mimeApps;
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
        mimeApps.insert(key, orderApps);
    }
    return mimeApps;
}

bool MimesAppsManager::lessByDateTime(const QFileInfo &f1, const QFileInfo &f2)
{
    return f1.created() < f2.created();
}


void MimesAppsManager::test()
{
    QMap<QString, QStringList> mimeApps = getMimeTypeApps();
    qDebug() << mimeApps.count();
    qDebug() << DesktopFile("/usr/share/applications/vlc.desktop").getMimeType();
    qDebug() << mimeApps.value("audio/x-mp3");
}
