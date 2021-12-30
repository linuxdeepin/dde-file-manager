/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#include "dialogservice.h"
#include "taskdialog/taskdialog.h"
#include "propertydialog/computerpropertydialog.h"
#include "propertydialog/trashpropertydialog.h"
#include "settingsdialog/settingdialog.h"

DSC_USE_NAMESPACE

DDialog *DialogService::showQueryScanningDialog(const QString &title)
{
    DDialog *d = new DDialog;
    d->setTitle(title);
    d->setAttribute(Qt::WA_DeleteOnClose);
    Qt::WindowFlags flags = d->windowFlags();
    d->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    d->setIcon(QIcon::fromTheme("dialog-warning"));
    d->addButton(QObject::tr("Cancel", "button"));
    d->addButton(QObject::tr("Stop", "button"), true, DDialog::ButtonWarning);   // 终止
    d->setMaximumWidth(640);
    d->show();
    return d;
}

void DialogService::showErrorDialog(const QString &title, const QString &message)
{
    DDialog d(title, message);
    Qt::WindowFlags flags = d.windowFlags();
    // dialog show top
    d.setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    d.setIcon(QIcon::fromTheme("dialog-error"));
    d.addButton(tr("Confirm", "button"), true, DDialog::ButtonRecommend);
    d.setMaximumWidth(640);
    d.exec();
}
/*!
 * \brief DialogService::addTask 添加一个文件操作任务，当收到这个任务的线程结束时，自己移除掉这个任务
 *
 * \param task 文件操作任务的处理器
 */
void DialogService::addTask(const JobHandlePointer &task)
{
    if (!taskdailog)
        taskdailog = new TaskDialog();

    taskdailog->addTask(task);
}

/*!
 * \brief 创建与显示计算机属性页面
 */
void DialogService::showComputerPropertyDialog()
{
    if (!computerPropertyDialog)
        computerPropertyDialog = new ComputerPropertyDialog();

    computerPropertyDialog->show();
}

/*!
 * \brief 创建与显示回收站属性页面
 */
void DialogService::showTrashPropertyDialog()
{
    if (!trashPropertyDialog)
        trashPropertyDialog = new TrashPropertyDialog();

    trashPropertyDialog->show();
}

void DialogService::showSetingsDialog(FileManagerWindow *window)
{
    Q_ASSERT(window);

    if (window->property("isSettingDialogShown").toBool()) {
        qWarning() << "isSettingDialogShown true";
        return;
    }
    window->setProperty("isSettingDialogShown", true);
    DSettingsDialog *dsd = new SettingDialog(window);
    dsd->show();
    connect(dsd, &DSettingsDialog::finished, [window] {
        window->setProperty("isSettingDialogShown", false);
    });
}

DialogService::DialogService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<DialogService>()
{
}

dfm_service_common::DialogService::~DialogService()
{
}
