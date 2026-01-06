// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/widgets/filemanagerwindow.h>

#include <dfm-mount/base/dmount_global.h>

#include <DSettingsWidgetFactory>
#include <DDialog>

#include <QObject>

using namespace DTK_NAMESPACE::Widget;

namespace dfmbase {

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
    enum OperateType {
        kMount,
        kUnmount,
        kRemove,
    };

    DDialog *showQueryScanningDialog(const QString &title);

    void showErrorDialog(const QString &title, const QString &message);

    void showErrorDialogWhenOperateDeviceFailed(OperateType type, DFMMOUNT::OperationErrorInfo err);
    void showNoPermissionDialog(const QList<QUrl> &urls);
    void showCopyMoveToSelfDialog();

    int showMessageDialog(const QString &title, const QString &message = "", QString btnTxt = tr("Confirm", "button"));
    int showMessageDialog(const QString &title, const QString &message, const QStringList &btnTxtList);

    void addTask(const JobHandlePointer task);

    void showSetingsDialog(DFMBASE_NAMESPACE::FileManagerWindow *window);

    bool askForFormat();
    QString askPasswordForLockedDevice(const QString &devName);

    int showRunExcutableScriptDialog(const QUrl &url);
    int showRunExcutableFileDialog(const QUrl &url);

    int showDeleteFilesDialog(const QList<QUrl> &urlList, bool isTrashFile = false);
    int showClearTrashDialog(const quint64 &count);
    int showNormalDeleteConfirmDialog(const QList<QUrl> &urls);

    void showRestoreFailedDialog(const int count);
    void showOperationFailedDialog(const QMap<QUrl, QString> &failedInfo);
    int showRestoreDeleteFilesDialog(const QList<QUrl> &urlList);

    // rename
    int showRenameNameSameErrorDialog(const QString &name);
    void showRenameBusyErrDialog();
    int showRenameNameDotBeginDialog();
    int showUnableToVistDir(const QString &dir);

    // link file not exist
    DFMBASE_NAMESPACE::GlobalEventType showBreakSymlinkDialog(const QString &targetName, const QUrl &linkfile);
    int showAskIfAddExcutableFlagAndRunDialog();
    void showDeleteSystemPathWarnDialog(quint64 winId);

    // 注册自定义控件创建器
    void registerSettingWidget(const QString &viewType,
                               std::function<DSettingsWidgetFactory::WidgetCreateHandler> handler);

private:
    explicit DialogManager(QObject *parent = nullptr);
    ~DialogManager();

    TaskDialog *taskdialog = nullptr;   // 文件任务进度和错误处理弹窗

    // 存储自定义控件创建器的映射
    QMap<QString, std::function<DSettingsWidgetFactory::WidgetCreateHandler>> settingWidgetCreators;
};

}

#define DialogManagerInstance DFMBASE_NAMESPACE::DialogManager::instance()

#endif   // DIALOGMANAGER_H
