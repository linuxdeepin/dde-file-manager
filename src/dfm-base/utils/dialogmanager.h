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

#include <dfm-mount/base/dfmmount_global.h>

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

    DDialog *showQueryScanningDialog(const QString &title);
    void showErrorDialog(const QString &title, const QString &message);
    int showMessageDialog(MessageType messageLevel, const QString &title, const QString &message = "", QString btnTxt = tr("Confirm", "button"));
    void showErrorDialogWhenMountDeviceFailed(DFMMOUNT::DeviceError err);
    void showErrorDialogWhenUnmountDeviceFailed(DFMMOUNT::DeviceError err);
    void addTask(const JobHandlePointer &task);
    void showSetingsDialog(DFMBASE_NAMESPACE::FileManagerWindow *window);
    QString askPasswordForLockedDevice();
    bool askForFormat();

private:
    explicit DialogManager(QObject *parent = nullptr);
    TaskDialog *taskdailog = nullptr;   // 文件任务进度和错误处理弹窗
};

DFMBASE_END_NAMESPACE

#define DialogManagerInstance DFMBASE_NAMESPACE::DialogManager::instance()

#endif   // DIALOGMANAGER_H
