#include "dialogmanager.h"
#include "closealldialogindicator.h"
#include "openwithotherdialog.h"
#include "trashpropertydialog.h"
#include "computerpropertydialog.h"
#include "usersharepasswordsettingdialog.h"
#include "dialogs/movetotrashconflictdialog.h"
#include "views/dfilemanagerwindow.h"
#include "interfaces/dstyleditemdelegate.h"
#include "shutil/mimetypedisplaymanager.h"
#include "previewdialog.h"

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

#include "dialogs/dtaskdialog.h"
#include "dialogs/propertydialog.h"
#include "dialogs/openwithdialog.h"
#include "dialogs/diskspaceoutofusedtipdialog.h"
#include "interfaces/dfmsetting.h"
#include "plugins/pluginmanager.h"
#include "preview/previewinterface.h"

#include "deviceinfo/udisklistener.h"
#include "deviceinfo/udiskdeviceinfo.h"

#include "singleton.h"

#ifdef SW_LABEL
#include "sw_label/llsdeepinlabellibrary.h"
#endif

#include <ddialog.h>
#include <DAboutDialog>
#include <dexpandgroup.h>
#include <settings.h>
#include <dsettingsdialog.h>

#include <QTimer>
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>
#include <qsettingbackend.h>

DTK_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DialogManager::DialogManager(QObject *parent) : QObject(parent)
{
    initTaskDialog();
    initCloseIndicatorDialog();
    initConnect();
}

DialogManager::~DialogManager()
{

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
    m_closeIndicatorTimer->setInterval(100);
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
    connect(fileSignalManager, &FileSignalManager::show4GFat32Dialog,
            this, &DialogManager::show4gFat32Dialog);

    connect(fileSignalManager, &FileSignalManager::requestShowFilePreviewDialog, this, &DialogManager::showFilePreviewDialog);

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

    if (!cursor_screen)
        cursor_screen = qApp->primaryScreen();

    int desktopWidth = cursor_screen->size().width();
    int desktopHeight = cursor_screen->size().height();
    int SpaceWidth = 20;
    int SpaceHeight = 100;
    int row, x , y;

    int numberPerRow =  desktopWidth / (dialogWidth + SpaceWidth);


    if (count % numberPerRow == 0){
        row = count / numberPerRow;

    }else{
        row = count / numberPerRow + 1;

    }

    int dialogsWidth;
    if (count / numberPerRow > 0){
        dialogsWidth = dialogWidth * numberPerRow + SpaceWidth * (numberPerRow - 1);
    }else{
        dialogsWidth = dialogWidth * (count % numberPerRow)  + SpaceWidth * (count % numberPerRow - 1);
    }

    int dialogsHeight = dialogHeight + SpaceHeight * (row - 1);

    x =  (desktopWidth - dialogsWidth) / 2 + (dialogWidth + SpaceWidth) * (index % numberPerRow);

    y = (desktopHeight - dialogsHeight) / 2 + (index / numberPerRow) * SpaceHeight;

    return QPoint(x, y) + cursor_screen->geometry().topLeft();
}

bool DialogManager::isTaskDialogEmpty()
{
    if (m_taskDialog->getTaskListWidget()->count() == 0)
        return true;
    return false;
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
    connect(job, &FileJob::requestNoEnoughSpaceDialogShowed, this, &DialogManager::showDiskSpaceOutOfUsedDialog);
    connect(job, &FileJob::requestCanNotMoveToTrashDialogShowed, this, &DialogManager::showMoveToTrashConflictDialog);
}


void DialogManager::removeJob(const QString &jobId)
{
    if (m_jobs.contains(jobId)){
        FileJob* job = m_jobs.value(jobId);
        job->setIsAborted(true);
        job->setApplyToAll(true);
        job->cancelled();
        m_jobs.remove(jobId);
    }
    if (m_jobs.count() == 0){
        emit fileSignalManager->requestStopUpdateJobTimer();
    }
}

void DialogManager::removeAllJobs()
{
    foreach (const QString& jobId, m_jobs.keys()) {
        removeJob(jobId);
    }
}

