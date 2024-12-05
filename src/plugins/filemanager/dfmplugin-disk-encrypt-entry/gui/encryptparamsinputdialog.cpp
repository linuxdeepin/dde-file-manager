// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encryptparamsinputdialog.h"
#include "utils/encryptutils.h"

#include <dfm-base/utils/finallyutil.h>
#include <dfm-mount/dmount.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QStackedLayout>
#include <QDebug>
#include <QFutureWatcher>
#include <QEventLoop>
#include <QtConcurrent/QtConcurrent>
#include <QAbstractButton>

#include <DDialog>
#include <DPasswordEdit>
#include <DComboBox>
#include <DFileChooserEdit>
#include <DSpinner>

using namespace dfmplugin_diskenc;
using namespace disk_encrypt;
DWIDGET_USE_NAMESPACE

enum StepPage {
    kPasswordInputPage,
    kExportKeyPage,
};

EncryptParamsInputDialog::EncryptParamsInputDialog(const disk_encrypt::DeviceEncryptParam &params,
                                                   QWidget *parent)
    : DTK_WIDGET_NAMESPACE::DDialog(parent),
      params(params)
{
    exportRecKeyEnabled = config_utils::exportKeyEnabled();
    initUi();
    initConn();
    if (dialog_utils::isWayland())
        setWindowFlag(Qt::WindowStaysOnTopHint);
}

DeviceEncryptParam EncryptParamsInputDialog::getInputs()
{
    QString password;
    if (kTPMAndPIN == encType->currentIndex() || kTPMOnly == encType->currentIndex()) {
        password = tpmPassword;
        tpmPassword.clear();
    } else if (kPasswordOnly == encType->currentIndex()) {
        password = encKeyEdit1->text();
    }

    params.type = static_cast<SecKeyType>(encType->currentIndex());
    params.key = password;
    params.exportPath = keyExportInput->text();
    return params;
}

void EncryptParamsInputDialog::initUi()
{
    clearContents();
    setOnButtonClickedClose(false);
    setFixedSize(472, 304);
    setIcon(QIcon::fromTheme("drive-harddisk"));

    QWidget *center = new QWidget(this);
    center->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addContent(center);

    pagesLay = new QStackedLayout(this);
    center->setLayout(pagesLay);

    pagesLay->addWidget(createPasswordPage());
    pagesLay->addWidget(createExportPage());

    onPageChanged(kPasswordInputPage);

    // for ui debugging
    //    setStyleSheet("border: 1px solid red;");
    //    center->setObjectName("center");
    //    center->setStyleSheet("QWidget#center{border: 1px solid red;}");
}

void EncryptParamsInputDialog::initConn()
{
    connect(pagesLay, &QStackedLayout::currentChanged,
            this, &EncryptParamsInputDialog::onPageChanged);
    connect(this, &EncryptParamsInputDialog::buttonClicked,
            this, &EncryptParamsInputDialog::onButtonClicked);
    connect(encType, static_cast<void (DComboBox::*)(int)>(&DComboBox::currentIndexChanged),
            this, &EncryptParamsInputDialog::onEncTypeChanged);
    connect(keyExportInput, &DFileChooserEdit::textChanged,
            this, [this](const QString &path) { onExpPathChanged(path, false); });
}

