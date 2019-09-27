/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#pragma once

#include "interface/dfmvaultcontentinterface.h"
#include "dtkwidget_global.h"

#include <QPushButton>

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
class QLineEdit;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DFloatingButton;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class VaultVerifyUserPage : public QWidget
{
    Q_OBJECT
public:
    explicit VaultVerifyUserPage(QWidget * parent = nullptr);
    ~VaultVerifyUserPage() override;

signals:
    void requestRedirect(DUrl url);

private slots:
    void unlock();

private:
    DPasswordEdit * m_passwordEdit;
    DFloatingButton * m_unlockButton;
};

// ------------------------------------------------

class VaultGeneratedKeyPage : public QWidget
{
    Q_OBJECT
public:
    explicit VaultGeneratedKeyPage(QWidget * parent = nullptr);
    ~VaultGeneratedKeyPage() override;

signals:
    void requestRedirect(DUrl url);

public slots:
    void startKeyGeneration();
    void saveKeyFile();
    void finishButtonPressed();

private:
    void clearData();
    DSecureString createRecoveryKeyString(const DSecureString &ivHexString, const DSecureString &keyHexString);

    QPushButton * m_saveFileButton;
    QPushButton * m_finishButton;
    QPlainTextEdit * m_generatedKeyEdit;
};

// ------------------------------------------------

class VaultVerifyRecoveryKeyPage : public QWidget
{
    Q_OBJECT
public:
    explicit VaultVerifyRecoveryKeyPage(QWidget * parent = nullptr);
    ~VaultVerifyRecoveryKeyPage() override;

signals:
    void requestRedirect(DUrl url);

public slots:
    void preparePage();
    void startVerifyKey();

private:
    bool verifyKey();

    QByteArray m_ivData;
    QByteArray m_encryptedPasswordData;

    QLineEdit * m_verifyKeyEdit;
    QLineEdit * m_recoveryKeyEdit;
    QPushButton * m_retrievePasswordButton;
};

// ------------------------------------------------

class VaultPasswordPage : public QWidget
{
    Q_OBJECT
public:
    explicit VaultPasswordPage(QWidget * parent = nullptr);
    ~VaultPasswordPage() override;

signals:
    void requestRedirect(DUrl url);

public slots:
    void showPassword();
    void quitPasswordPage();

private:
    QLineEdit * m_passwordEdit;
    QPushButton * m_finishButton;
};

// ------------------------------------------------

class DFMVaultRecoveryKeyPages : public DFMVaultPages
{
    Q_OBJECT
public:
    explicit DFMVaultRecoveryKeyPages(QWidget * parent = nullptr);
    ~DFMVaultRecoveryKeyPages() override;

    QPair<DUrl, bool> requireRedirect(VaultController::VaultState state) override;
    QString pageString(const DUrl & url) override;

signals:
    void requestCreateRecoveryKey();
    void requestPrepareRetrievePasswordPage();
    void requestShowPassword();

public slots:
    void onRootPageChanged(QString pageStr);
};

DFM_END_NAMESPACE
