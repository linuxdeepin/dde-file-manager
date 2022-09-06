// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERSHAREMANAGER_H
#define USERSHAREMANAGER_H

#include <QObject>

#include "dfmglobal.h"
#include "shareinfo.h"

class QDBusPendingCallWatcher;

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE
class UserShareInterface;
DFM_BEGIN_NAMESPACE
class DFileWatcherManager;
DFM_END_NAMESPACE

DFM_USE_NAMESPACE

class UserShareManager : public QObject
{
    Q_OBJECT
public:
    explicit UserShareManager(QObject *parent = nullptr);
    ~UserShareManager();

    inline static QString UserSharePath(){
        return "/var/lib/samba/usershares";
    }
    static QString CurrentUser;

    void initMonitorPath();
    void initConnect();
    QString getCacehPath();

    ShareInfo getOldShareInfoByNewInfo(const ShareInfo& newInfo) const;
    ShareInfo getShareInfoByPath(const QString& path) const;
    ShareInfo getsShareInfoByShareName(const QString& shareName) const;
    QString getShareNameByPath(const QString& path) const;
    uint getCreatorUidByShareName(const QString& shareName) const;
    ShareInfoList shareInfoList() const;
    int validShareInfoCount() const ;
    bool hasValidShareFolders() const;
    bool isShareFile(const QString &filePath) const;
    bool isSharePasswordSet(const QString &userName);

    static void writeCacheToFile(const QString &path, const QString &content);
    static QString readCacheFromFile(const QString &path);
    static QString getCurrentUserName();

signals:
    void userShareCountChanged(const int& count);
    void userShareAdded(const QString& path);
    void userShareDeleted(const QString& path);
    void userShareDeletedFailed(const QString &path);

public slots:
    void initSamaServiceSettings();
    void handleShareChanged(const QString &filePath);
    void updateUserShareInfo(bool sendSignal = true);
    void setSambaPassword(const QString& userName, const QString& password);
    bool addUserShare(const ShareInfo& info);

    void deleteUserShareByPath(const QString& path);
    void removeFiledeleteUserShareByPath(const QString& path);
    void onFileDeleted(const QString& filePath);
    void usershareCountchanged();

private slots:
    void callFinishedSlot(QDBusPendingCallWatcher *watcher);

private:
    void deleteUserShareByShareName(const QString& shareName);
    void loadUserShareInfoPathNames();
    void saveUserShareInfoPathNames();
    void updateFileAttributeInfo(const QString &filePath) const;
    void startSambaServiceAsync();


    DFileWatcherManager *m_fileMonitor = NULL;
    QTimer* m_shareInfosChangedTimer = NULL;
//    QTimer* m_lazyStartSambaServiceTimer = NULL;
    QMap<QString, ShareInfo> m_shareInfos = {};
    QMap<QString, QString> m_sharePathByFilePath = {};
    QMap<QString, QStringList> m_sharePathToNames = {};
    UserShareInterface* m_userShareInterface = NULL;
    ShareInfo m_currentInfo;
};

#endif // USERSHAREMANAGER_H
