/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef GVFSMOUNTCLIENT_H
#define GVFSMOUNTCLIENT_H

#include <QObject>
#include <QMutex>
#include "dfmevent.h"

#undef signals
extern "C" {
    #include <gio/gio.h>
}
#define signals public

class MountAskPasswordDialog;

class GvfsMountClient : public QObject
{
    Q_OBJECT

public:
    explicit GvfsMountClient(QObject *parent = 0);
    ~GvfsMountClient();

    void initConnect();

    enum MountOpState{
      MOUNT_OP_NONE,
      MOUNT_OP_ASKED,
      MOUNT_OP_ABORTED
    };

    static bool AskingPassword;
    static QJsonObject SMBLoginObj;
    static DFMUrlBaseEvent MountEvent;

    static void mount(GFile *file);
    static int mount_sync(const DFMUrlBaseEvent &event);
    static GMountOperation *new_mount_op(void);
    static void mount_done_cb(GObject *object, GAsyncResult *res, gpointer user_data);
    static void ask_password_cb(GMountOperation *op,
                     const char      *message,
                     const char      *default_user,
                     const char      *default_domain,
                     GAskPasswordFlags flags);
    static void ask_question_cb(GMountOperation *op, const char *message, const GStrv choices);

private:
    static QPointer<QEventLoop> eventLoop;
};

#endif // GVFSMOUNTCLIENT_H
