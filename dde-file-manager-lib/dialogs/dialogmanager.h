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

#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <memory>

#include <QObject>
#include <QMap>
#include <QIcon>

#include "durl.h"
#include "dfmglobal.h"
#include "dtkwidget_global.h"

class DTaskDialog;
class FileJob;
class DAbstractFileInfo;
class DUrl;
class DFMEvent;
class DFMUrlBaseEvent;
class DFMUrlListBaseEvent;
class PropertyDialog;
class CloseAllDialogIndicator;
class TrashPropertyDialog;
class ComputerPropertyDialog;
class QTimer;
class DMultiFilePropertyDialog;
class QDiskInfo;

DFM_BEGIN_NAMESPACE
class FilePreviewDialog;
DFM_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DDialog;
DWIDGET_END_NAMESPACE

class DialogManager : public QObject
{
    Q_OBJECT

public:
    explicit DialogManager(QObject *parent = 0);
    ~DialogManager();
    void initData();
    void initTaskDialog();
    void initCloseIndicatorDialog();
    void initConnect();
    QPoint getPropertyPos(int dialogWidth, int dialogHeight);
    QPoint getPerportyPos(int dialogWidth, int dialogHeight, int count, int index);
    bool isTaskDialogEmpty();
    DTaskDialog *taskDialog() const;

public slots:
    void handleConflictRepsonseConfirmed(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response);
    void addJob(FileJob *job);
    void removeJob(const QString &jobId);
    QString getJobIdByUrl(const DUrl &url);
    void removeAllJobs();
    void updateJob();
    void startUpdateJobTimer();
    void stopUpdateJobTimer();

    void abortJob(const QMap<QString, QString> &jobDetail);
    void abortJobByDestinationUrl(const DUrl &url);

    void showCopyMoveToSelfDialog(const QMap<QString, QString> &jobDetail);

    void showUrlWrongDialog(const DUrl &url);
    int showRunExcutableScriptDialog(const DUrl &url, quint64 winId);
    int showRunExcutableFileDialog(const DUrl &url, quint64 winId);
    int showAskIfAddExcutableFlagAndRunDialog(const DUrl &url, quint64 winId);
    int showRenameNameSameErrorDialog(const QString &name, const DFMEvent &event);
    int showOpticalBlankConfirmationDialog(const DFMUrlBaseEvent &event);
    int showOpticalImageOpSelectionDialog(const DFMUrlBaseEvent &event);
    void showOpticalJobFailureDialog(int type, const QString &err, const QStringList &details);
    void showOpticalJobCompletionDialog(const QString &msg, const QString &icon);
    int showDeleteFilesClearTrashDialog(const DFMUrlListBaseEvent &event);
    int showRemoveBookMarkDialog(const DFMEvent &event);
    void showOpenWithDialog(const DFMEvent &event);
    void showOpenFilesWithDialog(const DFMEvent &event);
    void showPropertyDialog(const DFMUrlListBaseEvent &event);
    void showShareOptionsInPropertyDialog(const DFMUrlListBaseEvent &event);
    void showTrashPropertyDialog(const DFMEvent &event);
    void showComputerPropertyDialog();
    void showDevicePropertyDialog(const DFMEvent &event);
    void showDiskErrorDialog(const QString &id, const QString &errorText);
    void showBreakSymlinkDialog(const QString &targetName, const DUrl &linkfile);
    void showConnectToServerDialog(quint64 winId);
    void showUserSharePasswordSettingDialog(quint64 winId);
    void showGlobalSettingsDialog(quint64 winId);
    void showDiskSpaceOutOfUsedDialogLater();
    void showDiskSpaceOutOfUsedDialog();
    void show4gFat32Dialog();
    void showFailToCreateSymlinkDialog(const QString &errorString);
    void showMoveToTrashConflictDialog(const DUrlList &urls);
    void showDeleteSystemPathWarnDialog(quint64 winId);
    void showFilePreviewDialog(const DUrlList &selectUrls, const DUrlList &entryUrls);
    void showRestoreFailedDialog(const DUrlList &urlList);
    void showRestoreFailedPerssionDialog(const QString &srcPath, const QString &targetPath);
    void showMultiFilesRenameDialog(const QList<DUrl> &selectedUrls);
    void showAddUserShareFailedDialog(const QString &sharePath);
    void showNoPermissionDialog(const DFMUrlListBaseEvent &event);
    void showNtfsWarningDialog(const QDiskInfo &diskInfo);
    void showErrorDialog(const QString &title, const QString &message);
    void removePropertyDialog(const DUrl &url);
    void closeAllPropertyDialog();
    void updateCloseIndicator();
    void raiseAllPropertyDialog();
    void handleFocusChanged(QWidget *old, QWidget *now);
    void showTaskProgressDlgOnActive();
    int showUnableToLocateDir(const QString &dir);

    void refreshPropertyDialogs(const DUrl &oldUrl, const DUrl &newUrl);

    int showMessageDialog(int messageLevel, const QString &message);

#ifdef SW_LABEL
    void onJobFailed_SW(int nRet, const QString &jobType, const QString &srcfilename);
    int showPrivilegeDialog_SW(int nRet, const QString &srcfilename);
#endif

private:
    QIcon m_dialogInfoIcon;
    QIcon m_dialogWarningIcon;
    QIcon m_dialogErrorIcon;
    DTaskDialog *m_taskDialog {nullptr};
    CloseAllDialogIndicator *m_closeIndicatorDialog {nullptr};
    TrashPropertyDialog *m_trashDialog {nullptr};
    ComputerPropertyDialog *m_computerDialog {nullptr};
    QMap<QString, FileJob *> m_jobs {};
    QMap<QString, FileJob *> m_Opticaljobs {};
    QMap<DUrl, PropertyDialog *> m_propertyDialogs {};
    QTimer *m_closeIndicatorTimer {nullptr};
    QTimer *m_updateJobTaskTimer {nullptr};
    dde_file_manager::FilePreviewDialog *m_filePreviewDialog {nullptr};
    Dtk::Widget::DDialog *m_noPemesrsionDialog {nullptr};

    std::unique_ptr<DMultiFilePropertyDialog> m_multiFilesPropertyDialog;
};

#endif // DIALOGMANAGER_H