QWidget *EncryptParamsInputDialog::createPasswordPage()
{
    QWidget *wid = new QWidget(this);
    QFormLayout *lay = new QFormLayout();
    lay->setContentsMargins(0, 10, 0, 0);
    wid->setLayout(lay);

    encType = new DComboBox(this);
    encType->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    lay->addRow(tr("Unlock type"), encType);

    unlockTypeHint = new QLabel(this); /*tr("User access to the partition is automatically "
                                "unlocked without passphrase checking.")*/
    unlockTypeHint->setWordWrap(true);
    lay->addRow("", unlockTypeHint);
    auto font = unlockTypeHint->font();
    font.setPixelSize(12);
    unlockTypeHint->setFont(font);
    unlockTypeHint->setFixedWidth(360);

    keyHint1 = new QLabel(this);
    encKeyEdit1 = new DPasswordEdit(this);
    keyHint1->setMinimumWidth(66);
    lay->addRow(keyHint1, encKeyEdit1);

    keyHint2 = new QLabel(this);
    encKeyEdit2 = new DPasswordEdit(this);
    lay->addRow(keyHint2, encKeyEdit2);

    encType->addItems({ tr("Unlocked by passphrase"),
                        tr("Use TPM+PIN to unlock on this computer (recommended)"),
                        tr("Automatic unlocking on this computer by TPM") });

    if (tpm_utils::checkTPM() != 0) {
        // encType->setItemData(kTPMAndPIN, QVariant(0), Qt::UserRole - 1);
        // encType->setItemData(kTPMOnly, QVariant(0), Qt::UserRole - 1);
        encType->removeItem(1);
        encType->removeItem(1);

        encType->setCurrentIndex(kPasswordOnly);
        onEncTypeChanged(kPasswordOnly);
    } else {
        encType->setCurrentIndex(kTPMAndPIN);
        onEncTypeChanged(kTPMAndPIN);
    }

    return wid;
}

QWidget *EncryptParamsInputDialog::createExportPage()
{
    QVBoxLayout *lay = new QVBoxLayout();
    QWidget *wid = new QWidget(this);
    wid->setLayout(lay);
    lay->setMargin(0);

    QLabel *hint = new QLabel(tr("In special cases such as forgetting the password or the encryption hardware is damaged, "
                                 "you can decrypt the encrypted partition with the recovery key, please export it to "
                                 "a non-encrypted partition and keep it in a safe place!"),
                              this);
    hint->setWordWrap(true);
    hint->adjustSize();
    lay->addWidget(hint);
    hint->setAlignment(Qt::AlignCenter);

    keyExportInput = new DFileChooserEdit(this);
    keyExportInput->setFileMode(QFileDialog::DirectoryOnly);
    if (keyExportInput->fileDialog() && dialog_utils::isWayland())
        keyExportInput->fileDialog()->setWindowFlag(Qt::WindowStaysOnTopHint);
    lay->addWidget(keyExportInput);

    keyExportInput->setPlaceholderText(tr("Please select a non-encrypted partition as the key file export path."));

    return wid;
}

bool EncryptParamsInputDialog::validatePassword()
{
    if (pagesLay->currentIndex() != kPasswordInputPage)
        return false;

    if (encType->currentIndex() == kTPMOnly)
        return true;

    QString pwd1 = encKeyEdit1->text().trimmed();
    QString pwd2 = encKeyEdit2->text().trimmed();

    QString keyType;
    if (encType->currentIndex() == kTPMAndPIN)
        keyType = "PIN";
    else if (encType->currentIndex() == kPasswordOnly)
        keyType = tr("Passphrase");

    QString hint = tr("%1 cannot be empty").arg(keyType);

    if (pwd1.isEmpty()) {
        encKeyEdit1->showAlertMessage(hint);
        return false;
    }

    if (pwd2.isEmpty()) {
        encKeyEdit2->showAlertMessage(hint);
        return false;
    }

    QList<QRegularExpression> regx {
        QRegularExpression { R"([A-Z])" },
        QRegularExpression { R"([a-z])" },
        QRegularExpression { R"([0-9])" },
        QRegularExpression { R"([^A-Za-z0-9])" }
    };

    int factor = 0;
    std::for_each(regx.cbegin(), regx.cend(), [&factor, pwd1](const QRegularExpression &reg) {
        if (pwd1.contains(reg))
            factor += 1;
    });

    if (factor < 3 || pwd1.length() < 8) {
        encKeyEdit1->showAlertMessage(tr("At least 8 bits, contains 3 types of A-Z, a-z, 0-9 and symbols"));
        return false;
    }

    if (pwd1 != pwd2) {
        encKeyEdit2->showAlertMessage(tr("%1 inconsistency").arg(keyType));
        return false;
    }

    return true;
}

