// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encryptprogressdialog.h"
#include "utils/encryptutils.h"
#include "dfmplugin_disk_encrypt_global.h"

#include <QCoreApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QStackedLayout>
#include <QIcon>
#include <QStorageInfo>
#include <QDebug>
#include <DFileDialog>

#include <dfm-mount/dmount.h>

using namespace dfmplugin_diskenc;

EncryptProgressDialog::EncryptProgressDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
}

void EncryptProgressDialog::setText(const QString &title, const QString &message)
{
    setTitle(title);
    this->message->setText(message);
}

void EncryptProgressDialog::updateProgress(double progress)
{
    mainLay->setCurrentIndex(0);
    int val = progress * 100 - 1;   // do not show 100%, wait at 99% and then jump to result page.
    if (val < 0) val = 0;
    this->progress->setValue(val);
    clearButtons();
    if (closeButtonVisible())
        setCloseButtonVisible(false);
}

void EncryptProgressDialog::showResultPage(bool success, const QString &title, const QString &message)
{
    this->progress->setValue(100);
    mainLay->setCurrentIndex(1);

    setTitle(title);
    resultMsg->setText(message);
    QIcon icon = success ? QIcon::fromTheme("dialog-ok") : QIcon::fromTheme("dialog-error");
    iconLabel->setPixmap(icon.pixmap(64, 64));

    addButton(tr("Confirm"));
    setAttribute(Qt::WA_DeleteOnClose);
    setOnButtonClickedClose(true);
}

void EncryptProgressDialog::showExportPage()
{
    clearButtons();
    addButton(tr("Re-export the recovery key"));
    setOnButtonClickedClose(false);
    setCloseButtonVisible(false);

    warningLabel->setText(tr("*Recovery key saving failed, please re-save the recovery "
                             "key to a non-encrypted partition and keep it in a safe place!"));
    warningLabel->setVisible(true);

    connect(this, &EncryptProgressDialog::buttonClicked,
            this, &EncryptProgressDialog::onCicked);
}

void EncryptProgressDialog::onCicked(int idx, const QString &btnTxt)
{
    if (btnTxt != tr("Re-export the recovery key")) {
        fmDebug() << "Non-export button clicked, ignoring";
        return;
    }

    QUrl url = DFileDialog::getExistingDirectoryUrl(this);
    QString msg;
    if (!validateExportPath(url.toLocalFile(), &msg)) {
        fmWarning() << "Export path validation failed:" << msg;
        dialog_utils::showDialog(tr("Error"), msg);
    } else {
        saveRecKey(url.toLocalFile());
    }
}

void EncryptProgressDialog::initUI()
{
    clearContents();
    setIcon(QIcon::fromTheme("drive-harddisk-root"));
    setFixedWidth(400);

    QFrame *frame = new QFrame(this);
    mainLay = new QStackedLayout(frame);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);
    addContent(frame);

    QFrame *progressPage = new QFrame(this);
    QVBoxLayout *progressLay = new QVBoxLayout(progressPage);
    progressLay->setSpacing(30);
    progressLay->setContentsMargins(0, 30, 0, 20);

    progress = new DWaterProgress(this);
    progress->setFixedSize(64, 64);
    progress->setValue(1);
    progressLay->addWidget(progress, 0, Qt::AlignCenter);
    progress->start();

    const int kMessageLabelLen = 380;

    message = new QLabel(this);
    message->setMinimumWidth(kMessageLabelLen);
    message->setWordWrap(true);
    message->setAlignment(Qt::AlignCenter);
    progressLay->addWidget(message, 0, Qt::AlignCenter);

    QFrame *resultPage = new QFrame(this);
    QVBoxLayout *resultLay = new QVBoxLayout(resultPage);
    resultLay->setSpacing(20);
    resultLay->setContentsMargins(0, 30, 0, 0);

    iconLabel = new QLabel(this);
    iconLabel->setFixedSize(64, 64);
    resultLay->addWidget(iconLabel, 0, Qt::AlignCenter);

    resultMsg = new QLabel(this);
    resultMsg->setMinimumWidth(kMessageLabelLen);
    resultMsg->setWordWrap(true);
    resultMsg->setAlignment(Qt::AlignCenter);
    resultLay->addWidget(resultMsg, 0, Qt::AlignCenter);

    warningLabel = new QLabel(this);
    warningLabel->setMinimumWidth(kMessageLabelLen);
    resultLay->addWidget(warningLabel);
    QPalette pal = warningLabel->palette();
    pal.setColor(QPalette::WindowText, QColor("red"));
    warningLabel->setPalette(pal);
    warningLabel->setWordWrap(true);
    warningLabel->setAlignment(Qt::AlignLeft);
    warningLabel->setVisible(false);

    mainLay->addWidget(progressPage);
    mainLay->addWidget(resultPage);
}

bool EncryptProgressDialog::validateExportPath(const QString &path, QString *msg)
{
    auto setMsg = [&](const QString &info) { if (msg) *msg = info; };
    if (path.isEmpty()) {
        setMsg(tr("Recovery key export path cannot be empty!"));
        return false;
    }

    if (!QDir(path).exists()) {
        fmWarning() << "Export path does not exist:" << path;
        setMsg(tr("Recovery key export path is not exists!"));
        return false;
    }

    QStorageInfo storage(path);
    if (storage.isReadOnly()) {
        fmWarning() << "Export path is read-only:" << path;
        setMsg(tr("This partition is read-only, please export to a writable "
                  "partition"));
        return false;
    }

    // Check if the export path itself is encrypted
    using namespace dfmmount;
    auto monitor = DDeviceManager::instance()->getRegisteredMonitor(DeviceType::kBlockDevice).objectCast<DBlockMonitor>();
    Q_ASSERT(monitor);
    auto devObjPaths = monitor->resolveDeviceNode(storage.device(), {});
    if (!devObjPaths.isEmpty()) {
        auto objPath = devObjPaths.constFirst();
        auto devPtr = monitor->createDeviceById(objPath);
        if (devPtr && devPtr->getProperty(Property::kBlockCryptoBackingDevice).toString() != "/") {
            fmWarning() << "Export path is on an encrypted partition:" << path;
            setMsg(tr("The partition is encrypted, please export to a non-encrypted "
                      "partition or external device such as a USB flash drive."));
            return false;
        }
    }

    return true;
}

void EncryptProgressDialog::saveRecKey(const QString &path)
{
    QString recFileName = QString("%1/%2_recovery_key.txt")
                                  .arg(path)
                                  .arg(device.mid(5));
    QFile f(recFileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        fmCritical() << "Failed to create recovery key file:" << recFileName << "error:" << f.errorString();
        dialog_utils::showDialog(tr("Error"), tr("Cannot create recovery key file!"));
        return;
    }
    f.write(recKey.toLocal8Bit());
    f.close();
    accept();
    fmInfo() << "Recovery key successfully saved to:" << recFileName;
}
