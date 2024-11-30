// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialogmanager.h"

#include <dfm-base/dialogs/mountpasswddialog/mountaskpassworddialog.h>
#include <dfm-base/dialogs/mountpasswddialog/mountsecretdiskaskpassworddialog.h>
#include <dfm-base/dialogs/settingsdialog/settingdialog.h>
#include <dfm-base/dialogs/taskdialog/taskdialog.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/utils/fileutils.h>

#include <QDir>
#include <QApplication>

using namespace dfmbase;

static const QString kUserTrashFullOpened = "user-trash-full-opened";

DialogManager *DialogManager::instance()
{
    static DialogManager ins;
    return &ins;
}

DDialog *DialogManager::showQueryScanningDialog(const QString &title)
{   // stop scan device
    DDialog *d = new DDialog;
    d->setTitle(title);
    d->setAttribute(Qt::WA_DeleteOnClose);
    Qt::WindowFlags flags = d->windowFlags();
    d->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    d->setIcon(warningIcon);
    d->addButton(QObject::tr("Cancel", "button"));
    d->addButton(QObject::tr("Stop", "button"), true, DDialog::ButtonWarning);   // 终止
    d->setMaximumWidth(640);
    d->show();
    return d;
}

void DialogManager::showErrorDialog(const QString &title, const QString &message)
{
    DDialog d(title, message, qApp->activeWindow());
    Qt::WindowFlags flags = d.windowFlags();
    // dialog show top
    d.setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    d.setIcon(errorIcon);
    d.addButton(tr("Confirm", "button"), true, DDialog::ButtonNormal);
    d.setMaximumWidth(640);
    d.exec();
}

int DialogManager::showMessageDialog(DialogManager::MessageType messageLevel, const QString &title, const QString &message, QString btnTxt)
{
    DDialog d(title, message, qApp->activeWindow());
    d.moveToCenter();
    QStringList buttonTexts;
    buttonTexts.append(btnTxt);
    d.addButtons(buttonTexts);
    d.setDefaultButton(0);
    if (messageLevel == kMsgWarn) {
        d.setIcon(warningIcon);
    } else if (messageLevel == kMsgErr) {
        d.setIcon(errorIcon);
    } else {
        d.setIcon(infoIcon);
    }
    int code = d.exec();
    return code;
}

