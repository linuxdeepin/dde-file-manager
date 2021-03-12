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
