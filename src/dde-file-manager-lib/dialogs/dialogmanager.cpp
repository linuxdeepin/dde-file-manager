/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

//fix:一旦刻录失败，必须清楚磁盘临时缓存的数据文件，否则下次刻录操作等就会报一些错误，不能正常进行操作流程
#include "qfile.h"
#include "dfilemenumanager.h"
#include "disomaster.h"

#include "dialogmanager.h"
#include "closealldialogindicator.h"
#include "trashpropertydialog.h"
#include "computerpropertydialog.h"
#include "usersharepasswordsettingdialog.h"
#include "movetotrashconflictdialog.h"
#include "views/dfilemanagerwindow.h"
#include "interfaces/dfmstyleditemdelegate.h"
#include "shutil/mimetypedisplaymanager.h"
#include "filepreviewdialog.h"
#include "dfmsettingdialog.h"
#include "connecttoserverdialog.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfmevent.h"

#include "fileoperations/filejob.h"
#include "dfileservices.h"
#include "interfaces/dfmstandardpaths.h"

#include "dfileinfo.h"
#include "models/trashfileinfo.h"

#include "controllers/vaultcontroller.h"

#include "views/windowmanager.h"

#include "xutil.h"
#include "utils.h"
#include "dialogs/ddesktoprenamedialog.h"
#include "dialogs/dtaskdialog.h"
#include "dialogs/propertydialog.h"
#include "dialogs/openwithdialog.h"
#include "dialogs/dmultifilepropertydialog.h"
#include "bluetooth/bluetoothtransdialog.h"
#include "plugins/pluginmanager.h"
#include "preview/previewinterface.h"
#include "views/dfmopticalmediawidget.h"

#include "deviceinfo/udisklistener.h"
#include "deviceinfo/udiskdeviceinfo.h"

#include "singleton.h"
#include "gvfs/gvfsmountmanager.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "utils/desktopinfo.h"

#ifdef SW_LABEL
#include "sw_label/llsdeepinlabellibrary.h"
#endif

#include <DDrawer>
#include <DDialog>
#include <DAboutDialog>
#include <DSettingsDialog>
#include <DSettingsWidgetFactory>
#include <DSettingsOption>
#include <QTimer>
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>
#include <DSysInfo>
#include <ddiskdevice.h>

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
    m_dialogInfoIcon = QIcon::fromTheme("dialog-information");
    m_dialogWarningIcon = QIcon::fromTheme("dialog-warning");
    m_dialogErrorIcon = QIcon::fromTheme("dialog-error");
}

void DialogManager::initTaskDialog()
{
    m_taskDialog = new DTaskDialog;
    m_taskDialog->setWindowIcon(QIcon::fromTheme("dde-file-manager"));
    m_updateJobTaskTimer = new QTimer(this);
    m_updateJobTaskTimer->setInterval(1000);
    connect(m_updateJobTaskTimer, &QTimer::timeout, this, &DialogManager::updateJob);
}

void DialogManager::initCloseIndicatorDialog()
{
    m_closeIndicatorDialog = new CloseAllDialogIndicator;
    m_closeIndicatorDialog->setWindowIcon(QIcon::fromTheme("dde-file-manager"));
    m_closeIndicatorTimer = new QTimer(this);
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
    connect(fileSignalManager, &FileSignalManager::requestShowOpenFilesWithDialog, this, &DialogManager::showOpenFilesWithDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowPropertyDialog, this, &DialogManager::showPropertyDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowShareOptionsInPropertyDialog, this, &DialogManager::showShareOptionsInPropertyDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowComputerPropertyDialog, this, &DialogManager::showComputerPropertyDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowTrashPropertyDialog, this, &DialogManager::showTrashPropertyDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowDevicePropertyDialog, this, &DialogManager::showDevicePropertyDialog);
    connect(fileSignalManager, &FileSignalManager::showDiskErrorDialog,
            this, &DialogManager::showDiskErrorDialog);
    connect(fileSignalManager, &FileSignalManager::requestShow4GFat32Dialog,
            this, &DialogManager::show4gFat32Dialog);
    connect(fileSignalManager, &FileSignalManager::requestShowRestoreFailedDialog,
            this, &DialogManager::showRestoreFailedDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowRestoreFailedPerssionDialog,
            this, &DialogManager::showRestoreFailedPerssionDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowRestoreFailedSourceNotExist,
            this, &DialogManager::showRestoreFailedSourceNotExists);
    connect(fileSignalManager, &FileSignalManager::requestShowNoPermissionDialog,
            this, &DialogManager::showNoPermissionDialog);

    connect(fileSignalManager, &FileSignalManager::requestShowAddUserShareFailedDialog,
            this, &DialogManager::showAddUserShareFailedDialog);

    connect(fileSignalManager, &FileSignalManager::requestShowFilePreviewDialog, this, &DialogManager::showFilePreviewDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowErrorDialog, this, &DialogManager::showErrorDialog);
    connect(fileSignalManager, &FileSignalManager::activeTaskDlg, this, &DialogManager::showTaskProgressDlgOnActive);
#ifdef SW_LABEL
    connect(fileSignalManager, &FileSignalManager::jobFailed, this, &DialogManager::onJobFailed_SW);
#endif

//    connect(qApp, &QApplication::focusChanged, this, &DialogManager::handleFocusChanged);

}

