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
#include "dialogmanager.h"

#include "dfm-base/dialogs/mountpasswddialog/mountaskpassworddialog.h"
#include "dfm-base/dialogs/mountpasswddialog/mountsecretdiskaskpassworddialog.h"
#include "dfm-base/dialogs/settingsdialog/settingdialog.h"
#include "dfm-base/dialogs/taskdialog/taskdialog.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm_global_defines.h"
#include "dfm-base/file/local/localfilehandler.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/dfm_global_defines.h"

#include <QDir>

DFMBASE_USE_NAMESPACE

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
    DDialog d(title, message);
    Qt::WindowFlags flags = d.windowFlags();
    // dialog show top
    d.setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    d.setIcon(errorIcon);
    d.addButton(tr("Confirm", "button"), true, DDialog::ButtonRecommend);
    d.setMaximumWidth(640);
    d.exec();
}

int DialogManager::showMessageDialog(DialogManager::MessageType messageLevel, const QString &title, const QString &message, QString btnTxt)
{
    DDialog d(title, message);
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

void DialogManager::showErrorDialogWhenMountDeviceFailed(DFMMOUNT::DeviceError err)
{
    switch (err) {
    case DFMMOUNT::DeviceError::kUserErrorNetworkAnonymousNotAllowed:
        showErrorDialog(tr("Mount error"), tr("Anonymous mount is not allowed"));
        break;
    case DFMMOUNT::DeviceError::kUserErrorNetworkWrongPasswd:
        showErrorDialog(tr("Mount error"), tr("Wrong password is inputed"));
        break;
    case DFMMOUNT::DeviceError::kUserErrorUserCancelled:
        break;
    default:
        showErrorDialog(tr("Mount error"), tr("Error occured while mounting device"));
        qWarning() << "mount device failed: " << err;
        break;
    }
}

void DialogManager::showErrorDialogWhenUnmountDeviceFailed(DFMMOUNT::DeviceError err)
{
    switch (err) {
    case DFMMOUNT::DeviceError::kUDisksErrorDeviceBusy:
        showErrorDialog(tr("The device was not safely unmounted"), tr("The device is busy, cannot remove now"));
        break;
    default:
        showErrorDialog(tr("The device was not safely unmounted"), tr("The device is busy, cannot remove now"));
        break;
    }
}

void DialogManager::showNoPermissionDialog(const QList<QUrl> &urls)
{
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

        if (fm.width(message) > Global::kMaxFileNameCharCount) {
            message = fm.elidedText(message, Qt::ElideMiddle, Global::kMaxFileNameCharCount);
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
            if (fm.width(s) > Global::kMaxFileNameCharCount) {
                s = fm.elidedText(s, Qt::ElideMiddle, Global::kMaxFileNameCharCount);
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

    d.addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);
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

void DialogManager::showSetingsDialog(FileManagerWindow *window)
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

/*!
 * \brief DialogService::askPasswordForLockedDevice
 * \return the password user inputed.
 */
QString DialogManager::askPasswordForLockedDevice()
{
    MountSecretDiskAskPasswordDialog dialog(tr("Need password to unlock device"));
    return dialog.exec() == QDialog::Accepted ? dialog.getUerInputedPassword() : "";
}

bool DialogManager::askForFormat()
{
    DDialog dlg;
    dlg.setIcon(warningIcon);
    dlg.addButton(tr("Cancel", "button"));
    dlg.addButton(tr("Format", "button"), true, DDialog::ButtonRecommend);
    dlg.setTitle(tr("To access the device, you must format the disk first. Are you sure you want to format it now?"));
    dlg.setMaximumWidth(640);
    return dlg.exec() == QDialog::Accepted;
}

int DialogManager::showRunExcutableScriptDialog(const QUrl &url)
{
    DDialog d;
    const int maxDisplayNameLength = 250;

    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

    const QString &fileDisplayName = info->fileDisplayName();
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
    DDialog d;
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

    const int maxDisplayNameLength = 200;
    const QString &fileDisplayName = info->fileDisplayName();
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

int DialogManager::showDeleteFilesClearTrashDialog(const QList<QUrl> &urlList, const bool showEmptyBtText)
{
    if (urlList.isEmpty())
        return QDialog::Rejected;

    static QString ClearTrash = tr("Are you sure you want to empty %1 item?");
    static QString ClearTrashMutliple = tr("Are you sure you want to empty %1 items?");
    static QString DeleteFileName = tr("Permanently delete %1?");
    static QString DeleteFileItems = tr("Permanently delete %1 items?");

    const int maxFileNameWidth = Global::kMaxFileNameCharCount;

    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("Delete", "button"));

    QString title;
    QString fileName;
    QIcon icon(QIcon::fromTheme(kUserTrashFullOpened));
    bool isLocalFile = urlList.first().isLocalFile();
    if (showEmptyBtText) {
        buttonTexts[1] = tr("Empty");
        //const AbstractFileInfoPointer &fileInfo =InfoFactory::create<AbstractFileInfo>(urlList.first());//todo(zhuangshu)：add new function: filesCount()
        QString filePath = urlList.first().path();
        QDir dir(filePath);
        QStringList entryList = dir.entryList(QDir::AllEntries | QDir::System
                                              | QDir::NoDotAndDotDot | QDir::Hidden);
        int fCount = entryList.count();   //todo(zhuangshu)：add new function: filesCount()
        if (fCount == 1) {
            title = ClearTrash.arg(fCount);
        } else {
            title = ClearTrashMutliple.arg(fCount);
        }
    } else if (isLocalFile) {
        if (urlList.size() == 1) {
            LocalFileInfo f(urlList.first());
            fileName = f.fileDisplayName();
        } else {
            title = DeleteFileItems.arg(urlList.size());
        }
    } else {
        title = DeleteFileItems.arg(urlList.size());
    }

    DDialog d;
    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    QFontMetrics fm(d.font());
    if (!fileName.isEmpty()) {
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
    int code = d.exec();
    return code;
}

int DialogManager::showNormalDeleteConfirmDialog(const QList<QUrl> &urls)
{
    DDialog d;

    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    QFontMetrics fm(d.font());
    d.setIcon(QIcon::fromTheme("user-trash-full-opened"));

    QString deleteFileName = tr("Do you want to delete %1?");
    QString deleteFileItems = tr("Do you want to delete the selected %1 items?");

    const QUrl &urlFirst = urls.first();
    if (urlFirst.isLocalFile()) {   // delete local file
        if (urls.size() == 1) {
            AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(urlFirst);
            d.setTitle(deleteFileName.arg(fm.elidedText(info->fileDisplayName(), Qt::ElideMiddle, Global::kMaxFileNameCharCount)));
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

void DialogManager::showRestoreFailedDialog(const QList<QUrl> &urlList)
{
    DDialog d;
    d.setTitle(tr("Operation failed!"));
    if (urlList.count() == 1) {
        d.setMessage(tr("Failed to restore %1 file, the target folder is read-only").arg(QString::number(1)));
    } else if (urlList.count() > 1) {
        d.setMessage(tr("Failed to restore %1 files, the target folder is read-only").arg(QString::number(urlList.count())));
    }
    d.setIcon(warningIcon);
    d.addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);
    d.exec();
}

int DialogManager::showRenameNameSameErrorDialog(const QString &name)
{
    DDialog d;
    QFontMetrics fm(d.font());
    d.setTitle(tr("\"%1\" already exists, please use another name.").arg(fm.elidedText(name, Qt::ElideMiddle, 150)));
    QStringList buttonTexts;
    buttonTexts.append(tr("Confirm", "button"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    d.setIcon(warningIcon);
    int code = d.exec();
    return code;
}

void DialogManager::showRenameBusyErrDialog()
{
    DDialog d;
    QFontMetrics fm(d.font());
    d.setTitle(tr("Device or resource busy"));
    QStringList buttonTexts;
    buttonTexts.append(tr("Confirm", "button"));
    d.addButton(buttonTexts[0], true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    d.setIcon(warningIcon);
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
