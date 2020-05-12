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

//#include "interface/dfmvaultcontentinterface.h"
#include "dtkwidget_global.h"
#include <DDialog>

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class DFMVaultUnlockPages : public DDialog
{
    Q_OBJECT
public:        
    static DFMVaultUnlockPages *instance();

public slots:
    void onButtonClicked(const int &index);

    void onPasswordChanged(const QString &pwd);

    void onVaultUlocked(int state);
private:
    DFMVaultUnlockPages(QWidget * parent = nullptr);
    ~DFMVaultUnlockPages() override {}

private:
    DPasswordEdit * m_passwordEdit;
    QPushButton * m_helpButton;
};
