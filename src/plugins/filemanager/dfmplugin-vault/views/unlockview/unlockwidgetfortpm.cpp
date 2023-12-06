// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unlockwidgetfortpm.h"
#include "dbus/vaultdbusutils.h"
#include "utils/vaulthelper.h"
#include "utils/vaultautolock.h"
#include "utils/fileencrypthandle.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/encryption/vaultconfig.h"
#include "events/vaulteventcaller.h"

#include <DFontSizeManager>
#include <DSpinner>

#include <QMouseEvent>
#include <QtConcurrent/QtConcurrent>

using namespace dfmplugin_vault;
DWIDGET_USE_NAMESPACE

UnlockWidgetForTpm::UnlockWidgetForTpm(QWidget *parent) : QWidget(parent)
{
    initUI();
    initConnect();
}
QStringList UnlockWidgetForTpm::btnText() const
{
    return { tr("Cancel"), tr("Unlock") };
}

QString UnlockWidgetForTpm::titleText() const
{
    return QString(tr("Unlock File Vault"));
}

void UnlockWidgetForTpm::buttonClicked(int index, const QString &text)
{
    Q_UNUSED(text)

    switch (index) {
    case 0: {
        emit sigCloseDialog();
    } break;
    case 1: {
        emit sigBtnEnabled(1, false);

        int nLeftoverErrorTimes = VaultDBusUtils::getLeftoverErrorInputTimes();
        if (nLeftoverErrorTimes < 1) {
            int nNeedWaitMinutes = VaultDBusUtils::getNeedWaitMinutes();
            pinEdit->showAlertMessage(tr("Please try again %1 minutes later").arg(nNeedWaitMinutes));
            emit sigBtnEnabled(1, true);
            return;
        }

        QString pinCode = pinEdit->text();
        QString strCipher("");
        if (!OperatorCenter::getInstance()->checkPassword(pinCode, strCipher)) {
            pinEdit->setAlert(true);
            VaultDBusUtils::leftoverErrorInputTimesMinusOne();
            nLeftoverErrorTimes = VaultDBusUtils::getLeftoverErrorInputTimes();
            if (nLeftoverErrorTimes < 1) {
                VaultDBusUtils::startTimerOfRestorePasswordInput();
                int nNeedWaitMinutes = VaultDBusUtils::getNeedWaitMinutes();
                pinEdit->showAlertMessage(tr("Wrong password, please try again %1 minutes later").arg(nNeedWaitMinutes));
            } else {
                if (nLeftoverErrorTimes == 1)
                    pinEdit->showAlertMessage(tr("Wrong password, one chance left"));
                else
                    pinEdit->showAlertMessage(tr("Wrong password, %1 chances left").arg(nLeftoverErrorTimes));
            }
            emit sigBtnEnabled(1, true);
            return;
        }

        QString psw;
        VaultConfig config;
        const QString hashAlgo = config.get(kConfigNodeNameOfTPM, kConfigKeyPrimaryHashAlgo).toString();
        const QString keyAlgo = config.get(kConfigNodeNameOfTPM, kConfigKeyPrimaryKeyAlgo).toString();
        if (hashAlgo.isEmpty() || keyAlgo.isEmpty()) {
            fmCritical() << "Vault: get tpm algo failed!";
            return;
        }
        QVariantMap map {
            { "PropertyKey_EncryptType", 2 },
            { "PropertyKey_PrimaryHashAlgo", hashAlgo },
            { "PropertyKey_PrimaryKeyAlgo", keyAlgo },
            { "PropertyKey_DirPath", kVaultBasePath },
            { "PropertyKey_PinCode", pinCode }
        };

        QFutureWatcher<int> watcher;
        QEventLoop loop;
        QFuture<int> future = QtConcurrent::run([map, &psw]{
            if (!VaultEventCaller::checkTPMAvailable())
                return 1;

            if (!VaultEventCaller::decryptByTPM(map, &psw))
                return 2;

            return 0;
        });
        connect(&watcher, &QFutureWatcher<bool>::finished, this, [&watcher, &loop]{
            loop.exit(watcher.result());
        });
        watcher.setFuture(future);

        DSpinner spinner(this);
        spinner.setFixedSize(50, 50);
        spinner.move((width() - spinner.width())/2, (height() - spinner.height())/2);
        spinner.start();
        spinner.show();

        emit setAllowClose(false);
        int result = loop.exec();
        emit setAllowClose(true);
        if (result == 1) {
            infoLabel->setText(tr("TPM can not use!"));
            emit sigBtnEnabled(1, true);
            return;
        } else if (result == 2) {
            infoLabel->setText(tr("TPM decrypt failed!"));
            emit sigBtnEnabled(1, true);
            return;
        }

        VaultDBusUtils::restoreLeftoverErrorInputTimes();
        VaultDBusUtils::restoreNeedWaitMinutes();
        if (!VaultHelper::instance()->unlockVault(psw)) {
            infoLabel->setText(tr("Cryfs decrypt failed!"));
            emit sigBtnEnabled(1, true);
        }
    } break;
    }
}

