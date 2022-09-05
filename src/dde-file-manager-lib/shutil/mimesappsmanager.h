// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include <QIcon>

class DUrl;

class MimeAppsWorker: public QObject
{
    Q_OBJECT

public:
    explicit MimeAppsWorker(QObject *parent = nullptr);
    ~MimeAppsWorker();

    void initConnect();

public slots:
    void startWatch();
    void handleDirectoryChanged(const QString &filePath);
    void handleFileChanged(const QString &filePath);
    void updateCache();
    void writeData(const QString &path, const QByteArray &content);
    QByteArray readData(const QString &path);


private:
    QFileSystemWatcher *m_fileSystemWatcher = nullptr;
    QTimer *m_updateCacheTimer;
};


class MimesAppsManager: public QObject
{
    Q_OBJECT

public:
    explicit MimesAppsManager(QObject *parent = nullptr);
    ~MimesAppsManager();

    static QStringList DesktopFiles;
    static QMap<QString, QStringList> MimeApps;
    static QMap<QString, QStringList> DDE_MimeTypes;
    //specially cache for video, image, text and audio
    static QMap<QString, DesktopFile> VideoMimeApps;
    static QMap<QString, DesktopFile> ImageMimeApps;
    static QMap<QString, DesktopFile> TextMimeApps;
    static QMap<QString, DesktopFile> AudioMimeApps;
    static QMap<QString, DesktopFile> DesktopObjs;

    static QMimeType getMimeType(const QString &fileName);
    static QString getMimeTypeByFileName(const QString &fileName);
    static QString getDefaultAppByFileName(const QString &fileName);
    static QString getDefaultAppByMimeType(const QMimeType &mimeType);
    static QString getDefaultAppByMimeType(const QString &mimeType);
    static QString getDefaultAppDisplayNameByMimeType(const QMimeType &mimeType);
    static QString getDefaultAppDisplayNameByGio(const QString &mimeType);
    static QString getDefaultAppDesktopFileByMimeType(const QString &mimeType);

    static bool setDefautlAppForTypeByGio(const QString &mimeType,
                                          const QString &appPath);

    static QStringList getRecommendedApps(const DUrl &url);
    static QStringList getRecommendedAppsByQio(const QMimeType &mimeType);
    static QStringList getRecommendedAppsByGio(const QString &mimeType);
    static QStringList getrecommendedAppsFromMimeWhiteList(const DUrl &url);


    static QStringList getApplicationsFolders();
    static QString getMimeAppsCacheFile();
    static QString getMimeInfoCacheFilePath();
    static QString getMimeInfoCacheFileRootPath();
    static QString getDesktopFilesCacheFile();
    static QString getDesktopIconsCacheFile();
    static QStringList getDesktopFiles();
    static QString getDDEMimeTypeFile();
    static QMap<QString, DesktopFile> getDesktopObjs();
    static void initMimeTypeApps();
    static void loadDDEMimeTypes();
    static bool lessByDateTime(const QFileInfo &f1,  const QFileInfo &f2);
    static bool removeOneDupFromList(QStringList &list, const QString desktopFilePath);

signals:
    void requestUpdateCache();

private:
    MimeAppsWorker *m_mimeAppsWorker = nullptr;

};

#endif // MIMESAPPSMANAGER_H