void DialogManager::showErrorDialogWhenOperateDeviceFailed(OperateType type, DFMMOUNT::OperationErrorInfo err)
{
    static const QString kOpFailed = tr("Operating failed");
    static const QString kMountFailed = tr("Mount failed");
    static const QString kUnmountFailed = tr("Unmount failed");

    DFM_MOUNT_USE_NS

    switch (err.code) {
    case DeviceError::kUDisksBusyFileSystemUnmounting:
        showErrorDialog(kOpFailed, tr("Unmounting device now..."));
        return;
    case DeviceError::kUDisksBusyFileSystemMounting:
        showErrorDialog(kOpFailed, tr("Mounting device now..."));
        return;
    case DeviceError::kUDisksBusyFormatErasing:
        showErrorDialog(kOpFailed, tr("Erasing device now..."));
        return;
    case DeviceError::kUDisksBusyFormatMkfsing:
        showErrorDialog(kOpFailed, tr("Making filesystem for device now..."));
        return;
    case DeviceError::kUDisksBusyEncryptedLocking:
        showErrorDialog(kOpFailed, tr("Locking device now..."));
        return;
    case DeviceError::kUDisksBusyEncryptedUnlocking:
        showErrorDialog(kOpFailed, tr("Unlocking device now..."));
        return;

    case DeviceError::kUDisksBusySMARTSelfTesting:
    case DeviceError::kUDisksBusyDriveEjecting:
    case DeviceError::kUDisksBusyEncryptedModifying:
    case DeviceError::kUDisksBusyEncryptedResizing:
    case DeviceError::kUDisksBusySwapSpaceStarting:
    case DeviceError::kUDisksBusySwapSpaceStoping:
    case DeviceError::kUDisksBusySwpaSpaceModifying:
    case DeviceError::kUDisksBusyFileSystemModifying:
    case DeviceError::kUDisksBusyFileSystemResizing:
    case DeviceError::kUDisksBusyLoopSetuping:
    case DeviceError::kUDisksBusyPartitionModifying:
    case DeviceError::kUDisksBusyPartitionDeleting:
    case DeviceError::kUDisksBusyPartitionCreating:
    case DeviceError::kUDisksBusyCleanuping:
    case DeviceError::kUDisksBusyATASecureErasing:
    case DeviceError::kUDisksBusyATAEnhancedSecureErasing:
    case DeviceError::kUDisksBusyMdRaidStarting:
    case DeviceError::kUDisksBusyMdRaidStoping:
    case DeviceError::kUDisksBusyMdRaidFaultingDevice:
    case DeviceError::kUDisksBusyMdRaidRemovingDevice:
    case DeviceError::kUDisksBusyMdRaidCreating:
        showErrorDialog(kOpFailed, tr("The device is busy now"));
        return;
    default:
        break;
    }

    QString errMsg = "", title = "";
    if (type == OperateType::kMount) {
        title = kMountFailed;
        qCWarning(logDFMBase) << "mount device failed: " << err.code << err.message;

        if (err.code == DeviceError::kUserErrorNetworkAnonymousNotAllowed)
            errMsg = tr("Anonymous mount is not allowed");
        else if (err.code == DeviceError ::kUserErrorNetworkWrongPasswd)
            errMsg = tr("Wrong password");
        else if (err.code == DeviceError::kUserErrorUserCancelled)
            errMsg.clear();
        else if (err.code == DeviceError::kDaemonErrorCannotMkdirMountPoint)
            errMsg = tr("Cannot create the mountpoint: the file name is too long");
        else if (static_cast<int>(err.code) == EACCES)
            errMsg = tr("Permission denied");
        else if (static_cast<int>(err.code) == ENOENT)
            errMsg = tr("No such file or directory");
        else if (err.code >= DeviceError::kGIOError
                 && err.code <= DeviceError::kGIOErrorMessageTooLarge)
            errMsg = err.message;
        else
            errMsg = tr("Error occured while mounting device");

        if (err.message.contains("Operation not permitted.")) {   // TASK(222725)
            errMsg = tr("The device has been blocked and you do not have permission to access it. "
                        "Please configure its connection policy in Security Center or contact your administrator.");
        }
    } else if (type == OperateType::kRemove || type == OperateType::kUnmount) {
        title = kUnmountFailed;
        errMsg = tr("The device is busy, cannot remove now");
    }

    if (!errMsg.isEmpty())
        showErrorDialog(title, errMsg);
}