void DialogManager::updateJob()
{
    foreach (QString jobId, m_jobs.keys()) {
        FileJob* job = m_jobs.value(jobId);
        if (job){
            if (job->currentMsec() - job->lastMsec() > FileJob::Msec_For_Display){
                if (!job->isJobAdded()){
                    job->jobAdded();
                    job->jobUpdated();
                }else{
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
        FileJob* job = m_jobs.value(jobId);
        qDebug() << jobId << job->getTargetDir();
        if (!QFile(job->getTargetDir()).exists()){
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
    d.setIcon(QIcon(":/images/dialogs/images/dialog_warning_64.png"));
    QTimer::singleShot(200, &d, &DDialog::raise);
    int code = d.exec();
    qDebug() << code;
    if (code == 0){
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
    QString fileDisplayName = d.fontMetrics().elidedText(_fileDisplayName,Qt::ElideRight, maxDisplayNameLength);
    QString message = tr("Do you want to run %1 or display its content?").arg(fileDisplayName);
    QString tipMessage = tr("It is an executable text file.");
    QStringList buttonKeys, buttonTexts;
    buttonKeys << "OptionCancel" << "OptionRun" << "OptionRunInTerminal" << "OptionDisplay";
    buttonTexts << tr("Cancel") << tr("Run") << tr("Run in terminal") << tr("Display");
#ifdef ARCH_MIPSEL
    d.setIcon(QIcon(svgToPixmap(":/images/images/application-x-shellscript.svg", 64, 64)));
#else
    d.setIconPixmap(QIcon::fromTheme("application-x-shellscript").pixmap(64, 64));
#endif
    d.setTitle(message);
    d.setMessage(tipMessage);
//    d.addButtons(buttonTexts);
    d.addButton(buttonTexts[0], true);
    d.addButton(buttonTexts[1], false);
    d.addButton(buttonTexts[2], false);
    d.addButton(buttonTexts[3], false, DDialog::ButtonRecommend);
    d.setDefaultButton(2);
    d.setFixedWidth(480);
    int code = d.exec();
    return code;
}

int DialogManager::showRunExcutableFileDialog(const DUrl &url, quint64 winId)
{
    DDialog d(WindowManager::getWindowById(winId));
    const DAbstractFileInfoPointer& pfileInfo = fileService->createFileInfo(this, url);
    int maxDisplayNameLength = 200;
    QString _fileDisplayName = QFileInfo(url.path()).fileName();
    QString fileDisplayName = d.fontMetrics().elidedText(_fileDisplayName, Qt::ElideRight, maxDisplayNameLength);
    QString message = tr("Do you sure to run %1?").arg(fileDisplayName);
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


int DialogManager::showRenameNameSameErrorDialog(const QString &name, const DFMEvent &event)
{
    DDialog d(WindowManager::getWindowById(event.windowId()));
    QFontMetrics fm(d.font());
    d.setTitle(tr("\"%1\" already exists, please use another name.").arg(fm.elidedText(name, Qt::ElideMiddle, 150)));
    QStringList buttonTexts;
    buttonTexts << tr("Confirm");
    d.addButton(buttonTexts[0], true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    d.setIcon(QIcon(":/images/dialogs/images/dialog_warning_64.png"));
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

    DDialog d(WindowManager::getWindowById(event.windowId()));

    if (!d.parentWidget())
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);

    QFontMetrics fm(d.font());
    d.setIcon(QIcon(":/images/dialogs/images/user-trash-full-opened.png"));
    if (urlList.first() == DUrl::fromTrashFile("/")){
        buttonTexts[1]= tr("Empty");
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, urlList.first());
        if(fileInfo->filesCount() == 1)
            d.setTitle(ClearTrash.arg(fileInfo->filesCount()));
        else
            d.setTitle(ClearTrashMutliple.arg(fileInfo->filesCount()));
    }else if (urlList.first().isLocalFile()){
        if (urlList.size() == 1) {
            DFileInfo f(urlList.first());
            d.setTitle(DeleteFileName.arg(fm.elidedText(f.fileDisplayName(),Qt::ElideMiddle, maxFileNameWidth)));
        } else {
            d.setTitle(DeleteFileItems.arg(urlList.size()));
        }
    }else if (urlList.first().isTrashFile()){
        if (urlList.size() == 1) {
            TrashFileInfo f(urlList.first());
            d.setTitle(DeleteFileName.arg(fm.elidedText(f.fileDisplayName(),Qt::ElideMiddle, maxFileNameWidth)));
        } else {
            d.setTitle(DeleteFileItems.arg(urlList.size()));
        }
    }else if (urlList.first().isTrashFile()){
        if (urlList.size() == 1) {
            TrashFileInfo f(urlList.first());
            d.setTitle(DeleteFileName.arg(fm.elidedText(f.fileDisplayName(),Qt::ElideMiddle, maxFileNameWidth)));
        } else {
            d.setTitle(DeleteFileItems.arg(urlList.size()));
        }
    }else{
        d.setTitle(DeleteFileItems.arg(urlList.size()));
    }
    d.setMessage(tr("This action cannot be restored"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    int code = d.exec();
    return code;
}

int DialogManager::showRemoveBookMarkDialog(const DFMEvent &event)
{
    DDialog d(WindowManager::getWindowById(event.windowId()));
    d.setTitle(tr("Sorry, unable to locate your bookmark directory, remove it?"));
    QStringList buttonTexts;
    buttonTexts << tr("Cancel") << tr("Remove");
    d.addButton(buttonTexts[0], true);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    d.setIconPixmap(QIcon::fromTheme("folder").pixmap(64, 64));
    int code = d.exec();
    return code;
}

void DialogManager::showOpenWithDialog(const DFMEvent &event)
{
    QWidget* w = WindowManager::getWindowById(event.windowId());
    if (w){
        OpenWithDialog* d = new OpenWithDialog(event.fileUrl(), w);
        d->setDisplayPostion(OpenWithDialog::DisplayCenter);
        d->exec();
    }
}

void DialogManager::showPropertyDialog(const DFMUrlListBaseEvent &event)
{
    const DUrlList& urlList  = event.urlList();
    int count = urlList.count();
    foreach (const DUrl& url, urlList) {
        int index = urlList.indexOf(url);

        if (url.isComputerFile()){
            showComputerPropertyDialog();
        }else{

            PropertyDialog *dialog;
            if (m_propertyDialogs.contains(url)){
                dialog = m_propertyDialogs.value(url);
                dialog->raise();
            }else{
                dialog = new PropertyDialog(event, url);
                m_propertyDialogs.insert(url, dialog);
                QPoint pos = getPerportyPos(dialog->size().width(), dialog->size().height(), count, index);

                dialog->show();
                dialog->move(pos);

                connect(dialog, &PropertyDialog::closed, this, &DialogManager::removePropertyDialog);
    //                connect(dialog, &PropertyDialog::raised, this, &DialogManager::raiseAllPropertyDialog);
                QTimer::singleShot(100, dialog, &PropertyDialog::raise);
            }

            if (urlList.count() >= 2){
                m_closeIndicatorDialog->show();
                m_closeIndicatorTimer->start();
            }
        }
    }
}

void DialogManager::showShareOptionsInPropertyDialog(const DFMUrlListBaseEvent &event)
{
    DUrl url = event.fileUrlList().first();
    showPropertyDialog(event);
    PropertyDialog *dialog;
    if (m_propertyDialogs.contains(url)){
        dialog = m_propertyDialogs.value(url);
        if (dialog->expandGroup()->expands().count() > 1){
            dialog->expandGroup()->expand(1)->setExpand(true);
        }
    }
}

void DialogManager::showTrashPropertyDialog(const DFMEvent &event)
{
    if (m_trashDialog){
        m_trashDialog->close();
    }
    m_trashDialog = new TrashPropertyDialog(event.fileUrl());
    connect(m_trashDialog, &TrashPropertyDialog::finished, [=](){
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
    if (m_computerDialog){
        m_computerDialog->close();
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
    QWidget* w = WindowManager::getWindowById(event.windowId());
    if (w){
        PropertyDialog* dialog = new PropertyDialog(event, event.fileUrl());
        dialog->show();
    }
}

void DialogManager::showDiskErrorDialog(const QString & id, const QString & errorText)
{
    Q_UNUSED(errorText)

    static QSet<QString> dialogHadShowForId;

    if (dialogHadShowForId.contains(id))
        return;

    UDiskDeviceInfoPointer info = deviceListener->getDevice(id);

    if (info){
        DDialog d;
        d.setTitle(tr("Disk file is being used, can not unmount now"));
        d.setMessage(tr("Name: ") + info->fileDisplayName()/* + ", " + tr("Path: ") + info->getPath()*/);
        QStringList buttonTexts;
        buttonTexts << tr("Cancel") << tr("Force unmount");
        d.addButton(buttonTexts[0], true);
        d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
        d.setDefaultButton(0);
        d.setIcon(info->fileIcon(64, 64));

        dialogHadShowForId << id;

        int code = d.exec();

        dialogHadShowForId.remove(id);

        if (code == 1){
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
    d.setIcon(fileInfo->fileIcon().pixmap(64, 64));
    int code = d.exec();
    if (code == 1){
        DUrlList urls;
        urls << linkfile;
        fileService->moveToTrash(this, urls);
    }
}

void DialogManager::showAboutDialog(quint64 winId)
{
    QWidget* w = WindowManager::getWindowById(winId);
    if(!w || w->property("AboutDialogShown").toBool())
        return;

    QString icon(":/images/images/dde-file-manager_96.png");
    DAboutDialog *dialog = new DAboutDialog(w);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("");
    dialog->setProductIcon(QIcon(icon));
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
    connect(dialog, &DAboutDialog::closed, [=]{
        w->setProperty("AboutDialogShown", false);
    });
}

void DialogManager::showUserSharePasswordSettingDialog(quint64 winId)
{
    QWidget* w = WindowManager::getWindowById(winId);
    if(!w || w->property("UserSharePwdSettingDialogShown").toBool())
        return;

    UserSharePasswordSettingDialog* dialog = new UserSharePasswordSettingDialog(w);
    dialog->show();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &UserSharePasswordSettingDialog::finished, dialog, &UserSharePasswordSettingDialog::onButtonClicked);
    w->setProperty("UserSharePwdSettingDialogShown", true);
    connect(dialog, &UserSharePasswordSettingDialog::closed, [=]{
        w->setProperty("UserSharePwdSettingDialogShown", false);
    });
}

void DialogManager::showGlobalSettingsDialog(quint64 winId)
{
    QWidget* w = WindowManager::getWindowById(winId);
    if(!w)
        return;
    if(w->property("isSettingDialogShown").toBool())
        return;

    w->setProperty("isSettingDialogShown", true);

    DSettingsDialog* dsd = new DSettingsDialog(w);
    dsd->updateSettings(globalSetting->settings());
    dsd->show();

    connect(dsd, &DSettingsDialog::finished, [=]{
        w->setProperty("isSettingDialogShown", false);
    });
}

void DialogManager::showDiskSpaceOutOfUsedDialog()
{
    QTimer::singleShot(200,[=]{
        DiskSpaceOutOfUsedTipDialog d;
        QRect rect = d.geometry();
        rect.moveCenter(qApp->desktop()->geometry().center());
        d.move(rect.x(), rect.y());
        d.exec();
    });
}

void DialogManager::show4gFat32Dialog()
{
    DDialog d;
    d.setTitle(tr("Failed, file size must be less than 4GB."));
    d.setIcon(QIcon(":/images/dialogs/images/dialog_warning_64.png"));
    d.addButton(tr("OK"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showFailToCreateSymlinkDialog(const QString& errorString)
{
    DDialog d;
    d.setTitle(tr("Fail to create symlink, cause:") + errorString);
    d.setIcon(QIcon(":/images/dialogs/images/dialog_warning_64.png"));
    d.addButton(tr("OK"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showMoveToTrashConflictDialog(const DUrlList &urls)
{
    MoveToTrashConflictDialog d(0, urls);
    int code = d.exec();
    if (code == 1) {
        fileService->deleteFiles(this, urls);
    }
}

void DialogManager::showDeleteSystemPathWarnDialog(quint64 winId)
{
    DDialog d(WindowManager::getWindowById(winId));
    d.setTitle(tr("The selected files contain system file/directory, and it cannot be deleted"));
    d.setIcon(QIcon(":/images/dialogs/images/dialog_warning_64.png"));
    d.addButton(tr("OK"), true, DDialog::ButtonRecommend);
    d.exec();
}

void DialogManager::showFilePreviewDialog(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    //Filter out all folders
    DUrlList urlList;
    foreach (const DUrl& url, event->fileUrlList()) {
        const DAbstractFileInfoPointer& info = fileService->createFileInfo(this, url);
        if(info->isFile())
            urlList << url;
    }

    if(urlList.count() <= 0)
        return;

    QWidget* w = WindowManager::getWindowById(event->windowId());
    if(!w || w->property("AboutDialogShown").toBool())
        return;

    PreviewDialog* d = new PreviewDialog(urlList);

    w->setProperty("UserSharePwdSettingDialogShown", true);
    connect(d, &PreviewDialog::finished, this, [=] {
        w->setProperty("UserSharePwdSettingDialogShown", false);
    });

    d->show();
}

void DialogManager::removePropertyDialog(const DUrl &url)
{
    if (m_propertyDialogs.contains(url)){
        m_propertyDialogs.remove(url);
    }
    if (m_propertyDialogs.count() == 0){
        m_closeIndicatorDialog->hide();
    }
}

void DialogManager::closeAllPropertyDialog()
{
    foreach (const DUrl& url, m_propertyDialogs.keys()) {
        m_propertyDialogs.value(url)->close();
    }
    if (m_closeIndicatorDialog){
        m_closeIndicatorTimer->stop();
        m_closeIndicatorDialog->close();
    }
    if (m_trashDialog){
        m_trashDialog->close();
    }
}

void DialogManager::updateCloseIndicator()
{
    qint64 size = 0;
    int fileCount = 0;
    foreach (PropertyDialog* d, m_propertyDialogs.values()) {
        size += d->getFileSize();
        fileCount += d->getFileCount();
    }
    m_closeIndicatorDialog->setTotalMessage(size, fileCount);
}

void DialogManager::raiseAllPropertyDialog()
{
    foreach (PropertyDialog* d, m_propertyDialogs.values()) {
        qDebug() << d->getUrl() << d->isVisible() << d->windowState();
//        d->showMinimized();
        d->showNormal();
        QtX11::utils::ShowNormalWindow(d);
        qobject_cast<QWidget*>(d)->showNormal();
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

    if (m_propertyDialogs.values().contains(qobject_cast<PropertyDialog*>(qApp->activeWindow()))){
        raiseAllPropertyDialog();
    }else if(m_closeIndicatorDialog == qobject_cast<CloseAllDialogIndicator*>(qApp->activeWindow())){
        raiseAllPropertyDialog();
    }
}

void DialogManager::refreshPropertyDialogs(const DUrl &oldUrl, const DUrl &newUrl)
{
    PropertyDialog* d = m_propertyDialogs.value(oldUrl);
    if (d){
        m_propertyDialogs.remove(oldUrl);
        m_propertyDialogs.insert(newUrl, d);
    }
}


void DialogManager::handleConflictRepsonseConfirmed(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response)
{
    QString jobId = jobDetail.value("jobId");
    FileJob * job = m_jobs.value(jobId);
    if(job != NULL)
    {
        bool applyToAll = response.value("applyToAll").toBool();
        int code = response.value("code").toInt();
        job->setApplyToAll(applyToAll);
        //0 = coexist, 1 = replace, 2 = skip
        switch(code)
        {
        case 0:
            job->setIsCoExisted(true);
            job->started();
            break;
        case 1:
            job->setReplace(true);
            job->started();
            break;
        case 2:
            job->setIsSkip(true);
            job->cancelled();
            break;
        default:
            qDebug() << "unknown code"<<code;
        }
    }
}

int DialogManager::showMessageDialog(int messageLevel, const QString& message)
{
    DDialog d;
    d.moveToCenter();
    d.setTitle(message);
    QStringList buttonTexts;
    buttonTexts << tr("Confirm");
    d.addButtons(buttonTexts);
    d.setDefaultButton(0);
    if (messageLevel == 1){
        d.setIcon(QIcon(":/images/dialogs/images/dialog_info_64.png"));
    }else if (messageLevel == 2){
        d.setIcon(QIcon(":/images/dialogs/images/dialog_warning_64.png"));
    }else if (messageLevel == 3){
        d.setIcon(QIcon(":/images/dialogs/images/dialog_error_64.png"));
    }else{
        d.setIcon(QIcon(":/images/dialogs/images/dialog_info_64.png"));
    }
    int code = d.exec();
    qDebug() << code;
    return code;
}

#ifdef SW_LABEL

void DialogManager::onJobFailed_SW( int nRet, const QString &jobType, const QString &srcfilename)
{
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        int ret = showPrivilegeDialog_SW(nRet, srcfilename);
        if (ret == 0){
            qDebug() << jobType << nRet << srcfilename;
        }
    }
}

int DialogManager::showPrivilegeDialog_SW(int nRet, const QString &srcfilename)
{
    if (LlsDeepinLabelLibrary::instance()->isCompletion()){
        qDebug() << "文件全路径名称" << srcfilename;
        qDebug() << "错误码；"<< nRet;

        std::string serrordst=""; //错误描述
        int nerrorlevel=0;  //错误级别
//        nRet =  lls_geterrordesc(nRet, serrordst, nerrorlevel);
        nRet =  LlsDeepinLabelLibrary::instance()->lls_geterrordesc()(nRet, serrordst, nerrorlevel);
        if (nRet == 0){
            qDebug() << "错误描述:" << QString::fromStdString(serrordst);
            qDebug() << "错误级别:" << nerrorlevel;
            QString message = QString("%1 %2").arg(QFileInfo(srcfilename).fileName(), QString::fromStdString(serrordst));
            int code = showMessageDialog(nerrorlevel, message);
            return code;

        }else{
            qDebug() << "get error message fail" << nRet;
            return -1;
        }
    }
}

#endif

