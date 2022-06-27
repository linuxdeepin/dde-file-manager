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

#include "usershare/usershare_defines.h"
#include "dfm_common_service_global.h"

#include <QObject>
#include <QTimer>
#include <QSharedPointer>
#include <QMap>
#include <QFuture>

class QDBusInterface;
namespace dfm_service_common {

class ShareWatcherManager;
class UserShareHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserShareHelper)

public:
    static UserShareHelper *instance();

    bool share(const ShareInfo &info);
    void setSambaPasswd(const QString &userName, const QString &passwd);
    void removeShareByPath(const QString &path);
    ShareInfoList shareInfos() const;
    ShareInfo getShareInfoByPath(const QString &path) const;
    ShareInfo getShareInfoByShareName(const QString &name) const;
    QString getShareNameByPath(const QString &path) const;
    uint getUidByShareName(const QString &name) const;
    bool isShared(const QString &path) const;
    QString getCurrentUserName() const;

    bool isSambaServiceRunning();
    void startSambaServiceAsync(StartSambaFinished onFinished);

    ~UserShareHelper();
    explicit UserShareHelper(QObject *parent = nullptr);

Q_SIGNALS:
    void shareCountChanged(int count);
    void shareAdded(const QString &path);
    void shareRemoved(const QString &path);
    void shareRemoveFailed(const QString &path);

protected Q_SLOTS:
    void readShareInfos(bool sendSignal = true);
    void onShareChanged(const QString &path);
    void onShareFileDeleted(const QString &path);
    void onShareMoved(const QString &from, const QString &to);

private:
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

private:
    QTimer *pollingSharesTimer;
    QSharedPointer<QDBusInterface> userShareInter { nullptr };

    QMap<QString, ShareInfo> sharedInfos {};
    QMap<QString, QStringList> sharePathToShareName {};

    ShareWatcherManager *watcherManager { nullptr };
};
}

#define UserShareHelperInstance DSC_NAMESPACE::UserShareHelper::instance()

#endif   // USERSHAREHELPER_H