bool EncryptParamsInputDialog::validateExportPath(const QString &path, QString *msg)
{
    auto setMsg = [&](const QString &info) { if (msg) *msg = info; };
    if (path.isEmpty()) {
        setMsg(tr("Recovery key export path cannot be empty!"));
        return false;
    }

    if (!QDir(path).exists()) {
        setMsg(tr("Recovery key export path is not exists!"));
        return false;
    }

    QStorageInfo storage(path);
    QString dev = storage.device();
    if (dev == params.devDesc) {
        setMsg(tr("Please export to an external device such as a non-encrypted partition or USB flash drive."));
        return false;
    }

    if (storage.isReadOnly()) {
        setMsg(tr("This partition is read-only, please export to a writable partition"));
        return false;
    }

    using namespace dfmmount;
    auto monitor = DDeviceManager::instance()->getRegisteredMonitor(DeviceType::kBlockDevice).objectCast<DBlockMonitor>();
    Q_ASSERT(monitor);
    auto devObjPaths = monitor->resolveDeviceNode(dev, {});
    if (!devObjPaths.isEmpty()) {
        auto objPath = devObjPaths.constFirst();
        auto devPtr = monitor->createDeviceById(objPath);
        if (devPtr && devPtr->getProperty(Property::kBlockCryptoBackingDevice).toString() != "/") {
            setMsg(tr("The partition is encrypted, please export to a non-encrypted "
                      "partition or external device such as a USB flash drive."));
            return false;
        }
    }

    return true;
}

void EncryptParamsInputDialog::setPasswordInputVisible(bool visible)
{
    keyHint1->setVisible(visible);
    keyHint2->setVisible(visible);
    encKeyEdit1->setVisible(visible);
    encKeyEdit2->setVisible(visible);
}

void EncryptParamsInputDialog::onButtonClicked(int idx)
{
    qDebug() << "button clicked:" << idx << "page: " << pagesLay->currentIndex();

    int currPage = pagesLay->currentIndex();
    if (currPage == kPasswordInputPage) {
        if (!validatePassword() && !params.initOnly)
            return;

        if (exportRecKeyEnabled) {
            pagesLay->setCurrentIndex(kExportKeyPage);
            onExpPathChanged(keyExportInput->text(), true);
        } else {
            confirmEncrypt();
        }
    } else if (currPage == kExportKeyPage) {
        if (idx == 0)
            pagesLay->setCurrentIndex(kPasswordInputPage);
        else if (idx == 1)
            confirmEncrypt();
    }
}

void EncryptParamsInputDialog::onPageChanged(int page)
{
    if (page > kExportKeyPage && page < kPasswordInputPage) {
        qWarning() << "invalid page index!" << page;
        return;
    }

    pagesLay->setCurrentIndex(page);
    clearButtons();
    if (page == kPasswordInputPage) {
        QString devName = QString("%1(%2)").arg(params.deviceDisplayName).arg(params.devDesc.mid(5));
        setTitle(tr("Please continue to encrypt partition %1").arg(devName));
        exportRecKeyEnabled ? addButton(tr("Next")) : addButton(tr("Confirm encrypt"));
        encKeyEdit1->setFocus();
    } else if (page == kExportKeyPage) {
        setTitle(tr("Export Recovery Key"));
        addButton(tr("Previous"));
        addButton(tr("Confirm encrypt"), true, ButtonType::ButtonRecommend);
        keyExportInput->setFocus();
    }
}

