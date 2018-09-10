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

#include "gvfsmountclient.h"

#include "mountaskpassworddialog.h"

#include "app/filesignalmanager.h"
#include "app/define.h"

#include "views/windowmanager.h"
#include "dialogs/dialogmanager.h"

#include "singleton.h"

#include <QDebug>
#include <QTimer>
#include <QLoggingCategory>


bool GvfsMountClient::AskingPassword = false;
QJsonObject GvfsMountClient::SMBLoginObj = {};
DFMUrlBaseEvent GvfsMountClient::MountEvent = DFMUrlBaseEvent(Q_NULLPTR, DUrl());

QPointer<QEventLoop> GvfsMountClient::eventLoop;

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(mountClient, "gvfs.mountCli")
#else
Q_LOGGING_CATEGORY(mountClient, "gvfs.mountCli", QtInfoMsg)
#endif

GvfsMountClient::GvfsMountClient(QObject *parent) : QObject(parent)
{
    qCDebug(mountClient()) << "Create GvfsMountClient";
    initConnect();
}

GvfsMountClient::~GvfsMountClient()
{

}

void GvfsMountClient::initConnect()
{
//    connect(fileSignalManager, &FileSignalManager::requestSMBMount, this, &GvfsMountClient::mountByEvent);
}

void GvfsMountClient::mount(GFile *file)
{
    GMountOperation *op;

    if (file == NULL)
        return;

    op = new_mount_op();

    g_file_mount_enclosing_volume (file, static_cast<GMountMountFlags>(0), op, NULL, mount_done_cb, op);
}

int GvfsMountClient::mount_sync(const DFMUrlBaseEvent &event)
{
    MountEvent = event;
    QPointer<QEventLoop> oldEventLoop = eventLoop;
    QEventLoop event_loop;

    eventLoop = &event_loop;

    GFile* file = g_file_new_for_uri(event.fileUrl().toString().toUtf8().constData());

    if (file == NULL)
        return -1;

    GMountOperation *op = new_mount_op();
    g_file_mount_enclosing_volume(file, static_cast<GMountMountFlags>(0),
                                  op, nullptr, mount_done_cb, op);

    int ret = eventLoop->exec();

    if (oldEventLoop) {
        oldEventLoop->exit(ret);
    }

    return ret;
}

GMountOperation *GvfsMountClient::new_mount_op()
{
    GMountOperation *op;

    op = g_mount_operation_new ();

    g_object_set_data (G_OBJECT (op), "state", GINT_TO_POINTER (MOUNT_OP_NONE));

    g_signal_connect (op, "ask_password", G_CALLBACK (ask_password_cb), NULL);
    g_signal_connect (op, "ask_question", G_CALLBACK (ask_question_cb), NULL);

  /* TODO: we *should* also connect to the "aborted" signal but since the
   *       main thread is blocked handling input we won't get that signal
   *       anyway...
   */
    return op;
}

void GvfsMountClient::mount_done_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
    gboolean succeeded;
    GError *error = NULL;
    GMountOperation *op = static_cast<GMountOperation*>(user_data);

    succeeded = g_file_mount_enclosing_volume_finish (G_FILE (object), res, &error);

    if (!succeeded) {
        Q_ASSERT(error->domain == G_IO_ERROR);

        bool showWarnDlg = false;

        switch (error->code) {
        case G_IO_ERROR_FAILED_HANDLED: // Operation failed and a helper program has already interacted with the user. Do not display any error dialog.
            break;
        default:
            showWarnDlg = true;
            break;
        }

        if (showWarnDlg) {
            DThreadUtil::runInMainThread(dialogManager, &DialogManager::showErrorDialog,
                                         tr("Mounting device error"), QString(error->message));
        }

        qCDebug(mountClient()) << "g_file_mount_enclosing_volume_finish" << succeeded << error;
        qCDebug(mountClient()) << "username" << g_mount_operation_get_username(op) << error->message;
    } else {
        qCDebug(mountClient()) << "g_file_mount_enclosing_volume_finish" << succeeded << AskingPassword;
        if (AskingPassword) {
            SMBLoginObj.insert("id", MountEvent.url().toString());
            if (SMBLoginObj.value("passwordSave").toInt() == 2) {
                SMBLoginObj.remove("password");
                emit fileSignalManager->requsetCacheLoginData(SMBLoginObj);
            }
            SMBLoginObj = {};
            AskingPassword = false;
        } else {
            qCDebug(mountClient()) << "username" << g_mount_operation_get_username(op);
        }
    }

    if (eventLoop) {
        eventLoop->exit(succeeded ? 0 : -1);
    }

    emit fileSignalManager->requestChooseSmbMountedFile(MountEvent);
}

static QJsonObject requestPasswordDialog(WId parentWindowId, bool showDomainLine, const QJsonObject &data)
{
    MountAskPasswordDialog askPasswordDialog(WindowManager::getWindowById(parentWindowId));

    askPasswordDialog.setLoginData(data);
    askPasswordDialog.setDomainLineVisible(showDomainLine);

    int ret = askPasswordDialog.exec();

    if (ret == DDialog::Accepted) {
        return askPasswordDialog.getLoginData();
    }

    return QJsonObject();
}