void UnlockWidgetForTpm::vaultUnlocked(int state)
{
    switch (state) {
    case 0: {
        VaultEventCaller::sendItemActived(VaultHelper::instance()->currentWindowId(),
                                          VaultHelper::instance()->rootUrl());
        VaultHelper::recordTime(kjsonGroupName, kjsonKeyInterviewItme);
        VaultAutoLock::instance()->slotUnlockVault(state);
        emit sigCloseDialog();
    } break;
    case 1: {

    } break;
    default: break;
    }
}

void UnlockWidgetForTpm::pinEditFocusChanged(bool onFocus)
{
    if (onFocus) {
        pinEdit->setAlert(false);
    }
}

void UnlockWidgetForTpm::showHintInfo()
{
    QString hint { "" };
    if (OperatorCenter::getInstance()->getPasswordHint(hint)) {
        if (!hint.isEmpty()) {
            hint = tr("PIN hint: ") + hint;
            pinEdit->showAlertMessage(hint);
        }
    }
}

void UnlockWidgetForTpm::pinEditTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        emit sigBtnEnabled(1, false);
    } else {
        emit sigBtnEnabled(1, true);
    }
}

bool UnlockWidgetForTpm::eventFilter(QObject *obj, QEvent *evt)
{
    if (obj == forgetPin) {
        if (evt->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(evt);
            if (mouseEvent->button() == Qt::LeftButton)
                emit signalJump(PageType::kRetrievePage);
        }
    }

    return QWidget::eventFilter(obj, evt);
}

void UnlockWidgetForTpm::initUI()
{
    pinEdit = new DPasswordEdit(this);
    pinEdit->lineEdit()->setPlaceholderText(tr("PIN code"));
    pinEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    tipsBtn = new DPushButton(this);
    tipsBtn->setIcon(QIcon(":/icons/images/icons/light_32px.svg"));

    QHBoxLayout *pinLay = new QHBoxLayout;
    pinLay->setContentsMargins(0, 0, 0, 0);
    pinLay->setMargin(0);
    pinLay->addWidget(pinEdit);
    pinLay->addWidget(tipsBtn);

    forgetPin = new DLabel(tr("Forget pin?"));
    DFontSizeManager::instance()->bind(forgetPin, DFontSizeManager::T8, QFont::Medium);
    forgetPin->setForegroundRole(DPalette::ColorType::LightLively);
    forgetPin->installEventFilter(this);

    infoLabel = new DLabel(this);
    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::red);
    infoLabel->setPalette(pe);
    infoLabel->setText("");

    QVBoxLayout *mainLay = new QVBoxLayout;
    mainLay->addItem(pinLay);
    mainLay->addWidget(forgetPin, 0, Qt::AlignRight);
    mainLay->addWidget(infoLabel, 0, Qt::AlignHCenter);
    setLayout(mainLay);
}

void UnlockWidgetForTpm::initConnect()
{
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalUnlockVault,
             this, &UnlockWidgetForTpm::vaultUnlocked);
    connect(pinEdit, &DPasswordEdit::focusChanged,
            this, &UnlockWidgetForTpm::pinEditFocusChanged);
    connect(pinEdit, &DPasswordEdit::textChanged,
            this, &UnlockWidgetForTpm::pinEditTextChanged);
    connect(tipsBtn, &QPushButton::clicked, this, &UnlockWidgetForTpm::showHintInfo);
}
