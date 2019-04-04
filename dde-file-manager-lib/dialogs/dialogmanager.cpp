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

#include "dialogmanager.h"
#include "closealldialogindicator.h"
#include "trashpropertydialog.h"
#include "computerpropertydialog.h"
#include "usersharepasswordsettingdialog.h"
#include "movetotrashconflictdialog.h"
#include "views/dfilemanagerwindow.h"
#include "interfaces/dstyleditemdelegate.h"
#include "shutil/mimetypedisplaymanager.h"
#include "previewdialog.h"
#include "filepreviewdialog.h"
#include "dfmsettingdialog.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfmevent.h"

#include "fileoperations/filejob.h"
#include "dfileservices.h"
#include "interfaces/dfmstandardpaths.h"

#include "dfileinfo.h"
#include "models/trashfileinfo.h"

#include "views/windowmanager.h"

#include "xutil.h"
#include "utils.h"
#include "dialogs/ddesktoprenamedialog.h"
#include "dialogs/dtaskdialog.h"
#include "dialogs/propertydialog.h"
#include "dialogs/openwithdialog.h"
#include "dialogs/dmultifilepropertydialog.h"
#include "plugins/pluginmanager.h"
#include "preview/previewinterface.h"

#include "deviceinfo/udisklistener.h"
#include "deviceinfo/udiskdeviceinfo.h"

#include "singleton.h"
#include "gvfs/gvfsmountmanager.h"
#include "partman/partition.h"

#ifdef SW_LABEL
#include "sw_label/llsdeepinlabellibrary.h"
#endif

#include <ddialog.h>
#include <DAboutDialog>
#include <dexpandgroup.h>
#include <dsettingsdialog.h>
#include <DSettingsWidgetFactory>
#include <DSettingsOption>
#include <QTimer>
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>
#include <qsettingbackend.h>

DTK_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DFM_USE_NAMESPACE

DialogManager::DialogManager(QObject *parent) : QObject(parent)
{
    initData();
    initTaskDialog();
    initCloseIndicatorDialog();
    initConnect();
}

DialogManager::~DialogManager()
{

}

void DialogManager::initData()
{
    m_dialogInfoIcon = QIcon::fromTheme("dialog-information", QIcon(":/images/dialogs/images/dialog_info.png"));
    m_dialogWarningIcon = QIcon::fromTheme("dialog-warning", QIcon(":/images/dialogs/images/dialog_warning.png"));
    m_dialogErrorIcon = QIcon::fromTheme("dialog-error", QIcon(":/images/dialogs/images/dialog_error.png"));
}

void DialogManager::initTaskDialog()
{
    m_taskDialog = new DTaskDialog;
    m_taskDialog->setWindowIcon(QIcon(":/images/images/dde-file-manager.svg"));
    m_taskDialog->setStyleSheet(getQssFromFile(":/qss/dialogs/qss/light.qss"));
    m_updateJobTaskTimer = new QTimer;
    m_updateJobTaskTimer->setInterval(1000);
    connect(m_updateJobTaskTimer, &QTimer::timeout, this, &DialogManager::updateJob);
}

void DialogManager::initCloseIndicatorDialog()
{
    m_closeIndicatorDialog = new CloseAllDialogIndicator;
    m_closeIndicatorDialog->setWindowIcon(QIcon(":/images/images/dde-file-manager.svg"));
    m_closeIndicatorDialog->setStyleSheet(getQssFromFile(":/qss/dialogs/qss/light.qss"));
    m_closeIndicatorTimer = new QTimer;
    m_closeIndicatorTimer->setInterval(1000);
    connect(m_closeIndicatorTimer, &QTimer::timeout, this, &DialogManager::updateCloseIndicator);
}

void DialogManager::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requestStartUpdateJobTimer, this, &DialogManager::startUpdateJobTimer);
    connect(fileSignalManager, &FileSignalManager::requestStopUpdateJobTimer, this, &DialogManager::stopUpdateJobTimer);

    connect(fileSignalManager, &FileSignalManager::requestAbortJob, this, &DialogManager::abortJobByDestinationUrl);

    connect(m_taskDialog, &DTaskDialog::conflictRepsonseConfirmed, this, &DialogManager::handleConflictRepsonseConfirmed);

    connect(m_taskDialog, &DTaskDialog::abortTask, this, &DialogManager::abortJob);
    connect(m_taskDialog, &DTaskDialog::closed, this, &DialogManager::removeAllJobs);
    connect(m_closeIndicatorDialog, &CloseAllDialogIndicator::allClosed, this, &DialogManager::closeAllPropertyDialog);

    connect(fileSignalManager, &FileSignalManager::requestShowUrlWrongDialog, this, &DialogManager::showUrlWrongDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowOpenWithDialog, this, &DialogManager::showOpenWithDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowPropertyDialog, this, &DialogManager::showPropertyDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowShareOptionsInPropertyDialog, this, &DialogManager::showShareOptionsInPropertyDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowComputerPropertyDialog, this, &DialogManager::showComputerPropertyDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowTrashPropertyDialog, this, &DialogManager::showTrashPropertyDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowDevicePropertyDialog, this, &DialogManager::showDevicePropertyDialog);
    connect(fileSignalManager, &FileSignalManager::showDiskErrorDialog,
            this, &DialogManager::showDiskErrorDialog);
    connect(fileSignalManager, &FileSignalManager::showAboutDialog,
            this, &DialogManager::showAboutDialog);
    connect(fileSignalManager, &FileSignalManager::requestShow4GFat32Dialog,
            this, &DialogManager::show4gFat32Dialog);
    connect(fileSignalManager, &FileSignalManager::requestShowRestoreFailedDialog,
            this, &DialogManager::showRestoreFailedDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowRestoreFailedPerssionDialog,
            this, &DialogManager::showRestoreFailedPerssionDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowNoPermissionDialog,
            this, &DialogManager::showNoPermissionDialog);

    connect(fileSignalManager, &FileSignalManager::requestShowAddUserShareFailedDialog,
            this, &DialogManager::showAddUserShareFailedDialog);

    connect(fileSignalManager, &FileSignalManager::requestShowFilePreviewDialog, this, &DialogManager::showFilePreviewDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowErrorDialog, this, &DialogManager::showErrorDialog);

    if (getGvfsMountManager(false)) {
        connect(gvfsMountManager, &GvfsMountManager::mount_added, this, &DialogManager::showNtfsWarningDialog);
    }


