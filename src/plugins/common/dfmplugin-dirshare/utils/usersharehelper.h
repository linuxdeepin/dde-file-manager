/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#ifndef USERSHAREHELPER_H
#define USERSHAREHELPER_H

#include "dfmplugin_dirshare_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

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

    static bool canShare(AbstractFileInfoPointer info);
    static bool needDisableShareWidget(AbstractFileInfoPointer info);

    bool share(const QVariantMap &info);
    bool isUserSharePasswordSet(const QString &username);
    void setSambaPasswd(const QString &userName, const QString &passwd);
    void removeShareByPath(const QString &path);
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
    bool handleRestoreViewCreationState();
    bool shareDrawerExpand();

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

    void removeShareByShareName(const QString &name);
    void removeShareWhenShareFolderDeleted(const QString &deletedPath);
    ShareInfo getOldShareByNewShare(const ShareInfo &newShare);

    int runNetCmd(const QStringList &args, int wait = 30000, QString *err = nullptr);
    void handleErrorWhenShareFailed(int code, const QString &err) const;
    ShareInfo makeInfoByFileContent(const QMap<QString, QString> &contents);
    int validShareInfoCount() const;

    QPair<bool, QString> startSmbService();
    bool setSmbdAutoStart();

    bool isValidShare(const ShareInfo &info) const;

    void emitShareCountChanged(int count);
    void emitShareAdded(const QString &path);
    void emitShareRemoved(const QString &path);
    void emitShareRemoveFailed(const QString &path);

private:
    QTimer *pollingSharesTimer;
    QSharedPointer<QDBusInterface> userShareInter { nullptr };

    QMap<QString, ShareInfo> sharedInfos {};
    QMap<QString, QStringList> sharePathToShareName {};

    ShareWatcherManager *watcherManager { nullptr };
    bool initExpandState { false };
};
}

Q_DECLARE_METATYPE(DPDIRSHARE_NAMESPACE::StartSambaFinished)
Q_DECLARE_METATYPE(bool *)

#define UserShareHelperInstance DPDIRSHARE_NAMESPACE::UserShareHelper::instance()

#endif   // USERSHAREHELPER_H
