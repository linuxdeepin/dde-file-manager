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

#include <QPushButton>
#include <dtkwidget_global.h>

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class VaultSetupWelcomePage : public QWidget
{
    Q_OBJECT
public:
    explicit VaultSetupWelcomePage(QWidget * parent = nullptr);
    ~VaultSetupWelcomePage() override;

signals:
    void requestRedirect(DUrl url);

private:
    QPushButton * m_createNewButton;
    QPushButton * m_importButton;
};

// --------------------------------------

class VaultSetupSetPasswordPage : public QWidget
{
    Q_OBJECT
public:
    explicit VaultSetupSetPasswordPage(QWidget * parent = nullptr);
    ~VaultSetupSetPasswordPage() override;

signals:
    void requestRedirect(DUrl url);

public slots:
    void onFinishButtonPressed();

    void slotSetup(int state);

private:
    DPasswordEdit * m_enterPassword;
    DPasswordEdit * m_confirmPassword;
    QPushButton * m_nextButton;
};

// --------------------------------------

class DFMVaultSetupPages : public DFMVaultPages
{
    Q_OBJECT
public:
    DFMVaultSetupPages(QWidget * parent = nullptr);
    ~DFMVaultSetupPages() override {}

    QPair<DUrl, bool> requireRedirect(VaultController::VaultState state) override;
    QString pageString(const DUrl & url) override;
};

DFM_END_NAMESPACE