#ifdef SW_LABEL
    connect(fileSignalManager, &FileSignalManager::jobFailed, this, &DialogManager::onJobFailed_SW);
#endif

//    connect(qApp, &QApplication::focusChanged, this, &DialogManager::handleFocusChanged);

}

QPoint DialogManager::getPerportyPos(int dialogWidth, int dialogHeight, int count, int index)
{
    const QScreen *cursor_screen = Q_NULLPTR;
    const QPoint &cursor_pos = QCursor::pos();

    for (const QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(cursor_pos)) {
            cursor_screen = screen;
            break;
        }
    }

    if (!cursor_screen) {
        cursor_screen = qApp->primaryScreen();
    }

    int desktopWidth = cursor_screen->size().width();
    int desktopHeight = cursor_screen->size().height();
    int SpaceWidth = 20;
    int SpaceHeight = 100;
    int row, x, y;

    int numberPerRow =  desktopWidth / (dialogWidth + SpaceWidth);


    if (count % numberPerRow == 0) {
        row = count / numberPerRow;

    } else {
        row = count / numberPerRow + 1;

    }

    int dialogsWidth;
    if (count / numberPerRow > 0) {
        dialogsWidth = dialogWidth * numberPerRow + SpaceWidth * (numberPerRow - 1);
    } else {
        dialogsWidth = dialogWidth * (count % numberPerRow)  + SpaceWidth * (count % numberPerRow - 1);
    }

    int dialogsHeight = dialogHeight + SpaceHeight * (row - 1);

    x = (desktopWidth - dialogsWidth) / 2 + (dialogWidth + SpaceWidth) * (index % numberPerRow);

    y = (desktopHeight - dialogsHeight) / 2 + (index / numberPerRow) * SpaceHeight;

    return QPoint(x, y) + cursor_screen->geometry().topLeft();
}

bool DialogManager::isTaskDialogEmpty()
{
    if (m_taskDialog->getTaskListWidget()->count() == 0) {
        return true;
    }
    return false;
}

DTaskDialog *DialogManager::taskDialog() const
{
    return m_taskDialog;
}

void DialogManager::addJob(FileJob *job)
{
    m_jobs.insert(job->getJobId(), job);
    emit fileSignalManager->requestStartUpdateJobTimer();
    connect(job, &FileJob::requestJobAdded, m_taskDialog, &DTaskDialog::addTask);
    connect(job, &FileJob::requestJobRemoved, m_taskDialog, &DTaskDialog::delayRemoveTask);
    connect(job, &FileJob::requestJobRemovedImmediately, m_taskDialog, &DTaskDialog::removeTaskImmediately);
    connect(job, &FileJob::requestJobDataUpdated, m_taskDialog, &DTaskDialog::handleUpdateTaskWidget);
    connect(job, &FileJob::requestAbortTask, m_taskDialog, &DTaskDialog::handleTaskClose);
    connect(job, &FileJob::requestConflictDialogShowed, m_taskDialog, &DTaskDialog::showConflictDiloagByJob);
    connect(job, &FileJob::requestCopyMoveToSelfDialogShowed, this, &DialogManager::showCopyMoveToSelfDialog);
    connect(job, &FileJob::requestNoEnoughSpaceDialogShowed, this, &DialogManager::showDiskSpaceOutOfUsedDialogLater);
    connect(job, &FileJob::requestCanNotMoveToTrashDialogShowed, this, &DialogManager::showMoveToTrashConflictDialog);
}


void DialogManager::removeJob(const QString &jobId)
{
    if (m_jobs.contains(jobId)) {
        FileJob *job = m_jobs.value(jobId);
        job->setIsAborted(true);
        job->setApplyToAll(true);
        job->cancelled();
        m_jobs.remove(jobId);

//        if (job->getIsGvfsFileOperationUsed()){
//            if (job->getIsFinished()){
//                emit fileSignalManager->requestFreshFileView(job->getWindowId());
//            }
//        }
    }
    if (m_jobs.count() == 0) {
        emit fileSignalManager->requestStopUpdateJobTimer();
    }
}

void DialogManager::removeAllJobs()
{
    foreach (const QString &jobId, m_jobs.keys()) {
        removeJob(jobId);
    }
}

void DialogManager::updateJob()
{
    foreach (QString jobId, m_jobs.keys()) {
        FileJob *job = m_jobs.value(jobId);
        if (job) {
            if (job->currentMsec() - job->lastMsec() > FileJob::Msec_For_Display) {
                if (!job->isJobAdded()) {
                    job->jobAdded();
                    job->jobUpdated();
                } else {
                    job->jobUpdated();
                }
            }

//            if (job->getIsGvfsFileOperationUsed()){
//                if (job->getIsFinished()){
//                    emit fileSignalManager->requestFreshFileView(job->getWindowId());
//                }
//            }
        }
    }
}

void DialogManager::startUpdateJobTimer()
{
    m_updateJobTaskTimer->start();
}

void DialogManager::stopUpdateJobTimer()
{
    m_updateJobTaskTimer->stop();
}

void DialogManager::abortJob(const QMap<QString, QString> &jobDetail)
{
    QString jobId = jobDetail.value("jobId");
    removeJob(jobId);
}