void EncryptParamsInputDialog::onEncTypeChanged(int type)
{
    QString filed1 = tr("Set %1");
    QString filed2 = tr("Repeat %1");
    QString placeholder1 = tr("At least 8 bits, contains 3 types of A-Z, a-z, 0-9 and symbols");
    QString placeholder2 = tr("Please enter the %1 again");

    if (type == kPasswordOnly) {
        setPasswordInputVisible(true);
        keyHint1->setText(filed1.arg(tr("passphrase")));
        keyHint2->setText(filed2.arg(tr("passphrase")));
        encKeyEdit1->setPlaceholderText(placeholder1);
        encKeyEdit2->setPlaceholderText(placeholder2.arg(tr("Passphrase")));
        unlockTypeHint->setText(tr("Access to the partition will be unlocked using a passphrase."));
    } else if (type == kTPMAndPIN) {
        setPasswordInputVisible(true);
        keyHint1->setText(filed1.arg(tr("PIN")));
        keyHint2->setText(filed2.arg(tr("PIN")));
        encKeyEdit1->setPlaceholderText(placeholder1);
        encKeyEdit2->setPlaceholderText(placeholder2.arg(tr("PIN")));
        unlockTypeHint->setText(tr("Access to the partition will be unlocked using the TPM security chip and PIN."));
    } else if (type == kTPMOnly) {
        setPasswordInputVisible(false);
        unlockTypeHint->setText(tr("Access to the partition will be automatically unlocked using the TPM security chip, "
                                   "no passphrase checking is required."));
    } else {
        qWarning() << "wrong encrypt type!" << type;
    }

    if (params.initOnly)
        setPasswordInputVisible(false);
}

void EncryptParamsInputDialog::onExpPathChanged(const QString &path, bool silent)
{
    auto btnNext = getButton(1);
    if (!btnNext)
        return;
    QString msg;
    btnNext->setEnabled(validateExportPath(path, &msg));
    if (!msg.isEmpty() && !silent)
        keyExportInput->showAlertMessage(msg);
}

bool EncryptParamsInputDialog::encryptByTpm(const QString &deviceName)
{
    auto btns = this->getButtons();
    for (auto btn : btns)
        btn->setEnabled(false);
    dfmbase::FinallyUtil finalClear([=] {
        for (auto btn : btns)
            btn->setEnabled(true);
    });

    QString sessionHashAlgo, sessionKeyAlgo, primaryHashAlgo, primaryKeyAlgo, minorHashAlgo, minorKeyAlgo;
    bool checkAlgo = tpm_passphrase_utils::getAlgorithm(&sessionHashAlgo,
                                                        &sessionKeyAlgo,
                                                        &primaryHashAlgo,
                                                        &primaryKeyAlgo,
                                                        &minorHashAlgo,
                                                        &minorKeyAlgo);
    if (!checkAlgo) {
        qCritical() << "TPM algo choice failed!";
        return false;
    }

    DSpinner spinner(this);
    spinner.setFixedSize(50, 50);
    spinner.move((width() - spinner.width()) / 2, (height() - spinner.height()) / 2);
    spinner.start();
    spinner.show();

    QString pin = (encType->currentIndex() == SecKeyType::kTPMAndPIN)
            ? encKeyEdit1->text()
            : "";
    int exitCode = tpm_passphrase_utils::genPassphraseFromTPM_NonBlock(deviceName, pin, &tpmPassword);
    if (exitCode != 0) {
        qCritical() << "TPM encrypt failed!";
        // dialog_utils::showTPMError(tr("Encrypt failed"),
        //                            static_cast<tpm_passphrase_utils::TPMError>(exitCode));
        return false;
    }
    return true;
}

void EncryptParamsInputDialog::confirmEncrypt()
{
    if (encType->currentIndex() == kPasswordOnly) {
        accept();
        return;
    }
    if (!params.initOnly && !encryptByTpm(params.devDesc)) {
        qWarning() << "encrypt by TPM failed!";
        if (tpm_utils::ownerAuthStatus() == 1) {
            QString msg = tr("TPM is locked and cannot be used for partition encryption. "
                             "Please cancel the TPM password or choose another unlocking method.");
            dialog_utils::showDialog(tr("TPM error"), msg, dialog_utils::DialogType::kError);
            return;
        }
        dialog_utils::showDialog(tr("TPM error"), tr("TPM status error!"), dialog_utils::DialogType::kError);
        return;
    }
    accept();
}
