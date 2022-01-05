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
#ifndef DIALOGSERVICE_H
#define DIALOGSERVICE_H

#include "dfm_common_service_global.h"

#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <dfm-framework/service/pluginservicecontext.h>

#include <DDialog>

using namespace DTK_NAMESPACE::Widget;
DSC_BEGIN_NAMESPACE
class TaskDialog;
class ComputerPropertyDialog;
class TrashPropertyDialog;
class DialogService final : public dpf::PluginService, dpf::AutoServiceRegister<DialogService>
{
    Q_OBJECT
    Q_DISABLE_COPY(DialogService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.DialogService";
    }

    DDialog *showQueryScanningDialog(const QString &title);
    void showErrorDialog(const QString &title, const QString &message);
    void addTask(const JobHandlePointer &task);
    void showComputerPropertyDialog();
    void showTrashPropertyDialog();
    void showSetingsDialog(DFMBASE_NAMESPACE::FileManagerWindow *window);
    QString askPasswordForLockedDevice();

private:
    explicit DialogService(QObject *parent = nullptr);
    virtual ~DialogService() override;
    TaskDialog *taskdailog = nullptr;   // 文件任务进度和错误处理弹窗
    ComputerPropertyDialog *computerPropertyDialog { nullptr };
    TrashPropertyDialog *trashPropertyDialog { nullptr };
};

DSC_END_NAMESPACE

#endif   // DIALOGSERVICE_H