void GvfsMountClient::ask_password_cb(GMountOperation *op, const char *message, const char *default_user, const char *default_domain, GAskPasswordFlags flags)
{
    bool anonymous = g_mount_operation_get_anonymous(op);
    GPasswordSave passwordSave = g_mount_operation_get_password_save(op);

    const char* default_password = g_mount_operation_get_password(op);

    qCDebug(mountClient()) << "anonymous" << anonymous;
    qCDebug(mountClient()) << "message" << message;
    qCDebug(mountClient()) << "username" << default_user;
    qCDebug(mountClient()) << "domain" << default_domain;
    qCDebug(mountClient()) << "password" << default_password;
    qCDebug(mountClient()) << "GAskPasswordFlags" << flags;
    qCDebug(mountClient()) << "passwordSave" << passwordSave;

    QJsonObject obj;
    obj.insert("message", message);
    obj.insert("anonymous", anonymous);
    obj.insert("username", default_user);
    obj.insert("domain", default_domain);
    obj.insert("password", default_password);
    obj.insert("GAskPasswordFlags", flags);
    obj.insert("passwordSave", passwordSave);

    QJsonObject loginObj = DThreadUtil::runInMainThread(requestPasswordDialog, MountEvent.windowId(), MountEvent.fileUrl().isSMBFile(), obj);

    if (!loginObj.isEmpty()) {
        anonymous = loginObj.value("anonymous").toBool();
        QString username = loginObj.value("username").toString();
        QString domain = loginObj.value("domain").toString();
        QString password = loginObj.value("password").toString();
        GPasswordSave passwordsaveFlag =  static_cast<GPasswordSave>(loginObj.value("passwordSave").toInt());

        SMBLoginObj = loginObj;

        if ((flags & G_ASK_PASSWORD_ANONYMOUS_SUPPORTED) && anonymous) {
            g_mount_operation_set_anonymous (op, TRUE);
        } else {
            if (flags & G_ASK_PASSWORD_NEED_USERNAME) {
                g_mount_operation_set_username (op, username.toStdString().c_str());
            }

            if (flags & G_ASK_PASSWORD_NEED_DOMAIN) {
                g_mount_operation_set_domain (op, domain.toStdString().c_str());
            }

            if (flags & G_ASK_PASSWORD_NEED_PASSWORD) {
                g_mount_operation_set_password (op, password.toStdString().c_str());
            }

            if (flags & G_ASK_PASSWORD_SAVING_SUPPORTED) {
                g_mount_operation_set_password_save(op, passwordsaveFlag);
            }
        }

        /* Only try anonymous access once. */
        if (anonymous &&
            GPOINTER_TO_INT (g_object_get_data (G_OBJECT (op), "state")) == MOUNT_OP_ASKED)
        {
            g_object_set_data (G_OBJECT (op), "state", GINT_TO_POINTER (MOUNT_OP_ABORTED));
            g_mount_operation_reply (op, G_MOUNT_OPERATION_ABORTED);
        }
        else
        {
            qCDebug(mountClient()) << "g_mount_operation_reply before";
//            g_object_set_data (G_OBJECT (op), "state", GINT_TO_POINTER (MOUNT_OP_ASKED));
            g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
            qCDebug(mountClient()) << "g_mount_operation_reply end";
        }
        AskingPassword = true;
        qCDebug(mountClient()) << "AskingPassword" << AskingPassword;

    } else {
        qCDebug(mountClient()) << "cancel connect";
        g_object_set_data (G_OBJECT (op), "state", GINT_TO_POINTER (MOUNT_OP_ABORTED));
        g_mount_operation_reply (op, G_MOUNT_OPERATION_ABORTED);
    }
}

static int requestAnswerDialog(WId parentWindowId, const QString& message, QStringList choices)
{
    DDialog askQuestionDialog(WindowManager::getWindowById(parentWindowId));

    askQuestionDialog.setMessage(message);
    askQuestionDialog.addButtons(choices);
    askQuestionDialog.setMaximumWidth(480);

    return askQuestionDialog.exec();
}

// blumia: This callback is mainly for sftp fingerprint identity dialog, but should works on any ask-question signal.
//         ref: https://www.freedesktop.org/software/gstreamer-sdk/data/docs/latest/gio/GMountOperation.html#GMountOperation-ask-question
void GvfsMountClient::ask_question_cb(GMountOperation *op, const char *message, const GStrv choices)
{
    char **ptr = choices;
    int choice;
    QStringList choiceList;

    QString oneMessage(message);
    qCDebug(mountClient()) << "ask_question_cb() message: " << message;

    while (*ptr) {
        QString oneOption = QString::asprintf("%s", *ptr++);
        qCDebug(mountClient()) << "ask_question_cb()  - option(s): " << oneOption;
        choiceList << oneOption;
    }

    choice = DThreadUtil::runInMainThread(requestAnswerDialog, MountEvent.windowId(), oneMessage, choiceList);
    qCDebug(mountClient()) << "ask_question_cb() user choice(start at 0): " << choice;

    // check if choose is invalid
    if (choice < 0 && choice >= choiceList.count()) {
        g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
        return;
    }

    g_mount_operation_set_choice(op, choice);
    g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);

    return;
}
