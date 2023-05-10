// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MIMESAPPSMANAGER_H
#define MIMESAPPSMANAGER_H

#include <dfm-base/utils/desktopfile.h>
#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>

#include <QObject>
#include <QSet>
#include <QMimeType>
#include <QMap>
#include <QFileInfo>
#include <QTimer>
#include <QIcon>
#include <QDebug>
#include <QThread>
#include <QUrl>
#include <QStandardPaths>

namespace dfmbase {

class MimeAppsWorker : public QObject
{
    Q_OBJECT

public:
    explicit MimeAppsWorker(QObject *parent = nullptr);
    ~MimeAppsWorker();

    void initConnect();

public Q_SLOTS:
    void startWatch();
    void updateCache();
    void writeData(const QString &path, const QByteArray &content);
    QByteArray readData(const QString &path);

private:
    QTimer *updateCacheTimer = nullptr;
    QList<AbstractFileWatcherPointer> watcherGroup;
};

class MimesAppsManager : public QObject
{
    Q_OBJECT

public:
    static MimesAppsManager *instance();
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

    static bool setDefautlAppForTypeByGio(const QString &mimeType, const QString &appPath);

    static QStringList getRecommendedApps(const QUrl &url);
    static QStringList getRecommendedAppsByQio(const QMimeType &mimeType);
    static QStringList getRecommendedAppsByGio(const QString &mimeType);
    static QStringList getrecommendedAppsFromMimeWhiteList(const QUrl &url);

    static QStringList getApplicationsFolders();
    static QString getMimeAppsCacheFile();
    static QString getMimeInfoCacheFilePath();
    static QString getMimeInfoCacheFileRootPath();
    static QString getDesktopFilesCacheFile();
    static QString getDesktopIconsCacheFile();
    static QString getDDEMimeTypeFile();
    static QMap<QString, DesktopFile> getDesktopObjs();
    static void initMimeTypeApps();
    static void loadDDEMimeTypes();
    static bool lessByDateTime(const QFileInfo &f1, const QFileInfo &f2);
    static bool removeOneDupFromList(QStringList &list, const QString desktopFilePath);

Q_SIGNALS:
    void requestUpdateCache();

private:
    explicit MimesAppsManager(QObject *parent = nullptr);
    MimeAppsWorker *mimeAppsWorker = nullptr;
    QThread mimeAppsThread;
};

}

#endif   // MIMESAPPSMANAGER_H
