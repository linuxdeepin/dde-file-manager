/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <dfm-mount/base/dmount_global.h>

#include <QObject>
#include <DDialog>

using namespace DTK_NAMESPACE::Widget;

DFMBASE_BEGIN_NAMESPACE

class TaskDialog;
class ComputerPropertyDialog;
class TrashPropertyDialog;

class DialogManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DialogManager)

public:
    static DialogManager *instance();

public:
    enum MessageType {
        kMsgInfo = 1,
        kMsgWarn = 2,
        kMsgErr = 3
    };
    enum OperateType {
        kMount,
        kUnmount,
        kRemove,
    };

    DDialog *showQueryScanningDialog(const QString &title);

    void showErrorDialog(const QString &title, const QString &message);

    void showErrorDialogWhenOperateDeviceFailed(OperateType type, DFMMOUNT::DeviceError err);
    void showNoPermissionDialog(const QList<QUrl> &urls);
    void showCopyMoveToSelfDialog();

    int showMessageDialog(MessageType messageLevel, const QString &title, const QString &message = "", QString btnTxt = tr("Confirm", "button"));

    void addTask(const JobHandlePointer task);

    void showSetingsDialog(DFMBASE_NAMESPACE::FileManagerWindow *window);

    bool askForFormat();
    QString askPasswordForLockedDevice();

    int showRunExcutableScriptDialog(const QUrl &url);
    int showRunExcutableFileDialog(const QUrl &url);

    int showDeleteFilesClearTrashDialog(const QList<QUrl> &urlList, const bool showEmptyBtText = false);
    int showNormalDeleteConfirmDialog(const QList<QUrl> &urls);

    void showRestoreFailedDialog(const QList<QUrl> &urlList);

    // rename
    int showRenameNameSameErrorDialog(const QString &name);
    void showRenameBusyErrDialog();

private:
    explicit DialogManager(QObject *parent = nullptr);
    ~DialogManager();

    TaskDialog *taskdialog = nullptr;   // 文件任务进度和错误处理弹窗
    QIcon infoIcon;
    QIcon warningIcon;
    QIcon errorIcon;
};

DFMBASE_END_NAMESPACE

#define DialogManagerInstance DFMBASE_NAMESPACE::DialogManager::instance()

#endif   // DIALOGMANAGER_H
