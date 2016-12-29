#ifndef MIMESAPPSMANAGER_H
#define MIMESAPPSMANAGER_H


#include <QObject>
#include <QSet>
#include <QMimeType>
#include <QMap>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTimer>
#include "desktopfile.h"


class MimeAppsWorker: public QObject
{
   Q_OBJECT

public:
    MimeAppsWorker(QObject *parent = 0);
    ~MimeAppsWorker();

    void initConnect();
public slots:
    void startWatch();
    void handleDirectoryChanged(const QString& filePath);
    void handleFileChanged(const QString& filePath);
    void updateCache();
    void writeData(const QString& path, const QByteArray& content);
    QByteArray readData(const QString& path);


private:
    QFileSystemWatcher* m_fileSystemWatcher = NULL;
    QTimer* m_updateCacheTimer;
};


class MimesAppsManager: public QObject
{
    Q_OBJECT

public:
    MimesAppsManager(QObject *parent = 0);
    ~MimesAppsManager();

    static QStringList DesktopFiles;
    static QMap<QString, QStringList> MimeApps;
    static QMap<QString, DesktopFile> DesktopObjs;

    static QMimeType getMimeType(const QString& fileName);
    static QString getMimeTypeByFileName(const QString& fileName);
    static QString getDefaultAppByFileName(const QString& fileName);
    static QString getDefaultAppByMimeType(const QMimeType& mimeType);
    static QString getDefaultAppByMimeType(const QString& mimeType);

    static QStringList getApplicationsFolders();
    static QString getMimeAppsCacheFile();
    static QString getDesktopFilesCacheFile();
    static QString getDesktopIconsCacheFile();
    static QStringList getDesktopFiles();
    static QMap<QString, DesktopFile> getDesktopObjs();
    static QMap<QString, QStringList> getMimeTypeApps();
    static bool lessByDateTime(const QFileInfo& f1,  const QFileInfo& f2);

signals:
    void requestUpdateCache();

private:
    MimeAppsWorker* m_mimeAppsWorker=NULL;

};

#endif // MIMESAPPSMANAGER_H