QPoint DialogManager::getPropertyPos(int dialogWidth, int dialogHeight)
{
    const QScreen *cursor_screen = Q_NULLPTR;
    const QPoint &cursor_pos = QCursor::pos();

    auto screens = qApp->screens();
    auto iter = std::find_if(screens.begin(), screens.end(), [cursor_pos](const QScreen * screen) {
        return screen->geometry().contains(cursor_pos);
    });

    if (iter != screens.end()) {
        cursor_screen = *iter;
    }

    if (!cursor_screen) {
        cursor_screen = qApp->primaryScreen();
    }

    int desktopWidth = cursor_screen->size().width();
    int desktopHeight = cursor_screen->size().height();
    int x = (desktopWidth - dialogWidth) / 2;

    int y = (desktopHeight - dialogHeight) / 2;

    return QPoint(x, y) + cursor_screen->geometry().topLeft();
}

QPoint DialogManager::getPerportyPos(int dialogWidth, int dialogHeight, int count, int index)
{
    Q_UNUSED(dialogHeight)
    const QScreen *cursor_screen = Q_NULLPTR;
    const QPoint &cursor_pos = QCursor::pos();

    auto screens = qApp->screens();
    auto iter = std::find_if(screens.begin(), screens.end(), [cursor_pos](const QScreen * screen) {
        return screen->geometry().contains(cursor_pos);
    });

    if (iter != screens.end()) {
        cursor_screen = *iter;
    }

    if (!cursor_screen) {
        cursor_screen = qApp->primaryScreen();
    }

    int desktopWidth = cursor_screen->size().width();
//    int desktopHeight = cursor_screen->size().height();//后面未用，注释掉
    int SpaceWidth = 20;
    int SpaceHeight = 70;
    int row, x, y;
    int numberPerRow =  desktopWidth / (dialogWidth + SpaceWidth);
    Q_ASSERT(numberPerRow != 0);
    if (count % numberPerRow == 0) {
        row = count / numberPerRow;
    } else {
        row = count / numberPerRow + 1;
    }
    Q_UNUSED(row)
    int dialogsWidth;
    if (count / numberPerRow > 0) {
        dialogsWidth = dialogWidth * numberPerRow + SpaceWidth * (numberPerRow - 1);
    } else {
        dialogsWidth = dialogWidth * (count % numberPerRow)  + SpaceWidth * (count % numberPerRow - 1);
    }

//    int dialogsHeight = dialogHeight + SpaceHeight * (row - 1);//未用注释掉

    x = (desktopWidth - dialogsWidth) / 2 + (dialogWidth + SpaceWidth) * (index % numberPerRow);

    y = 5 + (index / numberPerRow) * SpaceHeight;
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

void DialogManager::addJob(QSharedPointer<FileJob> job)
{
    QMutexLocker locker(&m_mutexJob);
    qInfo() << "add job: " << job->jobTypeToString() << "," << job->getJobId();

    m_jobs.insert(job->getJobId(), job);
    emit fileSignalManager->requestStartUpdateJobTimer();

    job->disconnect(m_taskDialog); // 对于光盘刻录、擦除的 job，可能会因为进度框的关闭打开而多次被添加导致多次连接槽，所以这里在连接前先断开这部分的信号槽
    job->disconnect(this);
    connect(job.data(), &FileJob::requestJobAdded, m_taskDialog, &DTaskDialog::addTask);
    connect(job.data(), &FileJob::requestJobRemoved, m_taskDialog, &DTaskDialog::delayRemoveTask);
    connect(job.data(), &FileJob::requestJobRemovedImmediately, m_taskDialog, &DTaskDialog::removeTaskImmediately);
    connect(job.data(), &FileJob::requestJobDataUpdated, m_taskDialog, &DTaskDialog::handleUpdateTaskWidget);
    connect(job.data(), &FileJob::requestAbortTask, m_taskDialog, &DTaskDialog::handleTaskClose);
    connect(job.data(), &FileJob::requestCopyMoveToSelfDialogShowed, this, &DialogManager::showCopyMoveToSelfDialog);
    connect(job.data(), &FileJob::requestNoEnoughSpaceDialogShowed, this, &DialogManager::showDiskSpaceOutOfUsedDialogLater);
    connect(job.data(), &FileJob::requestCanNotMoveToTrashDialogShowed, this, &DialogManager::showMoveToTrashConflictDialog);
    connect(job.data(), &FileJob::requestOpticalJobFailureDialog, this, &DialogManager::showOpticalJobFailureDialog);
    connect(job.data(), &FileJob::requestOpticalJobCompletionDialog, this, &DialogManager::showOpticalJobCompletionDialog);
}


void DialogManager::removeJob(const QString &jobId, bool isRemoveOpticalJob)
{
    QMutexLocker locker(&m_mutexJob);

    if (m_jobs.contains(jobId)) {
        QSharedPointer<FileJob> job = m_jobs.value(jobId);
        if (job->getIsOpticalJob() && !job->getIsFinished()) {
            if(!isRemoveOpticalJob) {
                qDebug() << "ignore to remove job: " << job->jobTypeToString() << "," << job->getJobId();
                return;
            }
        }
        qInfo() << "remove job: " << job->jobTypeToString() << "," << job->getJobId();

        job->setIsAborted(true);
        job->setApplyToAll(true);
        job->cancelled();
        m_jobs.remove(jobId);
    }
    if (m_jobs.count() == 0) {
        emit fileSignalManager->requestStopUpdateJobTimer();
    }
}

QString DialogManager::getJobIdByUrl(const DUrl &url)
{
    foreach (const QString &jobId, m_jobs.keys()) {
        QSharedPointer<FileJob> job = m_jobs.value(jobId);
        bool ret = false;
        QStringList pathlist = job->property("pathlist").toStringList();

        auto iter = std::find_if(pathlist.begin(), pathlist.end(), [url](const QString & path) {
            return path == url.toLocalFile();
        });

        if (iter != pathlist.end()) {
            ret = true;
        }

        if (ret) {
            return job->getJobId();
        }
    }

    return QString();
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
        QSharedPointer<FileJob> job = m_jobs.value(jobId);
        if (job) {
            if (!job->isCanShowProgress())
                return;

            if (job->currentMsec() - job->lastMsec() > FileJob::Msec_For_Display) {
                if (!job->isJobAdded()) {
                    job->jobAdded();
                    job->jobUpdated();
                } else {
                    job->jobUpdated();
                }
            }
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
        QSharedPointer<FileJob> job = m_jobs.value(jobId);
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
    buttonTexts.append(tr("OK","button"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    d.setIcon(m_dialogWarningIcon);
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
    buttonKeys.append("OptionCancel");
    buttonKeys.append("OptionRun");
    buttonKeys.append("OptionRunInTerminal");
    buttonKeys.append("OptionDisplay");
    buttonTexts.append(tr("Cancel","button"));
    buttonTexts.append(tr("Run","button"));
    buttonTexts.append(tr("Run in terminal","button"));
    buttonTexts.append(tr("Display","button"));
    d.setIcon(QIcon::fromTheme("application-x-shellscript"));
    d.setTitle(message);
    d.setMessage(tipMessage);
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
    d.addButton(tr("Cancel","button"));
    d.addButton(tr("Run in terminal","button"));
    d.addButton(tr("Run","button"), true, DDialog::ButtonRecommend);
    d.setTitle(message);
    d.setMessage(tipMessage);
    d.setIcon(pfileInfo->fileIcon());
    int code = d.exec();
    return code;
}

int DialogManager::showAskIfAddExcutableFlagAndRunDialog(const DUrl &url, quint64 winId)
{
    DDialog d(WindowManager::getWindowById(winId));
    const DAbstractFileInfoPointer &pfileInfo = fileService->createFileInfo(this, url);
    // i18n text from: https://github.com/linuxdeepin/internal-discussion/issues/456 , seems a little weird..
    QString message = tr("This file is not executable, do you want to add the execute permission and run?");
    d.addButton(tr("Cancel","button"));
    d.addButton(tr("Run","button"), true, DDialog::ButtonRecommend);
    d.setTitle(message);
    d.setIcon(pfileInfo->fileIcon());
    int code = d.exec();
    return code;
}


int DialogManager::showRenameNameSameErrorDialog(const QString &name, const DFMEvent &event)
{
    DDialog d(WindowManager::getWindowById(event.windowId()));
    QFontMetrics fm(d.font());
    d.setTitle(tr("\"%1\" already exists, please use another name.").arg(fm.elidedText(name, Qt::ElideMiddle, 150)));
    QStringList buttonTexts;
    buttonTexts.append(tr("Confirm","button"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    d.setIcon(m_dialogWarningIcon);
    int code = d.exec();
    return code;
}

int DialogManager::showRenameNameDotDotErrorDialog(const DFMEvent &event)
{
    // 获取父对话框字体特性
    DDialog d(WindowManager::getWindowById(event.windowId()));
    QFontMetrics fm(d.font());
    d.setTitle(tr("The file name must not contain two dots (..)"));
    QStringList buttonTexts;
    buttonTexts.append(tr("Confirm","button"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    // 设置对话框icon
    d.setIcon(m_dialogWarningIcon);
    int code = d.exec();
    return code;
}

void DialogManager::showRenameBusyErrDialog(const DFMEvent &event)
{
    // 获取父对话框字体特性
    DDialog d(WindowManager::getWindowById(event.windowId()));
    QFontMetrics fm(d.font());
    d.setTitle(tr("Device or resource busy"));
    QStringList buttonTexts;
    buttonTexts.append(tr("Confirm","button"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    // 设置对话框icon
    d.setIcon(m_dialogWarningIcon);
    d.exec();
}

int DialogManager::showOpticalBlankConfirmationDialog(const DFMUrlBaseEvent &event)
{
    QString EraseDisk = tr("Are you sure you want to erase all data on the disc?");

    DUrl url = event.url();
    qDebug() << url;
    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel","button"));
    buttonTexts.append(tr("Erase","button"));

    DDialog d;

    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    d.setTitle(EraseDisk);
    d.setMessage(tr("This action cannot be undone"));
    d.setIcon(QIcon::fromTheme("media-optical").pixmap(64, 64));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.moveToCenter();
    int code = d.exec();
    return code;
}

int DialogManager::showOpticalImageOpSelectionDialog(const DFMUrlBaseEvent &event)
{
    QString EraseDisk = tr("How do you want to use this disc?");

    DUrl url = event.url();
    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel","button"));
    buttonTexts.append(tr("Burn image","button"));
    buttonTexts.append(tr("Burn files","button"));

    DDialog d;

    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    d.setTitle(EraseDisk);
    d.setIcon(QIcon::fromTheme("media-optical").pixmap(64, 64));
    d.addButton(buttonTexts[0], false, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonNormal);
    d.addButton(buttonTexts[2], true, DDialog::ButtonRecommend);
    d.setDefaultButton(2);
    d.getButton(2)->setFocus();
    d.moveToCenter();
    int code = d.exec();
    return code;
}

void DialogManager::showOpticalJobFailureDialog(int type, const QString &err, const QStringList &details)
{
    DDialog d;
    d.setIcon(QIcon::fromTheme("dialog-error"));
    QString failure_type;
    switch (type) {
    case FileJob::OpticalBlank:
        failure_type = tr("Disc erase failed");
        break;
    case FileJob::OpticalBurn:
    case FileJob::OpticalImageBurn:
        failure_type = tr("Burn process failed");
        break;
    case FileJob::OpticalCheck:
        failure_type = tr("Data verification failed");
        break;
    }
    QString failure_str = QString(tr("%1: %2")).arg(failure_type).arg(err);
    d.setTitle(failure_str);
    QWidget *detailsw = new QWidget(&d);
    detailsw->setLayout(new QVBoxLayout());
    QTextEdit *te = new QTextEdit();
    te->setPlainText(details.join('\n'));
    te->setReadOnly(true);
    te->hide();
    detailsw->layout()->addWidget(te);
    connect(&d, &DDialog::buttonClicked, this, [failure_str, te, &d](int idx, const QString &) {
        if (idx == 1) {
            d.done(idx);
            return;
        }
        if (te->isVisible()) {
            te->hide();
            d.getButton(0)->setText(tr("Show details"));
            d.setTitle(failure_str);
        } else {
            te->show();
            d.getButton(0)->setText(tr("Hide details"));
            d.setTitle(tr("Error"));
        }
    });

    detailsw->setFixedWidth(360);
    d.layout()->setSizeConstraint(QLayout::SetFixedSize); // make sure dialog can shrank after expanded for more info.

    d.addContent(detailsw);
    d.setOnButtonClickedClose(false);
    d.addButton(tr("Show details","button"));
    d.addButton(tr("Confirm","button"), true, DDialog::ButtonRecommend);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.exec();
}

void DialogManager::showOpticalJobCompletionDialog(const QString &msg, const QString &icon)
{
    DDialog d;
    d.setIcon(QIcon::fromTheme(icon));
    d.setTitle(msg);
    d.addButton(tr("OK","button"), true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    d.getButton(0)->setFocus();
    d.exec();
}

int DialogManager::showDeleteFilesClearTrashDialog(const DFMUrlListBaseEvent &event)
{
    QString ClearTrash = tr("Are you sure you want to empty %1 item?");
    QString ClearTrashMutliple = tr("Are you sure you want to empty %1 items?");
    QString DeleteFileName = tr("Permanently delete %1?");
    QString DeleteFileItems = tr("Permanently delete %1 items?");

    const int maxFileNameWidth = MAX_FILE_NAME_CHAR_COUNT;

    DUrlList urlList = event.urlList();
    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel","button"));
    buttonTexts.append(tr("Delete","button"));

    DDialog d;

    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    QFontMetrics fm(d.font());
    d.setIcon(QIcon::fromTheme("user-trash-full-opened"));
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
    } else {
        d.setTitle(DeleteFileItems.arg(urlList.size()));
    }
    d.setMessage(tr("This action cannot be undone"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.moveToCenter();
    int code = d.exec();
    return code;
}

int DialogManager::showNormalDeleteConfirmDialog(const DFMUrlListBaseEvent &event)
{
    DDialog d;

    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    QFontMetrics fm(d.font());
    d.setIcon(QIcon::fromTheme("user-trash-full-opened"));

    QString deleteFileName = tr("Do you want to delete %1?");
    QString deleteFileItems = tr("Do you want to delete the selected %1 items?");

    DUrlList urlList = event.urlList();

    if (urlList.first().isLocalFile()) { // delete local file
        if (urlList.size() == 1) {
            DFileInfo f(urlList.first());
            d.setTitle(deleteFileName.arg(fm.elidedText(f.fileDisplayName(), Qt::ElideMiddle, MAX_FILE_NAME_CHAR_COUNT)));
        } else {
            d.setTitle(deleteFileItems.arg(urlList.size()));
        }
    } else {
        d.setTitle(deleteFileItems.arg(urlList.size()));
    }

    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel","button"));
    buttonTexts.append(tr("Delete","button"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.moveToCenter();

    return d.exec();
}

int DialogManager::showRemoveBookMarkDialog(const DFMEvent &event)
{
    DDialog d(WindowManager::getWindowById(event.windowId()));
    d.setTitle(tr("Sorry, unable to locate your bookmark directory, remove it?"));
    d.setMessage(" ");
    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel","button"));
    buttonTexts.append(tr("Remove","button"));
    d.addButton(buttonTexts[0], true);
    d.addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    d.setDefaultButton(1);
    d.setIcon(QIcon::fromTheme("folder-bookmark", QIcon::fromTheme("folder")).pixmap(64, 64));
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

void DialogManager::showOpenFilesWithDialog(const DFMEvent &event)
{
//    QWidget *w = WindowManager::getWindowById(event.windowId());
//    if (w) {
//        OpenWithDialog *d = new OpenWithDialog(event.fileUrlList());
//        d->setDisplayPosition(OpenWithDialog::Center);
//        d->exec();
//    }
//    以前的方法，为了能够从命令行打开对话框，弃用
    OpenWithDialog *dialog = new OpenWithDialog(event.fileUrlList());
    dialog->setDisplayPosition(OpenWithDialog::Center);
    dialog->open(); //不建议使用show、exec
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
                if (urlList.count() >= 2) {
                    m_closeIndicatorDialog->show();
                    m_closeIndicatorTimer->start();
                }

                PropertyDialog *dialog;
                if (m_propertyDialogs.contains(url)) {
                    dialog = m_propertyDialogs.value(url);
                    dialog->raise();
                } else {
                    dialog = new PropertyDialog(event, url);
                    dialog->setWindowFlags(dialog->windowFlags() & ~ Qt::FramelessWindowHint);
                    m_propertyDialogs.insert(url, dialog);
                    if (1 == count) {
                        QPoint pos = getPropertyPos(dialog->size().width(), dialog->getDialogHeight());
                        dialog->move(pos);
                    } else {
                        QPoint pos = getPerportyPos(dialog->size().width(), dialog->size().height(), count, index);
                        dialog->move(pos);
                    }
                    dialog->show();

                    connect(dialog, &PropertyDialog::closed, this, &DialogManager::removePropertyDialog);
                    //                connect(dialog, &PropertyDialog::raised, this, &DialogManager::raiseAllPropertyDialog);
                    QTimer::singleShot(100, dialog, &PropertyDialog::raise);
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
    if (m_propertyDialogs.contains(url)) {
        PropertyDialog *dialog = m_propertyDialogs.value(url);
        if (dialog->expandGroup().count() > 1) {
            dialog->expandGroup().at(1)->setExpand(true);
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
        m_trashDialog = nullptr;
    });
    QPoint pos = getPerportyPos(m_trashDialog->size().width(), m_trashDialog->size().height(), 1, 0);
    m_trashDialog->move(pos);
    m_trashDialog->show();

    TIMER_SINGLESHOT(100, {
        if (m_trashDialog)
            m_trashDialog->raise();
    }, this)
}

void DialogManager::showComputerPropertyDialog()
{
    if (m_computerDialog) {
        m_computerDialog->updateComputerInfo();
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
    },
    this)
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
        d.setTitle(tr("Disk is busy, cannot unmount now"));
        d.setMessage(tr("Name: ") + info->fileDisplayName()/* + ", " + tr("Path: ") + info->getPath()*/);
        QStringList buttonTexts;
        buttonTexts.append(tr("Cancel","button"));
        buttonTexts.append(tr("Force unmount","button"));
        d.addButton(buttonTexts[0], true);
        d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
        d.setDefaultButton(0);
        d.setIcon(info->fileIcon(64, 64));

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
    buttonTexts.append(tr("Cancel","button"));
    buttonTexts.append(tr("Confirm","button"));
    d.addButton(buttonTexts[0], true);
    d.addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    d.setDefaultButton(1);
    d.setIcon(fileInfo->fileIcon());
    int code = d.exec();
    if (code == 1) {
        DUrlList urls;
        urls << linkfile;
        // fix bug#47512 回收站的无效链接需要单独处理，直接删除
        if (linkfile.toLocalFile().startsWith(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))) {
            fileService->deleteFiles(this, urls, false, true, true);
        } else {
            fileService->moveToTrash(this, urls);
        }
    }
}

void DialogManager::showConnectToServerDialog(quint64 winId)
{
    QWidget *w = WindowManager::getWindowById(winId);
    if (!w || w->property("ConnectToServerDialogShown").toBool()) {
        return;
    }

    ConnectToServerDialog *dialog = new ConnectToServerDialog(w);
    dialog->show();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &ConnectToServerDialog::finished, dialog, &ConnectToServerDialog::onButtonClicked);
    w->setProperty("ConnectToServerDialogShown", true);
    connect(dialog, &ConnectToServerDialog::closed, [ = ] {
        w->setProperty("ConnectToServerDialogShown", false);
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
    d.setIcon(m_dialogWarningIcon);
    d.setTitle(tr("Unable to copy. Not enough free space on the target disk."));
    d.addButton(tr("OK","button"));

    QRect rect = d.geometry();
    rect.moveCenter(qApp->desktop()->geometry().center());
    d.move(rect.x(), rect.y());
    d.exec();
}

void DialogManager::show4gFat32Dialog()
{
    DDialog d;
    d.setTitle(tr("Failed, file size must be less than 4GB."));
    d.setIcon(m_dialogWarningIcon);
    d.addButton(tr("OK","button"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showFailToCreateSymlinkDialog(const QString &errorString)
{
    DDialog d;
    d.setTitle(tr("Fail to create symlink, cause:") + errorString);
    d.setIcon(m_dialogWarningIcon);
    d.addButton(tr("OK","button"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showMoveToTrashConflictDialog(const DUrlList &urls)
{
    MoveToTrashConflictDialog d(nullptr, urls);
    int code = d.exec();
    if (code == 1) {
        fileService->deleteFiles(this, urls, false);
    }
}

void DialogManager::showDeleteSystemPathWarnDialog(quint64 winId)
{
    DDialog d(WindowManager::getWindowById(winId));
    d.setTitle(tr("The selected files contain system file/directory, and it cannot be deleted"));
    d.setIcon(m_dialogWarningIcon);
    d.addButton(tr("OK","button"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showFilePreviewDialog(const DUrlList &selectUrls, const DUrlList &entryUrls)
{
    DUrlList canPreivewlist;

    //记录是否有无效的链接文件
    bool hasInvalidSymlink = false;
    for (const DUrl &url : selectUrls) {
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

        if (info && (info->fileUrl().isLocalFile() || info->toQFileInfo().exists())) {
            //判断链接文件的源文件是否存在
            if (info->isSymLink()) {
                DUrl targetUrl = info->symLinkTarget();
                if (!targetUrl.isValid()) {
                    hasInvalidSymlink = true;
                    continue;
                }

                const DAbstractFileInfoPointer &linkInfo = DFileService::instance()->createFileInfo(this, targetUrl);
                if (!linkInfo || !linkInfo->exists()) {
                    hasInvalidSymlink = true;
                    continue;
                }
            }
            canPreivewlist << info->fileUrl();
        }
    }

    //链接文件源文件不存在或找不到的情况，弹错误提示窗
    if (hasInvalidSymlink) {
        dialogManager->showErrorDialog(tr("Unable to find the original file"), QString());
    }

    if (canPreivewlist.isEmpty()) {
        return;
    }

    if (DFMGlobal::isWayLand()) {
        //! 判断当前预览是否于新的预览列表相同
        if (m_filePreviewDialog && DUrlListCompare(canPreivewlist)) {
            m_filePreviewDialog->show();
            m_filePreviewDialog->raise();
            m_filePreviewDialog->activateWindow();
            return;
        }

        if (m_filePreviewDialog) {
            m_filePreviewDialog->close();
            m_filePreviewDialog = nullptr;
        }
        m_filePreviewDialog = new FilePreviewDialog(canPreivewlist, nullptr);

        //! 记录预览列表
        for (const DUrl &url : canPreivewlist) {
            m_urlList << url;
        }
        //! 对话框关闭时回收FilePreviewDialog对象
        m_filePreviewDialog->setAttribute(Qt::WA_DeleteOnClose);
        //! 对话框关闭时m_filePreviewDialog对象置空
        connect(m_filePreviewDialog, &FilePreviewDialog::signalCloseEvent, this, [ = ]() {
            m_filePreviewDialog->DoneCurrent();
            m_filePreviewDialog = nullptr;
            m_urlList.clear();
        });
    } else {
        if (!m_filePreviewDialog) {
            m_filePreviewDialog = new FilePreviewDialog(canPreivewlist, nullptr);
            if (!DFMGlobal::isWayLand())
                DPlatformWindowHandle::enableDXcbForWindow(m_filePreviewDialog, true);
        } else {
            m_filePreviewDialog->updatePreviewList(canPreivewlist);
        }
    }

    if (canPreivewlist.count() == 1) {
        m_filePreviewDialog->setEntryUrlList(entryUrls);
    }

    m_filePreviewDialog->show();
    m_filePreviewDialog->raise();
    m_filePreviewDialog->activateWindow();
}

void DialogManager::showRestoreFailedDialog(const DUrlList &urlList)
{
    DDialog d;
    d.setTitle(tr("Operation failed!"));
    if (urlList.count() == 1) {
        d.setMessage(tr("Failed to restore %1 file, the target folder is read-only").arg(QString::number(1)));
    } else if (urlList.count() > 1) {
        d.setMessage(tr("Failed to restore %1 files, the target folder is read-only").arg(QString::number(urlList.count())));
    }
    d.setIcon(m_dialogWarningIcon);
    d.addButton(tr("OK","button"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showRestoreFailedPerssionDialog(const QString &srcPath, const QString &targetPath)
{
    qDebug() << srcPath << "restore to" << targetPath;
    DDialog d;
    d.setTitle(tr("Operation failed!"));
    d.setMessage(tr("You do not have permission to operate file/folder!"));
    d.setIcon(m_dialogWarningIcon);
    d.addButton(tr("OK","button"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showRestoreFailedSourceNotExists(const DUrlList &urlList)
{
    DDialog d;
    d.setTitle(tr("Operation failed!"));
    if (urlList.count() == 1) {
        d.setMessage(tr("Failed to restore %1 file, the source file does not exist").arg(QString::number(1)));
    } else if (urlList.count() > 1) {
        d.setMessage(tr("Failed to restore %1 files, the source files do not exist").arg(QString::number(urlList.count())));
    }
    d.setIcon(m_dialogWarningIcon);
    d.addButton(tr("OK","button"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showNoPermissionDialog(const DFMUrlListBaseEvent &event)
{
    DUrlList urls = event.urlList();
    qDebug() << urls << "no perssion";
    if (urls.isEmpty()) {
        return;
    }

    QFont f;
    f.setPixelSize(16);
    QFontMetrics fm(f);

    DDialog d;

    if (urls.count() == 1) {

        d.setTitle(tr("You do not have permission to operate file/folder!"));
        QString message = urls.at(0).toLocalFile();

        if (fm.width(message) > MAX_FILE_NAME_CHAR_COUNT) {
            message = fm.elidedText(message, Qt::ElideMiddle, MAX_FILE_NAME_CHAR_COUNT);
        }

        d.setMessage(message);
        d.setIcon(m_dialogWarningIcon);
    } else {

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
            if (fm.width(s) > MAX_FILE_NAME_CHAR_COUNT) {
                s = fm.elidedText(s, Qt::ElideMiddle, MAX_FILE_NAME_CHAR_COUNT);
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
    }

    d.addButton(tr("OK","button"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showErrorDialog(const QString &title, const QString &message)
{
    DDialog d(title, message);
    Qt::WindowFlags flags = d.windowFlags();
    // dialog show top
    d.setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    d.setIcon(QIcon::fromTheme("dialog-error"));
    d.addButton(tr("Confirm","button"), true, DDialog::ButtonRecommend);
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
        if (!DesktopInfo().waylandDectected()) {
            QtX11::utils::ShowNormalWindow(d);
        }
        qobject_cast<QWidget *>(d)->showNormal();
        d->show();
        d->raise();
        qDebug() << d->getUrl() << d->isVisible() << d->windowState();
    }
    if (m_closeIndicatorDialog)
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

void DialogManager::showTaskProgressDlgOnActive()
{
    if (!m_taskDialog)
        return;

    m_taskDialog->show();
    m_taskDialog->raise();
    m_taskDialog->activateWindow();

    QMapIterator<QString, QSharedPointer<FileJob> > iter(m_jobs);
    while (iter.hasNext()) {
        iter.next();
        if (iter.value()->getIsFinished())
            continue;
        if (iter.value()->getIsOpticalJob()) // 目前业务只针对光盘业务
            emit iter.value()->requestJobAdded(iter.value()->jobDetail());
    }
}

int DialogManager::showUnableToLocateDir(const QString &dir)
{
    // Ensure that only one dialog is displayed in the current screen
    static bool showFlag = true;
    int code = -1;
    if (showFlag) {
        showFlag = false;
        DDialog d;
        d.setTitle(tr("Unable to access %1").arg(dir));
        d.setMessage(" ");
        QStringList buttonTexts;
        buttonTexts.append(tr("Confirm","button"));
        d.addButton(buttonTexts[0], true);
        d.setDefaultButton(0);
        d.setIcon(QIcon::fromTheme("folder").pixmap(64, 64));
        code = d.exec();
        showFlag = true;
    }

    return code;
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
    QSharedPointer<FileJob> job = m_jobs.value(jobId);
    if (job != nullptr) {
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

int DialogManager::showMessageDialog(messageType messageLevel, const QString &title, const QString &message, QString btnTxt)
{
    DDialog d(title, message);
    d.moveToCenter();
    QStringList buttonTexts;
    buttonTexts.append(btnTxt);
    d.addButtons(buttonTexts);
    d.setDefaultButton(0);
    if (messageLevel == msgInfo) {
        d.setIcon(m_dialogInfoIcon);
    } else if (messageLevel == msgWarn) {
        d.setIcon(m_dialogWarningIcon);
    } else if (messageLevel == msgErr) {
        d.setIcon(m_dialogErrorIcon);
    } else {
        d.setIcon(m_dialogInfoIcon);
    }
    int code = d.exec();
    qDebug() << code;
    return code;
}

void DialogManager::showBluetoothTransferDlg(const DUrlList &files)
{
    if (!BluetoothTransDialog::canSendFiles()) {
        showMessageDialog(messageType::msgInfo, tr("Sending files now, please try later"));
        return;
    }

    QStringList paths;
    foreach (auto u, files) {
        if (u.scheme() == RECENT_SCHEME) {
            u = DUrl::fromLocalFile(u.path());
        } else if (u.scheme() == SEARCH_SCHEME) { //搜索结果也存在右键批量打开不成功的问题，这里做类似处理
            u = u.searchedFileUrl();
        } else if (u.scheme() == BURN_SCHEME) {
            DAbstractFileInfoPointer info = fileService->createFileInfo(nullptr, u);
            if (!info)
                continue;
            if (info->canRedirectionFileUrl())
                u = info->redirectedFileUrl();
        } else if (u.scheme() == TAG_SCHEME) {
            u = DUrl::fromLocalFile(u.toLocalFile());
        }
        paths << u.path();
    }

    BluetoothTransDialog *dlg = new BluetoothTransDialog(paths);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void DialogManager::showFormatDialog(const QString &devId)
{
    if (!devId.startsWith("/dev/"))
        return;

    // fix 50005, 弹出格式化提示框仅针对可移动磁盘，且无法读取文件系统的设备，避免smb、mtp等设备也弹出提示框
    QString volTag = devId.mid(5);
    static const QString udisksPrefix = "/org/freedesktop/UDisks2/block_devices/";
    QScopedPointer<DBlockDevice> dev(DDiskManager::createBlockDevice(udisksPrefix + volTag));
    if (!dev || dev->hasFileSystem())
        return;
    QScopedPointer<DDiskDevice> drive(DDiskManager::createDiskDevice(dev->drive()));
    if (!drive)
        return;
    if (drive->optical() || !drive->removable()) // 光驱不管，非移动存储设备不管
        return;

    qDebug() << "device formatter has shown: " << devId;
    DDialog dlg;
    dlg.setIcon(m_dialogWarningIcon);
    dlg.addButton(tr("Cancel","button"));
    dlg.addButton(tr("Format","button"), true, DDialog::ButtonRecommend);
    dlg.setTitle(tr("To access the device, you must format the disk first. Are you sure you want to format it now?"));
    int code = dlg.exec();
    if (code == 1) {
        qDebug() << "start format " << devId;
        // 显示格式化窗口
        QProcess::startDetached("dde-device-formatter", QStringList {devId});
    }
}

int DialogManager::showStopScanningDialog()
{
    DDialog dlg;
    dlg.setIcon(m_dialogWarningIcon);
    dlg.addButton(tr("Cancel","button"));
    dlg.addButton(tr("Stop","button"), true, DDialog::ButtonWarning); // 终止
    dlg.setTitle(tr("Scanning the device, stop it?")); // 正在扫描当前设备，是否终止扫描？
    return dlg.exec();
}

bool DialogManager::DUrlListCompare(DUrlList urls)
{
    if (urls.size() != m_urlList.size()) {
        return false;
    } else {
        for (int i = 0; i < urls.size(); ++i) {
            if (urls[i] != m_urlList[i]) {
                return false;
            }
        }
        return true;
    }
}


void DialogManager::showMultiFilesRenameDialog(const QList<DUrl> &selectedUrls)
{
    DDesktopRenameDialog renameDialog;

    renameDialog.moveToCenter();
    renameDialog.setDialogTitle(QObject::tr("Rename %1 Files").arg(QString::fromStdString(std::to_string(selectedUrls.size()))));

    std::size_t code { static_cast<size_t>(renameDialog.exec()) };

    AppController::flagForDDesktopRenameBar.store(true, std::memory_order_seq_cst);

    if (static_cast<bool>(code) == true) { //###: yes!
        std::size_t modeIndex{ renameDialog.getCurrentModeIndex() };
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
    d.setIcon(m_dialogWarningIcon);
    d.addButton(tr("OK","button"), true, DDialog::ButtonRecommend);
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
            int code = showMessageDialog(messageType(nerrorlevel), message);
            return code;

        } else {
            qDebug() << "get error message fail" << nRet;
            return -1;
        }
    }
}

#endif

