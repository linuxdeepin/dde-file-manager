#include "dialogmanager.h"
#include <QTimer>
#include "dialogs/dtaskdialog.h"
#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../controllers/filejob.h"
#include "dialogs/messagewrongdialog.h"
#include "../dialogs/propertydialog.h"
#include "../dialogs/openwithdialog.h"
#include "../models/fileinfo.h"
#include "../app/fmevent.h"
#include "../views/windowmanager.h"
#include "../models/trashfileinfo.h"
#include <ddialog.h>
#include <DAboutDialog>
#include <dscrollbar.h>

DWIDGET_USE_NAMESPACE

DialogManager::DialogManager(QObject *parent) : QObject(parent)
{
    initTaskDialog();
    initConnect();
}

DialogManager::~DialogManager()
{

}

void DialogManager::initTaskDialog()
{
    m_taskDialog = new DTaskDialog;
    m_taskDialog->getTaskListWidget()->setVerticalScrollBar(new DScrollBar);
    m_taskDialog->setStyleSheet(getQssFromFile(":/qss/dialogs/qss/light.qss"));
}

void DialogManager::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::jobAdded, m_taskDialog, &DTaskDialog::addTask);
    connect(fileSignalManager, &FileSignalManager::jobRemoved, m_taskDialog, &DTaskDialog::removeTask);
    connect(fileSignalManager, &FileSignalManager::jobDataUpdated, m_taskDialog, &DTaskDialog::handleUpdateTaskWidget);
    connect(m_taskDialog, &DTaskDialog::abortTask, fileSignalManager, &FileSignalManager::abortTask);

    connect(fileSignalManager, &FileSignalManager::conflictDialogShowed, m_taskDialog, &DTaskDialog::showConflictDiloagByJob);
    connect(m_taskDialog, &DTaskDialog::conflictShowed, fileSignalManager, &FileSignalManager::conflictTimerStoped);
    connect(m_taskDialog, &DTaskDialog::conflictHided, fileSignalManager, &FileSignalManager::conflictTimerReStarted);
    connect(m_taskDialog, &DTaskDialog::conflictRepsonseConfirmed, fileSignalManager, &FileSignalManager::conflictRepsonseConfirmed);
    connect(m_taskDialog, &DTaskDialog::conflictRepsonseConfirmed, this, &DialogManager::handleConflictRepsonseConfirmed);

    connect(m_taskDialog, &DTaskDialog::abortTask, this, &DialogManager::abortJob);

    connect(fileSignalManager, &FileSignalManager::requestShowUrlWrongDialog, this, &DialogManager::showUrlWrongDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowOpenWithDialog, this, &DialogManager::showOpenWithDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowPropertyDialog, this, &DialogManager::showPropertyDialog);
    connect(fileSignalManager, &FileSignalManager::showDiskErrorDialog,
            this, &DialogManager::showDiskErrorDialog);
    connect(fileSignalManager, &FileSignalManager::showAboutDialog,
            this, &DialogManager::showAboutDialog);
}

void DialogManager::addJob(FileJob *job)
{
    m_jobs.insert(job->getJobId(), job);
}


void DialogManager::removeJob(const QString &jobId)
{
    m_jobs.remove(jobId);
}

void DialogManager::showTaskDialog()
{
    m_taskDialog->show();
}


void DialogManager::abortJob(const QMap<QString, QString> &jobDetail)
{
    QString jobId = jobDetail.value("jobId");
    FileJob * job = m_jobs.value(jobId);
    job->setApplyToAll(true);
    job->setStatus(FileJob::Cancelled);
}


void DialogManager::showUrlWrongDialog(const DUrl &url)
{
    MessageWrongDialog d(url.toString());
    qDebug() << url;
    d.exec();
}

int DialogManager::showRenameNameSameErrorDialog(const QString &name, const FMEvent &event)
{
    DDialog d(WindowManager::getWindowById(event.windowId()));;
    d.setMessage(tr("\"%1\" already exists, please select a different name.").arg(name));
    QStringList buttonTexts;
    buttonTexts << tr("Confirm");
    d.addButtons(buttonTexts);
    d.setDefaultButton(0);
    d.setIcon(QIcon(":/images/dialogs/images/dialog-warning.svg"));
    int code = d.exec();
    return code;
}