void DialogManager::abortJobByDestinationUrl(const DUrl &url)
{
    qDebug() << url;
    foreach (QString jobId, m_jobs.keys()) {
        FileJob *job = m_jobs.value(jobId);
        qDebug() << jobId << job->getTargetDir();
        if (!QFile(job->getTargetDir()).exists()) {
            job->jobAborted();
        }
    }
}

void DialogManager::showCopyMoveToSelfDialog(const QMap<QString, QString> &jobDetail)
{
    DDialog d;
    d.setTitle(tr("Operation failed!"));
    d.setMessage(tr("Target folder is inside the source folder!"));
    QStringList buttonTexts;
    buttonTexts << tr("OK");
    d.addButton(buttonTexts[0], true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    d.setIcon(m_dialogWarningIcon, QSize(64, 64));
    QTimer::singleShot(200, &d, &DDialog::raise);
    int code = d.exec();
    qDebug() << code;
    if (code == 0) {
        qDebug() << "close CopyMoveToSelf dialog" << jobDetail;
    }
}

void DialogManager::showUrlWrongDialog(const DUrl &url)
{
    Q_UNUSED(url)
}

int DialogManager::showRunExcutableScriptDialog(const DUrl &url, quint64 winId)
{
    DDialog d(WindowManager::getWindowById(winId));
    int maxDisplayNameLength = 250;
    QString _fileDisplayName = QFileInfo(url.path()).fileName();
    QString fileDisplayName = d.fontMetrics().elidedText(_fileDisplayName, Qt::ElideRight, maxDisplayNameLength);
    QString message = tr("Do you want to run %1 or display its content?").arg(fileDisplayName);
    QString tipMessage = tr("It is an executable text file.");
    QStringList buttonKeys, buttonTexts;
    buttonKeys << "OptionCancel" << "OptionRun" << "OptionRunInTerminal" << "OptionDisplay";
    buttonTexts << tr("Cancel") << tr("Run") << tr("Run in terminal") << tr("Display");
    d.setIconPixmap(QIcon::fromTheme("application-x-shellscript").pixmap(64, 64));
    d.setTitle(message);
    d.setMessage(tipMessage);
//    d.addButtons(buttonTexts);
    d.addButton(buttonTexts[0], true);
    d.addButton(buttonTexts[1], false);
    d.addButton(buttonTexts[2], false);
    d.addButton(buttonTexts[3], false, DDialog::ButtonRecommend);
    d.setDefaultButton(3);
    d.setFixedWidth(480);
    int code = d.exec();
    return code;
}

int DialogManager::showRunExcutableFileDialog(const DUrl &url, quint64 winId)
{
    DDialog d(WindowManager::getWindowById(winId));
    const DAbstractFileInfoPointer &pfileInfo = fileService->createFileInfo(this, url);
    int maxDisplayNameLength = 200;
    QString _fileDisplayName = QFileInfo(url.path()).fileName();
    QString fileDisplayName = d.fontMetrics().elidedText(_fileDisplayName, Qt::ElideRight, maxDisplayNameLength);
    QString message = tr("Do you want to run %1?").arg(fileDisplayName);
    QString tipMessage = tr("It is an executable file.");
    d.addButton(tr("Cancel"));
    d.addButton(tr("Run in terminal"));
    d.addButton(tr("Run"), true, DDialog::ButtonRecommend);
    d.setTitle(message);
    d.setMessage(tipMessage);
    d.setIconPixmap(pfileInfo->fileIcon().pixmap(64, 64));
    int code = d.exec();
    return code;
}

int DialogManager::showAskIfAddExcutableFlagAndRunDialog(const DUrl &url, quint64 winId)
{
    DDialog d(WindowManager::getWindowById(winId));
    const DAbstractFileInfoPointer &pfileInfo = fileService->createFileInfo(this, url);
    // i18n text from: https://github.com/linuxdeepin/internal-discussion/issues/456 , seems a little weird..
    QString message = tr("This file is not executable, do you want to add the execute permission and run?");
    d.addButton(tr("Cancel"));
    d.addButton(tr("Run"), true, DDialog::ButtonRecommend);
    d.setTitle(message);
    d.setIconPixmap(pfileInfo->fileIcon().pixmap(64, 64));
    int code = d.exec();
    return code;
}


int DialogManager::showRenameNameSameErrorDialog(const QString &name, const DFMEvent &event)
{
    DDialog d(WindowManager::getWindowById(event.windowId()));
    QFontMetrics fm(d.font());
    d.setTitle(tr("\"%1\" already exists, please use another name.").arg(fm.elidedText(name, Qt::ElideMiddle, 150)));
    QStringList buttonTexts;
    buttonTexts << tr("Confirm");
    d.addButton(buttonTexts[0], true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    d.setIcon(m_dialogWarningIcon, QSize(64, 64));
    int code = d.exec();
    return code;
}


int DialogManager::showDeleteFilesClearTrashDialog(const DFMUrlListBaseEvent &event)
{
    QString ClearTrash = tr("Are you sure to empty %1 item?");
    QString ClearTrashMutliple = tr("Are you sure to empty %1 items?");
    QString DeleteFileName = tr("Permanently delete %1?");
    QString DeleteFileItems = tr("Permanently delete %1 items?");

    const int maxFileNameWidth = 250;

    DUrlList urlList = event.urlList();
    QStringList buttonTexts;
    buttonTexts << tr("Cancel") << tr("Delete");

    DDialog d;

    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    QFontMetrics fm(d.font());
    d.setIcon(QIcon::fromTheme("user-trash-full-opened"), QSize(64, 64));
    if (urlList.first() == DUrl::fromTrashFile("/")) {
        buttonTexts[1] = tr("Empty");
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, urlList.first());
        if (fileInfo->filesCount() == 1) {
            d.setTitle(ClearTrash.arg(fileInfo->filesCount()));
        } else {
            d.setTitle(ClearTrashMutliple.arg(fileInfo->filesCount()));
        }
    } else if (urlList.first().isLocalFile()) {
        if (urlList.size() == 1) {
            DFileInfo f(urlList.first());
            d.setTitle(DeleteFileName.arg(fm.elidedText(f.fileDisplayName(), Qt::ElideMiddle, maxFileNameWidth)));
        } else {
            d.setTitle(DeleteFileItems.arg(urlList.size()));
        }
    } else if (urlList.first().isTrashFile()) {
        if (urlList.size() == 1) {
            TrashFileInfo f(urlList.first());
            d.setTitle(DeleteFileName.arg(fm.elidedText(f.fileDisplayName(), Qt::ElideMiddle, maxFileNameWidth)));
        } else {
            d.setTitle(DeleteFileItems.arg(urlList.size()));
        }
    } else if (urlList.first().isTrashFile()) {
        if (urlList.size() == 1) {
            TrashFileInfo f(urlList.first());
            d.setTitle(DeleteFileName.arg(fm.elidedText(f.fileDisplayName(), Qt::ElideMiddle, maxFileNameWidth)));
        } else {
            d.setTitle(DeleteFileItems.arg(urlList.size()));
        }
    } else {
        d.setTitle(DeleteFileItems.arg(urlList.size()));
    }
    d.setMessage(tr("This action cannot be restored"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.moveToCenter();
    int code = d.exec();
    return code;
}

int DialogManager::showRemoveBookMarkDialog(const DFMEvent &event)
{
    DDialog d(WindowManager::getWindowById(event.windowId()));
    d.setTitle(tr("Sorry, unable to locate your bookmark directory, remove it?"));
    d.setMessage(" ");
    QStringList buttonTexts;
    buttonTexts << tr("Cancel") << tr("Remove");
    d.addButton(buttonTexts[0], true);
    d.addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    d.setDefaultButton(1);
    d.setIconPixmap(QIcon::fromTheme("folder-bookmark", QIcon::fromTheme("folder")).pixmap(64, 64));
    int code = d.exec();
    return code;
}

void DialogManager::showOpenWithDialog(const DFMEvent &event)
{
    QWidget *w = WindowManager::getWindowById(event.windowId());
    if (w) {
        OpenWithDialog *d = new OpenWithDialog(event.fileUrl());
        d->setDisplayPosition(OpenWithDialog::Center);
        d->exec();
    }
}

void DialogManager::showPropertyDialog(const DFMUrlListBaseEvent &event)
{
    const DUrlList &urlList  = event.urlList();
    int count = urlList.count();

    if (count <= MAX_PROPERTY_DIALOG_NUMBER) {

        foreach (const DUrl &url, urlList) {
            int index = urlList.indexOf(url);
            if (DFMGlobal::isComputerDesktopFile(url) || url == DUrl::fromComputerFile("/")) {
                showComputerPropertyDialog();
            } else if (DFMGlobal::isTrashDesktopFile(url) || url == DUrl::fromTrashFile("/")) {
                DFMEvent event(this);
                event.setData(url);
                showTrashPropertyDialog(event);
            } else {

                PropertyDialog *dialog;
                if (m_propertyDialogs.contains(url)) {
                    dialog = m_propertyDialogs.value(url);
                    dialog->raise();
                } else {
                    dialog = new PropertyDialog(event, url);
                    m_propertyDialogs.insert(url, dialog);
                    QPoint pos = getPerportyPos(dialog->size().width(), dialog->size().height(), count, index);

                    dialog->show();
                    dialog->move(pos);

                    connect(dialog, &PropertyDialog::closed, this, &DialogManager::removePropertyDialog);
                    //                connect(dialog, &PropertyDialog::raised, this, &DialogManager::raiseAllPropertyDialog);
                    QTimer::singleShot(100, dialog, &PropertyDialog::raise);
                }

                if (urlList.count() >= 2) {
                    m_closeIndicatorDialog->show();
                    m_closeIndicatorTimer->start();
                }
            }
        }

    } else {
        m_multiFilesPropertyDialog = std::unique_ptr<DMultiFilePropertyDialog> { new DMultiFilePropertyDialog{ std::move(urlList) } };
        m_multiFilesPropertyDialog->show();
        m_multiFilesPropertyDialog->moveToCenter();
        m_multiFilesPropertyDialog->raise();
    }
}

void DialogManager::showShareOptionsInPropertyDialog(const DFMUrlListBaseEvent &event)
{
    DUrl url = event.fileUrlList().first();
    showPropertyDialog(event);
    PropertyDialog *dialog;
    if (m_propertyDialogs.contains(url)) {
        dialog = m_propertyDialogs.value(url);
        if (dialog->expandGroup()->expands().count() > 1) {
            dialog->expandGroup()->expand(1)->setExpand(true);
        }
    }
}

void DialogManager::showTrashPropertyDialog(const DFMEvent &event)
{
    Q_UNUSED(event);
    if (m_trashDialog) {
        m_trashDialog->close();
    }
    m_trashDialog = new TrashPropertyDialog(DUrl::fromTrashFile("/"));
    connect(m_trashDialog, &TrashPropertyDialog::finished, [ = ]() {
        m_trashDialog = NULL;
    });
    QPoint pos = getPerportyPos(m_trashDialog->size().width(), m_trashDialog->size().height(), 1, 0);
    m_trashDialog->show();
    m_trashDialog->move(pos);

    TIMER_SINGLESHOT(100, {
        m_trashDialog->raise();
    }, this)
}

void DialogManager::showComputerPropertyDialog()
{
    if (m_computerDialog) {
        m_computerDialog->show();
        m_computerDialog->raise();
        return;
    }
    m_computerDialog = new ComputerPropertyDialog;
    QPoint pos = getPerportyPos(m_computerDialog->size().width(), m_computerDialog->size().height(), 1, 0);
    m_computerDialog->show();
    m_computerDialog->move(pos);

    TIMER_SINGLESHOT(100, {
        m_computerDialog->raise();
    }, this)
}

void DialogManager::showDevicePropertyDialog(const DFMEvent &event)
{
    QWidget *w = WindowManager::getWindowById(event.windowId());
    if (w) {
        PropertyDialog *dialog = new PropertyDialog(event, event.fileUrl());
        dialog->show();
    }
}

void DialogManager::showDiskErrorDialog(const QString &id, const QString &errorText)
{
    Q_UNUSED(errorText)

    static QSet<QString> dialogHadShowForId;

    if (dialogHadShowForId.contains(id)) {
        return;
    }

    UDiskDeviceInfoPointer info = deviceListener->getDevice(id);

    if (info) {
        DDialog d;
        d.setTitle(tr("Disk file is being used, can not unmount now"));
        d.setMessage(tr("Name: ") + info->fileDisplayName()/* + ", " + tr("Path: ") + info->getPath()*/);
        QStringList buttonTexts;
        buttonTexts << tr("Cancel") << tr("Force unmount");
        d.addButton(buttonTexts[0], true);
        d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
        d.setDefaultButton(0);
        d.setIcon(info->fileIcon(64, 64), QSize(64, 64));

        dialogHadShowForId << id;

        int code = d.exec();

        dialogHadShowForId.remove(id);

        if (code == 1) {
            deviceListener->forceUnmount(id);
        }
    }
}

void DialogManager::showBreakSymlinkDialog(const QString &targetName, const DUrl &linkfile)
{
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, linkfile);

    DDialog d;
    QString warnText = tr("%1 that this shortcut refers to has been changed or moved");
    QFontMetrics fm(d.font());
    QString _targetName = fm.elidedText(targetName, Qt::ElideMiddle, 120);
    d.setTitle(warnText.arg(_targetName));
    d.setMessage(tr("Do you want to delete this shortcut？"));
    QStringList buttonTexts;
    buttonTexts << tr("Cancel") << tr("Confirm");
    d.addButton(buttonTexts[0], true);
    d.addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    d.setDefaultButton(1);
    d.setIcon(fileInfo->fileIcon(), QSize(64, 64));
    int code = d.exec();
    if (code == 1) {
        DUrlList urls;
        urls << linkfile;
        fileService->moveToTrash(this, urls);
    }
}

void DialogManager::showAboutDialog(quint64 winId)
{
    QWidget *w = WindowManager::getWindowById(winId);
    if (!w || w->property("AboutDialogShown").toBool()) {
        return;
    }

    QIcon productIcon;
    productIcon.addFile(":/images/images/dde-file-manager_96.png", QSize(96, 96));
    productIcon.addFile(":/images/images/dde-file-manager_96@2x.png", QSize(192, 192));
    DAboutDialog *dialog = new DAboutDialog(w);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("");
    dialog->setProductIcon(productIcon);
    dialog->setProductName(qApp->applicationDisplayName());
    dialog->setVersion(tr("Version:") + " V" + qApp->applicationVersion());
    dialog->setAcknowledgementLink("https://www.deepin.org/acknowledgments/" + qApp->applicationName());
    dialog->setDescription(tr("File Manager is a file management tool independently "
                              "developed by Deepin Technology, featured with searching, "
                              "copying, trash, compression/decompression, file property "
                              "and other file management functions."));
    const QPoint global = w->mapToGlobal(w->rect().center());
    dialog->move(global.x() - dialog->width() / 2, global.y() - dialog->height() / 2);
    dialog->show();
    w->setProperty("AboutDialogShown", true);
    connect(dialog, &DAboutDialog::closed, [ = ] {
        w->setProperty("AboutDialogShown", false);
    });
}

void DialogManager::showUserSharePasswordSettingDialog(quint64 winId)
{
    QWidget *w = WindowManager::getWindowById(winId);
    if (!w || w->property("UserSharePwdSettingDialogShown").toBool()) {
        return;
    }

    UserSharePasswordSettingDialog *dialog = new UserSharePasswordSettingDialog(w);
    dialog->show();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &UserSharePasswordSettingDialog::finished, dialog, &UserSharePasswordSettingDialog::onButtonClicked);
    w->setProperty("UserSharePwdSettingDialogShown", true);
    connect(dialog, &UserSharePasswordSettingDialog::closed, [ = ] {
        w->setProperty("UserSharePwdSettingDialogShown", false);
    });
}

void DialogManager::showGlobalSettingsDialog(quint64 winId)
{
    QWidget *w = WindowManager::getWindowById(winId);
    if (!w) {
        return;
    }
    if (w->property("isSettingDialogShown").toBool()) {
        return;
    }

    w->setProperty("isSettingDialogShown", true);

    DSettingsDialog *dsd = new DFMSettingDialog(w);
    dsd->show();

    connect(dsd, &DSettingsDialog::finished, [ = ] {
        w->setProperty("isSettingDialogShown", false);
    });
}

void DialogManager::showDiskSpaceOutOfUsedDialogLater()
{
    QTimer::singleShot(200, [ = ] {
        showDiskSpaceOutOfUsedDialog();
    });
}

void DialogManager::showDiskSpaceOutOfUsedDialog()
{
    DDialog d;
    d.setIcon(m_dialogWarningIcon, QSize(64, 64));
    d.setTitle(tr("Target disk doesn't have enough space, unable to copy!"));
    d.addButton(tr("OK"));

    QRect rect = d.geometry();
    rect.moveCenter(qApp->desktop()->geometry().center());
    d.move(rect.x(), rect.y());
    d.exec();
}

void DialogManager::show4gFat32Dialog()
{
    DDialog d;
    d.setTitle(tr("Failed, file size must be less than 4GB."));
    d.setIcon(m_dialogWarningIcon, QSize(64, 64));
    d.addButton(tr("OK"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showFailToCreateSymlinkDialog(const QString &errorString)
{
    DDialog d;
    d.setTitle(tr("Fail to create symlink, cause:") + errorString);
    d.setIcon(m_dialogWarningIcon, QSize(64, 64));
    d.addButton(tr("OK"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showMoveToTrashConflictDialog(const DUrlList &urls)
{
    MoveToTrashConflictDialog d(0, urls);
    int code = d.exec();
    if (code == 1) {
        fileService->deleteFiles(this, urls, false);
    }
}

void DialogManager::showDeleteSystemPathWarnDialog(quint64 winId)
{
    DDialog d(WindowManager::getWindowById(winId));
    d.setTitle(tr("The selected files contain system file/directory, and it cannot be deleted"));
    d.setIcon(m_dialogWarningIcon, QSize(64, 64));
    d.addButton(tr("OK"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showFilePreviewDialog(const DUrlList &selectUrls, const DUrlList &entryUrls)
{
    DUrlList canPreivewlist;

    for (const DUrl &url : selectUrls) {
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

        if (info && (info->fileUrl().isLocalFile() || info->toQFileInfo().exists())) {
            canPreivewlist << info->fileUrl();
        }
    }

    if (canPreivewlist.isEmpty()) {
        return;
    }

    if (!m_filePreviewDialog) {
        m_filePreviewDialog = new FilePreviewDialog(canPreivewlist, nullptr);
        DPlatformWindowHandle::enableDXcbForWindow(m_filePreviewDialog, true);
    } else {
        m_filePreviewDialog->updatePreviewList(canPreivewlist);
    }

    if (canPreivewlist.count() == 1) {
        m_filePreviewDialog->setEntryUrlList(entryUrls);
    }

    m_filePreviewDialog->show();
    m_filePreviewDialog->raise();
}

void DialogManager::showRestoreFailedDialog(const DUrlList &urlList)
{
    DDialog d;
    d.setTitle(tr("Operation failed!"));
    if (urlList.count() == 1) {
        d.setMessage(tr("Target file removed or location changed"));
    } else if (urlList.count() > 1) {
        d.setMessage(tr("%1 files failed to restore, target file removed or location changed").arg(QString::number(urlList.count())));
    }
    d.setIcon(m_dialogWarningIcon, QSize(64, 64));
    d.addButton(tr("OK"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showRestoreFailedPerssionDialog(const QString &srcPath, const QString &targetPath)
{
    qDebug() << srcPath << "restore to" << targetPath;
    DDialog d;
    d.setTitle(tr("Operation failed!"));
    d.setMessage(tr("You do not have permission to operate file/folder!"));
    d.setIcon(m_dialogWarningIcon, QSize(64, 64));
    d.addButton(tr("OK"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showNoPermissionDialog(const DFMUrlListBaseEvent &event)
{
    DUrlList urls = event.urlList();
    qDebug() << urls << "no perssion";
    if (urls.isEmpty()) {
        return;
    }

    int ret = 0;
    QFont f;
    f.setPixelSize(16);
    QFontMetrics fm(f);
    int maxWith = qApp->primaryScreen()->size().width() * 3 / 4;

    if (urls.count() == 1) {
        DDialog d;
        d.setTitle(tr("You do not have permission to operate file/folder!"));
        QString message = urls.at(0).toLocalFile();

        if (fm.width(message) > maxWith) {
            message = fm.elidedText(message, Qt::ElideMiddle, maxWith - 10);
        }

        d.setMessage(message);
        d.setIcon(m_dialogWarningIcon, QSize(64, 64));
        d.addButton(tr("Confirm"), true, DDialog::ButtonRecommend);
        ret = d.exec();
    } else {

        DDialog d;
        QFrame *contentFrame = new QFrame;

        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(m_dialogWarningIcon.pixmap(64, 64));

        QLabel *titleLabel = new QLabel;
        titleLabel->setText(tr("Sorry, you don't have permission to operate the following %1 file/folder(s)!").arg(QString::number(urls.count())));

        QLabel *messageLabel = new QLabel;
        messageLabel->setScaledContents(true);

        QString message;
        for (int i = 0; i < urls.count(); i++) {
            if (i >= 10) {
                break;
            }
            QString s = QString("%1.%2").arg(QString::number(i + 1), urls.at(i).toLocalFile());
            if (fm.width(s) > maxWith) {
                s = fm.elidedText(s, Qt::ElideMiddle, maxWith - 10);
            }
            message += s + "\n";
        }
        messageLabel->setText(message);

        QVBoxLayout *contentLayout = new QVBoxLayout;
        contentLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
        contentLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
        contentLayout->addWidget(messageLabel, 0, Qt::AlignCenter);
        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->setSpacing(10);
        contentFrame->setLayout(contentLayout);

        d.addContent(contentFrame, Qt::AlignCenter);
        d.addButton(tr("Cancel"), false, DDialog::ButtonNormal);
        d.addButton(tr("View"), true, DDialog::ButtonRecommend);
        ret = d.exec();
    }
    if (ret) {
        QWidget *window = WindowManager::getWindowById(event.windowId());
        if (window) {
            DFileManagerWindow *w = static_cast<DFileManagerWindow *>(window);
            DUrl parentUrl = event.urlList().at(0).parentUrl();
            w->cd(parentUrl);
            window->raise();
            QTimer::singleShot(1000, [ = ] { emit fileSignalManager->requestSelectFile(event); });
        }
    }
}

void DialogManager::showNtfsWarningDialog(const QDiskInfo &diskInfo)
{
    QTimer::singleShot(1000, [ = ]{
        if (qApp->applicationName() == QMAKE_TARGET && !DFMGlobal::IsFileManagerDiloagProcess)
        {
            QString fstype = PartMan::Partition::getPartitionByDevicePath(diskInfo.unix_device()).fs();
            if (fstype == "ntfs") {
                // blumia: the ntfs partition will be read-only if user mount the ntfs device by the kernel driver.
                //         the fstype (from `df -T` or `mount`) will be "ntfs" rather than "fuseblk" if user use the kernel driver
                //         to mount the block device, so we use `df -t ntfs /dev/xxxxx` to check if the fstype is "ntfs".
                QProcess checkFsDrv;
                checkFsDrv.start("df", {"--output=fstype", diskInfo.unix_device()});
                checkFsDrv.waitForFinished(-1);
                QString dfStdOut = checkFsDrv.readAllStandardOutput().split('\n').value(1);
                if (dfStdOut == QStringLiteral("ntfs")) return;

                bool isReadOnly = false;
                DUrl mountUrl = DUrl(diskInfo.mounted_root_uri());
                QFileInfo mountFileInfo(mountUrl.toLocalFile());
                isReadOnly = mountFileInfo.isReadable() && !QFileInfo(DUrl(diskInfo.mounted_root_uri()).toLocalFile()).isWritable();
                qDebug() << DUrl(diskInfo.mounted_root_uri()).toLocalFile() << fstype << "isReadOnly:" << isReadOnly;
                if (isReadOnly) {
                    DDialog d;
                    QFrame *contentFrame = new QFrame;

                    QLabel *iconLabel = new QLabel;
                    iconLabel->setPixmap(m_dialogWarningIcon.pixmap(64, 64));

                    QLabel *titleLabel = new QLabel;
                    titleLabel->setText(tr("Mount partition%1 to be read only").arg(diskInfo.unix_device()));

                    QLabel *discriptionLabel = new QLabel;
                    discriptionLabel->setScaledContents(true);
                    discriptionLabel->setText(tr("Disks in Windows will be unable to read and write normally if check \"Turn on fast startup (recommended)\" in Shutdown settings"));

                    QLabel *messageTitleLabel = new QLabel;
                    messageTitleLabel->setScaledContents(true);
                    QString messageTitleMessage = tr("Please restore by the following steps to normally access Windows disk");
                    messageTitleLabel->setText(messageTitleMessage);

                    QLabel *messageLabel = new QLabel;
                    messageLabel->setScaledContents(true);

                    QString message1 = tr("1. Reboot to enter Windows");
                    QString message2 = tr("2. Uncheck \"Turn on fast startup\" and \"Hibernate\" in shutdown settings and reboot");
                    QString message3 = tr("3. Reboot and enter deepin");

                    messageLabel->setText(QString("%1\n%2\n%3").arg(message1, message2, message3));

                    QVBoxLayout *contentLayout = new QVBoxLayout;
                    contentLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
                    contentLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
                    contentLayout->addWidget(discriptionLabel, 0, Qt::AlignLeft);
                    contentLayout->addSpacing(10);
                    contentLayout->addWidget(messageTitleLabel, 0, Qt::AlignLeft);
                    contentLayout->addWidget(messageLabel, 0, Qt::AlignLeft);
                    contentLayout->setContentsMargins(100, 0, 100, 0);
                    contentLayout->setSpacing(10);
                    contentFrame->setLayout(contentLayout);

                    d.addContent(contentFrame, Qt::AlignCenter);
                    d.addButton(tr("Cancel"), false, DDialog::ButtonNormal);
                    d.addButton(tr("Reboot"), true, DDialog::ButtonRecommend);
                    int ret = d.exec();
                    if (ret == 1) {
                        qDebug() << "===================Reboot system=====================";
                        QDBusInterface sessionManagerIface("com.deepin.SessionManager",
                        "/com/deepin/SessionManager",
                        "com.deepin.SessionManager",
                        QDBusConnection::sessionBus());
                        sessionManagerIface.asyncCall("Reboot");
                    }
                }
            }
        }
    });
}

void DialogManager::showErrorDialog(const QString &title, const QString &message)
{
    DDialog d(title, message);

    d.setIcon(QIcon::fromTheme("dialog-error"), QSize(64, 64));
    d.addButton(tr("Confirm"), true, DDialog::ButtonRecommend);
    d.setMaximumWidth(640);
    d.exec();
}

void DialogManager::removePropertyDialog(const DUrl &url)
{
    if (m_propertyDialogs.contains(url)) {
        m_propertyDialogs.remove(url);
    }
    if (m_propertyDialogs.count() == 0) {
        m_closeIndicatorDialog->hide();
    }
}

void DialogManager::closeAllPropertyDialog()
{
    foreach (const DUrl &url, m_propertyDialogs.keys()) {
        m_propertyDialogs.value(url)->close();
    }
    if (m_closeIndicatorDialog) {
        m_closeIndicatorTimer->stop();
        m_closeIndicatorDialog->close();
    }
    if (m_trashDialog) {
        m_trashDialog->close();
    }

    if (m_computerDialog) {
        m_computerDialog->close();
    }
}

void DialogManager::updateCloseIndicator()
{
    qint64 size = 0;
    int fileCount = 0;
    foreach (PropertyDialog *d, m_propertyDialogs.values()) {
        size += d->getFileSize();
        fileCount += d->getFileCount();
    }
    m_closeIndicatorDialog->setTotalMessage(size, fileCount);
}

void DialogManager::raiseAllPropertyDialog()
{
    foreach (PropertyDialog *d, m_propertyDialogs.values()) {
        qDebug() << d->getUrl() << d->isVisible() << d->windowState();
//        d->showMinimized();
        d->showNormal();
        QtX11::utils::ShowNormalWindow(d);
        qobject_cast<QWidget *>(d)->showNormal();
        d->show();
        d->raise();
        qDebug() << d->getUrl() << d->isVisible() << d->windowState();
    }
    m_closeIndicatorDialog->raise();
}

void DialogManager::handleFocusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(old)
    Q_UNUSED(now)

    if (m_propertyDialogs.values().contains(qobject_cast<PropertyDialog *>(qApp->activeWindow()))) {
        raiseAllPropertyDialog();
    } else if (m_closeIndicatorDialog == qobject_cast<CloseAllDialogIndicator *>(qApp->activeWindow())) {
        raiseAllPropertyDialog();
    }
}

void DialogManager::refreshPropertyDialogs(const DUrl &oldUrl, const DUrl &newUrl)
{
    PropertyDialog *d = m_propertyDialogs.value(oldUrl);
    if (d) {
        m_propertyDialogs.remove(oldUrl);
        m_propertyDialogs.insert(newUrl, d);
    }
}


void DialogManager::handleConflictRepsonseConfirmed(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response)
{
    QString jobId = jobDetail.value("jobId");
    FileJob *job = m_jobs.value(jobId);
    if (job != NULL) {
        bool applyToAll = response.value("applyToAll").toBool();
        int code = response.value("code").toInt();
        job->setApplyToAll(applyToAll);
        //0 = coexist, 1 = replace, 2 = skip
        switch (code) {
        case 0:
            job->setIsCoExisted(true);
            job->setIsSkip(false);
            job->setReplace(false);
            job->started();
            break;
        case 1:
            job->setReplace(true);
            job->setIsCoExisted(false);
            job->setIsSkip(false);
            job->started();
            break;
        case 2:
            job->setIsSkip(true);
            job->setIsCoExisted(false);
            job->setReplace(false);
            job->cancelled();
            break;
        default:
            qDebug() << "unknown code" << code;
        }
    }
}

int DialogManager::showMessageDialog(int messageLevel, const QString &message)
{
    DDialog d;
    d.moveToCenter();
    d.setTitle(message);
    QStringList buttonTexts;
    buttonTexts << tr("Confirm");
    d.addButtons(buttonTexts);
    d.setDefaultButton(0);
    if (messageLevel == 1) {
        d.setIcon(m_dialogInfoIcon, QSize(64, 64));
    } else if (messageLevel == 2) {
        d.setIcon(m_dialogWarningIcon, QSize(64, 64));
    } else if (messageLevel == 3) {
        d.setIcon(m_dialogErrorIcon, QSize(64, 64));
    } else {
        d.setIcon(m_dialogInfoIcon, QSize(64, 64));
    }
    int code = d.exec();
    qDebug() << code;
    return code;
}


void DialogManager::showMultiFilesRenameDialog(const QList<DUrl> &selectedUrls)
{
    DDesktopRenameDialog renameDialog;

    renameDialog.moveToCenter();
    renameDialog.setDialogTitle(QObject::tr("Rename %1 Files").arg(QString::fromStdString(std::to_string(selectedUrls.size()))));

    std::size_t code{ static_cast<size_t>(renameDialog.exec()) };
    std::size_t modeIndex{ renameDialog.getCurrentModeIndex() };

    AppController::flagForDDesktopRenameBar.store(true, std::memory_order_seq_cst);

    if (static_cast<bool>(code) == true) { //###: yes!

        if (modeIndex == 0) {
            QPair<QString, QString> replaceModeValue{ renameDialog.getModeOneContent() };
            DFileService::instance()->multiFilesReplaceName(selectedUrls, replaceModeValue);

        } else if (modeIndex == 1) {
            QPair<QString, DFileService::AddTextFlags> addModeValue{ renameDialog.getModeTwoContent() };
            DFileService::instance()->multiFilesAddStrToName(selectedUrls, addModeValue);

        } else {
            QPair<QString, QString> customModeValue{ renameDialog.getModeThreeContent() };
            DFileService::instance()->multiFilesCustomName(selectedUrls, customModeValue);
        }


        AppController::multiSelectionFilesCache.second = 1; //###: give a number must be bigger than 0.
        // We modified files will invoke dfilesystemmodel::selectAndRenameFile() at background.
    }

}

void DialogManager::showAddUserShareFailedDialog(const QString &sharePath)
{
    (void)sharePath;

    DDialog d;
    d.setTitle(tr("Share folder can't be named after the current username"));
    d.setIcon(m_dialogWarningIcon, QSize(64, 64));
    d.addButton(tr("OK"), true, DDialog::ButtonRecommend);
    d.exec();
}


#ifdef SW_LABEL

void DialogManager::onJobFailed_SW(int nRet, const QString &jobType, const QString &srcfilename)
{
    if (LlsDeepinLabelLibrary::instance()->isCompletion()) {
        int ret = showPrivilegeDialog_SW(nRet, srcfilename);
        if (ret == 0) {
            qDebug() << jobType << nRet << srcfilename;
        }
    }
}

int DialogManager::showPrivilegeDialog_SW(int nRet, const QString &srcfilename)
{
    if (LlsDeepinLabelLibrary::instance()->isCompletion()) {
        qDebug() << "文件全路径名称" << srcfilename;
        qDebug() << "错误码；" << nRet;

        std::string serrordst = ""; //错误描述
        int nerrorlevel = 0; //错误级别
//        nRet =  lls_geterrordesc(nRet, serrordst, nerrorlevel);
        nRet =  LlsDeepinLabelLibrary::instance()->lls_geterrordesc()(nRet, serrordst, nerrorlevel);
        if (nRet == 0) {
            qDebug() << "错误描述:" << QString::fromStdString(serrordst);
            qDebug() << "错误级别:" << nerrorlevel;
            QString message = QString("%1 %2").arg(QFileInfo(srcfilename).fileName(), QString::fromStdString(serrordst));
            int code = showMessageDialog(nerrorlevel, message);
            return code;

        } else {
            qDebug() << "get error message fail" << nRet;
            return -1;
        }
    }
}

#endif

