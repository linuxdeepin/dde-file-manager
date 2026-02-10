// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERSHAREHELPER_H
#define USERSHAREHELPER_H

#include "dfmplugin_dirshare_global.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <QObject>
#include <QTimer>
#include <QSharedPointer>
#include <QMap>
#include <QFuture>

class QDBusInterface;
namespace dfmplugin_dirshare {

using StartSambaFinished = std::function<void(bool, const QString &)>;
typedef QVariantMap ShareInfo;
typedef QList<QVariantMap> ShareInfoList;

class ShareWatcherManager;
class UserShareHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserShareHelper)

public:
    static UserShareHelper *instance();
    virtual ~UserShareHelper() override;

    static bool canShare(FileInfoPointer info);
    static bool needDisableShareWidget(FileInfoPointer info);

    bool share(const QVariantMap &info);
    bool isUserSharePasswordSet(const QString &username);
    void setSambaPasswd(const QString &userName, const QString &passwd);
    bool removeShareByPath(const QString &path);
    int readPort();
    ShareInfoList shareInfos();
    ShareInfo shareInfoByPath(const QString &path);
    ShareInfo shareInfoByShareName(const QString &name);
    QString shareNameByPath(const QString &path);
    uint whoShared(const QString &name);
    bool isShared(const QString &path);
    QString currentUserName();

    bool isSambaServiceRunning();
    void startSambaServiceAsync(StartSambaFinished onFinished);
    QString sharedIP() const;
    int getSharePort() const;
    void handleSetPassword(const QString &newPassword);

Q_SIGNALS:
    void shareCountChanged(int count);
    void shareAdded(const QString &path);
    void shareRemoved(const QString &path);
    void shareRemoveFailed(const QString &path);
    void sambaPasswordSet(bool success);

protected Q_SLOTS:
    void readShareInfos(bool sendSignal = true);
    void onShareChanged(const QString &path);
    void onShareFileDeleted(const QString &path);
    void onShareMoved(const QString &from, const QString &to);

private:
    explicit UserShareHelper(QObject *parent = nullptr);

    void initConnect();
    void initMonitorPath();

    bool removeShareByShareName(const QString &name, bool silent = false);
    void removeShareWhenShareFolderDeleted(const QString &deletedPath);
    ShareInfo getOldShareByNewShare(const ShareInfo &newShare);

    int runNetCmd(const QStringList &args, int wait = 30000, QString *err = nullptr);
    void handleErrorWhenShareFailed(int code, const QString &err) const;
    ShareInfo makeInfoByFileContent(const QMap<QString, QString> &contents);
    int validShareInfoCount() const;

    QPair<bool, QString> startSmbService();

    bool isValidShare(const ShareInfo &info) const;

    void emitShareCountChanged(int count);
    void emitShareAdded(const QString &path);
    void emitShareRemoved(const QString &path);
    void emitShareRemoveFailed(const QString &path);

    // Lazy initialization for D-Bus interface to avoid unnecessary service activation
    QDBusInterface *getUserShareInterface();

private:
    QTimer *pollingSharesTimer;
    QSharedPointer<QDBusInterface> userShareInter { nullptr };

    QMap<QString, ShareInfo> sharedInfos {};
    QMap<QString, QStringList> sharePathToShareName {};

    ShareWatcherManager *watcherManager { nullptr };
};
}

Q_DECLARE_METATYPE(DPDIRSHARE_NAMESPACE::StartSambaFinished)
Q_DECLARE_METATYPE(bool *)

#define UserShareHelperInstance DPDIRSHARE_NAMESPACE::UserShareHelper::instance()

#endif   // USERSHAREHELPER_H