int DialogManager::showDeleteFilesClearTrashDialog(const FMEvent &event)
{
    QString ClearTrash = tr("Are you sure to empty trash?");
    QString DeleteFileName = tr("Are you sure to delete %1?");
    QString DeleteFileItems = tr("Are you sure to delete %1 items?");

    DUrlList urlList = event.fileUrlList();

    QStringList buttonTexts;
    buttonTexts << tr("Cancel") << tr("Delete");

    DDialog d(WindowManager::getWindowById(event.windowId()));
    if (urlList.first() == DUrl::fromTrashFile("/") && event.source() == FMEvent::Menu && urlList.size() == 1){
        buttonTexts[1]= tr("Empty");
        d.setTitle(ClearTrash);
    }else if (urlList.first().isLocalFile() && event.source() == FMEvent::FileView && urlList.size() == 1){
        FileInfo f(urlList.first());
        d.setTitle(DeleteFileName.arg(f.displayName()));
    }else if (urlList.first().isLocalFile() && event.source() == FMEvent::FileView && urlList.size() > 1){
        d.setTitle(DeleteFileItems.arg(urlList.size()));
    }else if (urlList.first().isTrashFile() && event.source() == FMEvent::Menu && urlList.size() == 1){
        TrashFileInfo f(urlList.first());
        d.setTitle(DeleteFileName.arg(f.displayName()));
    }else if (urlList.first().isTrashFile() && event.source() == FMEvent::Menu && urlList.size() > 1){
        d.setTitle(DeleteFileItems.arg(urlList.size()));
    }else if (urlList.first().isTrashFile() && event.source() == FMEvent::FileView && urlList.size() == 1 ){
        TrashFileInfo f(urlList.first());
        d.setTitle(DeleteFileName.arg(f.displayName()));
    }else if (urlList.first().isTrashFile() && event.source() == FMEvent::FileView && urlList.size() > 1 ){
        d.setTitle(DeleteFileItems.arg(urlList.size()));
    }else{
        d.setTitle(DeleteFileItems.arg(urlList.size()));
    }
    d.setMessage(tr("This action cannot be restored"));
    d.addButtons(buttonTexts);
    d.setDefaultButton(1);
    d.setIcon(QIcon(":/images/images/user-trash-full.png"));
    int code = d.exec();
    return code;
}

int DialogManager::showRemoveBookMarkDialog(const FMEvent &event)
{
    DDialog d(WindowManager::getWindowById(event.windowId()));
    d.setTitle(tr("Sorry, unable to locate your bookmark directory, remove it?"));
    QStringList buttonTexts;
    buttonTexts << tr("Cancel") << tr("Remove bookmark");
    d.addButtons(buttonTexts);
    d.setDefaultButton(1);
    d.setIcon(fileIconProvider->getDesktopIcon("folder", 256));
    int code = d.exec();
    return code;
}

void DialogManager::showOpenWithDialog(const FMEvent &event)
{
    QWidget* w = WindowManager::getWindowById(event.windowId());
    if (w){
        OpenWithDialog* d = new OpenWithDialog(event.fileUrl());
        d->show();
    }
}

void DialogManager::showPropertyDialog(const FMEvent &event)
{
    QWidget* w = WindowManager::getWindowById(event.windowId());
    if (w){
        foreach (const DUrl& url, event.fileUrlList()) {
            PropertyDialog *dialog = new PropertyDialog(url);

            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->setWindowFlags(dialog->windowFlags()
                                   &~ Qt::WindowMaximizeButtonHint
                                   &~ Qt::WindowMinimizeButtonHint
                                   &~ Qt::WindowSystemMenuHint);
            dialog->setTitle("");
            dialog->setFixedSize(QSize(320, 480));
            dialog->show();
            QTimer::singleShot(100, dialog, &PropertyDialog::raise);
        }
    }
}

void DialogManager::showDiskErrorDialog(const QString & id, const QString & errorText)
{
    DDialog d;
    d.setTitle(id + ":" + errorText);
    QStringList buttonTexts;
    buttonTexts << tr("Ok");
    d.addButtons(buttonTexts);
    d.setDefaultButton(0);
    d.setIcon(QIcon(":/images/dialogs/images/dialog-warning.svg"));
    d.exec();
}

void DialogManager::showAboutDialog(const FMEvent &event)
{
    QWidget* w = WindowManager::getWindowById(event.windowId());
    QString icon(":/icons/images/system-file-manager.png");
    DAboutDialog *dialog = new DAboutDialog(icon,
                        icon,
                        tr("File Manager"),
                        tr("1.0"),
                        tr("File Manager is a file management tool independently "
                           "developed by Deepin Technology, featured with searching, "
                           "copying, trash, compression/decompression, file property "
                           "and other file management functions. "),w);
    const QPoint global = w->mapToGlobal(w->rect().center());
    dialog->move(global.x() - dialog->width() / 2, global.y() - dialog->height() / 2);
    dialog->show();
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
        case 0:job->started();break;
        case 1:
            job->started();
            job->setReplace(true);
            break;
        case 2:job->cancelled();break;
        default:
            qDebug() << "unknown code"<<code;
        }
    }
}