void DialogManager::showNoPermissionDialog(const QList<QUrl> &urls)
{
    qCDebug(logDFMBase) << urls << "no perssion";
    if (urls.isEmpty()) {
        return;
    }

    QFont f;
    f.setPixelSize(16);
    QFontMetrics fm(f);

    DDialog d(qApp->activeWindow());

    if (urls.count() == 1) {

        d.setTitle(tr("You do not have permission to operate file/folder!"));
        QString message = urls.at(0).toLocalFile();

        if (fm.horizontalAdvance(message) > NAME_MAX) {
            message = fm.elidedText(message, Qt::ElideMiddle, NAME_MAX);
        }

        d.setMessage(message);
        d.setIcon(warningIcon);
    } else {

        QFrame *contentFrame = new QFrame;

        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(warningIcon.pixmap(64, 64));

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
            if (fm.horizontalAdvance(s) > NAME_MAX) {
                s = fm.elidedText(s, Qt::ElideMiddle, NAME_MAX);
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

    d.addButton(tr("OK", "button"), true, DDialog::ButtonNormal);
    d.exec();
}

void DialogManager::showCopyMoveToSelfDialog()
{
    DDialog d(qApp->activeWindow());
    d.setTitle(tr("Operation failed!"));
    d.setMessage(tr("Target folder is inside the source folder!"));
    QStringList buttonTexts;
    buttonTexts.append(tr("OK", "button"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.setDefaultButton(0);
    d.setIcon(warningIcon);
    d.exec();
}

/*!
 * \brief DialogService::addTask 添加一个文件操作任务，当收到这个任务的线程结束时，自己移除掉这个任务
 *
 * \param task 文件操作任务的处理器
 */
void DialogManager::addTask(const JobHandlePointer task)
{
    if (!taskdialog)
        taskdialog = new TaskDialog();

    taskdialog->addTask(task);
}

void DialogManager::registerSettingWidget(const QString &viewType,
                                          std::function<DSettingsWidgetFactory::WidgetCreateHandler> handler)
{
    settingWidgetCreators[viewType] = handler;
}

void DialogManager::showSetingsDialog(FileManagerWindow *window)
{
    Q_ASSERT(window);

    if (window->property("isSettingDialogShown").toBool()) {
        qCWarning(logDFMBase) << "isSettingDialogShown true";
        return;
    }

    window->setProperty("isSettingDialogShown", true);
    SettingDialog *dsd = new SettingDialog(window);

    // 在初始化前注册所有缓存的自定义控件创建器
    auto factory = dsd->widgetFactory();
    for (auto iter = settingWidgetCreators.constBegin();
         iter != settingWidgetCreators.constEnd();
         ++iter) {
        factory->registerWidget(iter.key(), iter.value());
    }

    dsd->initialze();
    dsd->show();

    connect(dsd, &DSettingsDialog::finished, [window] {
        window->setProperty("isSettingDialogShown", false);
    });
}

/*!
 * \brief DialogService::askPasswordForLockedDevice
 * \return the password user inputed.
 */
QString DialogManager::askPasswordForLockedDevice(const QString &devName)
{
    MountSecretDiskAskPasswordDialog dialog(tr("The passphrase is needed to access encrypted data on %1.").arg(devName), qApp->activeWindow());
    return dialog.exec() == QDialog::Accepted ? dialog.getUerInputedPassword() : "";
}

bool DialogManager::askForFormat()
{
    DDialog dlg(qApp->activeWindow());
    dlg.setIcon(warningIcon);
    dlg.addButton(tr("Cancel", "button"));
    dlg.addButton(tr("Format", "button"), true, DDialog::ButtonRecommend);
    dlg.setTitle(tr("To access the device, you must format the disk first. Are you sure you want to format it now?"));
    dlg.setMaximumWidth(640);
    return dlg.exec() == QDialog::Accepted;
}

int DialogManager::showRunExcutableScriptDialog(const QUrl &url)
{
    DDialog d(qApp->activeWindow());
    const int maxDisplayNameLength = 250;

    FileInfoPointer info = InfoFactory::create<FileInfo>(url);

    const QString &fileDisplayName = info->displayOf(DisPlayInfoType::kFileDisplayName);
    const QString &fileDisplayNameNew = d.fontMetrics().elidedText(fileDisplayName, Qt::ElideRight, maxDisplayNameLength);
    const QString &message = tr("Do you want to run %1 or display its content?").arg(fileDisplayNameNew);
    const QString &tipMessage = tr("It is an executable text file.");
    QStringList buttonTexts;

    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("Run", "button"));
    buttonTexts.append(tr("Run in terminal", "button"));
    buttonTexts.append(tr("Display", "button"));

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

int DialogManager::showRunExcutableFileDialog(const QUrl &url)
{
    DDialog d(qApp->activeWindow());
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);

    const int maxDisplayNameLength = 200;
    const QString &fileDisplayName = info->displayOf(DisPlayInfoType::kFileDisplayName);
    const QString &fileDisplayNameNew = d.fontMetrics().elidedText(fileDisplayName, Qt::ElideRight, maxDisplayNameLength);
    const QString &message = tr("Do you want to run %1?").arg(fileDisplayNameNew);
    const QString &tipMessage = tr("It is an executable file.");

    d.addButton(tr("Cancel", "button"));
    d.addButton(tr("Run in terminal", "button"));
    d.addButton(tr("Run", "button"), true, DDialog::ButtonRecommend);
    d.setTitle(message);
    d.setMessage(tipMessage);
    d.setIcon(info->fileIcon());
    int code = d.exec();
    return code;
}

int DialogManager::showDeleteFilesDialog(const QList<QUrl> &urlList, bool isTrashFile)
{
    if (urlList.isEmpty())
        return QDialog::Rejected;

    QString DeleteFileItems = isTrashFile ? tr("Cannot move the selected %1 items to the trash. Do you want to permanently delete them?")
                                          : tr("Permanently delete %1 items?");

    const int maxFileNameWidth = NAME_MAX;

    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("Delete", "button"));

    QString title;
    QString fileName;
    QIcon icon(QIcon::fromTheme(kUserTrashFullOpened));
    bool isLocalFile = dfmbase::FileUtils::isLocalFile(urlList.first());
    if (isLocalFile) {
        if (urlList.size() == 1) {
            SyncFileInfo f(urlList.first());
            fileName = f.displayOf(DisPlayInfoType::kFileDisplayName);
        } else {
            title = DeleteFileItems.arg(urlList.size());
        }
    } else {
        title = DeleteFileItems.arg(urlList.size());
    }

    DDialog d(qApp->activeWindow());
    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    QFontMetrics fm(d.font());
    if (!fileName.isEmpty()) {
        QString DeleteFileName = isTrashFile ? tr("Cannot move \"%1\" to the trash. Do you want to permanently delete it?")
                                             : tr("Permanently delete %1?");
        title = DeleteFileName.arg(fm.elidedText(fileName, Qt::ElideMiddle, maxFileNameWidth));
    }

    d.setIcon(icon);
    d.setTitle(title);
    d.setMessage(tr("This action cannot be undone"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.moveToCenter();
    d.setFixedWidth(480);
    int code = d.exec();
    return code;
}

int DialogManager::showClearTrashDialog(const quint64 &count)
{
    static QString ClearTrash = tr("Are you sure you want to empty %1 item?");
    static QString ClearTrashMutliple = tr("Are you sure you want to empty %1 items?");

    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("Delete", "button"));

    QString title;
    QIcon icon(QIcon::fromTheme(kUserTrashFullOpened));

    buttonTexts[1] = tr("Empty");

    if (count == 1)
        title = ClearTrash.arg(count);
    else
        title = ClearTrashMutliple.arg(count);

    DDialog d(qApp->activeWindow());
    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    d.setIcon(icon);
    d.setTitle(title);
    d.setMessage(tr("This action cannot be undone"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.moveToCenter();
    int code = d.exec();
    return code;
}

int DialogManager::showNormalDeleteConfirmDialog(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return QDialog::Rejected;

    DDialog d(qApp->activeWindow());

    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    QFontMetrics fm(d.font());
    d.setIcon(QIcon::fromTheme("user-trash-full-opened"));

    QString deleteFileName = tr("Do you want to delete %1?");
    QString deleteFileItems = tr("Do you want to delete the selected %1 items?");

    const QUrl &urlFirst = urls.first();
    if (dfmbase::FileUtils::isLocalFile(urlFirst)) {   // delete local file
        if (urls.size() == 1) {
            FileInfoPointer info = InfoFactory::create<FileInfo>(urlFirst);
            d.setTitle(deleteFileName.arg(fm.elidedText(info->displayOf(DisPlayInfoType::kFileDisplayName), Qt::ElideMiddle, NAME_MAX)));
        } else {
            d.setTitle(deleteFileItems.arg(urls.size()));
        }
    } else {
        d.setTitle(deleteFileItems.arg(urls.size()));
    }

    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("Delete", "button"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.moveToCenter();

    return d.exec();
}

void DialogManager::showRestoreFailedDialog(const int count)
{
    DDialog d(qApp->activeWindow());
    d.setTitle(tr("Operation failed!"));
    if (count == 1) {
        d.setMessage(tr("Failed to restore %1 file, the target folder is read-only").arg(QString::number(1)));
    } else if (count > 1) {
        d.setMessage(tr("Failed to restore %1 files, the target folder is read-only").arg(QString::number(count)));
    }
    d.setIcon(warningIcon);
    d.addButton(tr("OK", "button"), true, DDialog::ButtonNormal);
    d.exec();
}

int DialogManager::showRestoreDeleteFilesDialog(const QList<QUrl> &urlList)
{
    if (urlList.isEmpty())
        return QDialog::Rejected;

    QString alertSingleFile {
        tr("After revocation, it will be completely deleted %1, do you want to delete it completely?")
    };
    QString alertMultiFiles {
        tr("These %1 contents will be completely deleted after revocation. Do you want to delete them completely?")
    };

    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("Delete", "button"));

    QString title;
    if (urlList.size() == 1) {
        SyncFileInfo file(urlList.first());
        const QString &fileName { file.displayOf(DisPlayInfoType::kFileDisplayName) };
        if (!fileName.isEmpty())
            title = alertSingleFile.arg(fileName);
    }

    if (title.isEmpty())
        title = alertMultiFiles.arg(urlList.size());

    DDialog d(qApp->activeWindow());
    if (!d.parentWidget())
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);

    QFontMetrics fm(d.font());
    d.setIcon(QIcon::fromTheme(kUserTrashFullOpened));
    d.setTitle(title);
    d.setMessage(tr("This operation cannot be reversed."));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.moveToCenter();
    d.setFixedWidth(480);

    int code = d.exec();
    return code;
}

int DialogManager::showRenameNameSameErrorDialog(const QString &name)
{
    DDialog d(qApp->activeWindow());
    QFontMetrics fm(d.font());
    d.setTitle(tr("\"%1\" already exists, please use another name.").arg(fm.elidedText(name, Qt::ElideMiddle, 150)));
    QStringList buttonTexts;
    buttonTexts.append(tr("Confirm", "button"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.setDefaultButton(0);
    d.setIcon(warningIcon);
    int code = d.exec();
    return code;
}

void DialogManager::showRenameBusyErrDialog()
{
    DDialog d(qApp->activeWindow());
    QFontMetrics fm(d.font());
    d.setTitle(tr("Device or resource busy"));
    QStringList buttonTexts;
    buttonTexts.append(tr("Confirm", "button"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.setDefaultButton(0);
    d.setIcon(warningIcon);
    d.exec();
}

int DialogManager::showRenameNameDotBeginDialog()
{
    DDialog d(qApp->activeWindow());
    QFontMetrics fm(d.font());
    d.setTitle(tr("This file will be hidden if the file name starts with '.'. Do you want to hide it?"));
    d.addButton(tr("Hide"), true, DDialog::ButtonWarning);
    d.addButton(tr("Cancel"));

    d.setDefaultButton(0);
    d.setIcon(warningIcon);

    int ret = -1;
    connect(&d, &DDialog::buttonClicked, this, [=, &ret](int index, const QString &text) {
        Q_UNUSED(text)
        if (index == 0)
            ret = 1;
    });
    d.exec();
    return ret;
}

int DialogManager::showUnableToVistDir(const QString &dir)
{
    // Ensure that only one dialog is displayed in the current screen
    static bool showFlag = true;
    int code = -1;
    if (showFlag) {
        showFlag = false;
        DDialog d(qApp->activeWindow());
        d.setTitle(tr("Unable to access %1").arg(dir));
        d.setMessage(" ");
        QStringList buttonTexts;
        buttonTexts.append(tr("Confirm", "button"));
        d.addButton(buttonTexts[0], true);
        d.setDefaultButton(0);
        d.setIcon(QIcon::fromTheme("folder").pixmap(64, 64));
        code = d.exec();
        showFlag = true;
    }

    return code;
}

DFMBASE_NAMESPACE::GlobalEventType DialogManager::showBreakSymlinkDialog(const QString &targetName, const QUrl &linkfile)
{
    DDialog d(qApp->activeWindow());
    QString warnText = tr("%1 that this shortcut refers to has been changed or moved");
    QFontMetrics fm(d.font());
    QString _targetName = fm.elidedText(targetName, Qt::ElideMiddle, 120);
    d.setTitle(warnText.arg(_targetName));
    d.setMessage(tr("Do you want to delete this shortcut？"));
    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("Confirm", "button"));
    d.addButton(buttonTexts[0], true);
    d.addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    d.setDefaultButton(1);
    d.setIcon(warningIcon);
    int code = d.exec();
    if (code == 1) {
        QList<QUrl> urls;
        urls << linkfile;
        if (Q_UNLIKELY(FileUtils::isTrashFile(linkfile))) {
            return DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles;
        } else {
            return DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash;
        }
    }
    return DFMBASE_NAMESPACE::GlobalEventType::kUnknowType;
}

int DialogManager::showAskIfAddExcutableFlagAndRunDialog()
{
    DDialog d(qApp->activeWindow());
    // i18n text from: https://github.com/linuxdeepin/internal-discussion/issues/456 , seems a little weird..
    QString message = tr("This file is not executable, do you want to add the execute permission and run?");
    d.addButton(tr("Cancel", "button"));
    d.addButton(tr("Run", "button"), true, DDialog::ButtonRecommend);
    d.setTitle(message);
    d.setIcon(warningIcon);
    int code = d.exec();
    return code;
}

void DialogManager::showDeleteSystemPathWarnDialog(quint64 winId)
{
    DDialog d(FMWindowsIns.findWindowById(winId));
    d.setTitle(tr("The selected files contain system file/directory, and it cannot be deleted"));
    d.setIcon(warningIcon);
    d.addButton(tr("OK", "button"), true, DDialog::ButtonNormal);
    d.exec();
}

DialogManager::DialogManager(QObject *parent)
    : QObject(parent)
{
    infoIcon = QIcon::fromTheme("dialog-information");
    warningIcon = QIcon::fromTheme("dialog-warning");
    errorIcon = QIcon::fromTheme("dialog-error");
}

DialogManager::~DialogManager()
{
}
