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
#include "dfmvaultpagebase.h"

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DToolTip;
class DFloatingWidget;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class DFMVaultUnlockPages : public DFMVaultPageBase
{
    Q_OBJECT
public:
    static DFMVaultUnlockPages *instance();

    enum EN_ToolTip {
        Warning = 0,
        Information
    };

public slots:
    void onButtonClicked(const int &index);

    void onPasswordChanged(const QString &pwd);

    void onVaultUlocked(int state);

private:
    explicit DFMVaultUnlockPages(QWidget *parent = nullptr);
    ~DFMVaultUnlockPages() override {}

    void showEvent(QShowEvent *event) override;

    void showToolTip(const QString &text, int duration, EN_ToolTip enType);

private:
    DPasswordEdit *m_passwordEdit {nullptr};
    QPushButton *m_tipsButton {nullptr};
    bool m_bUnlockByPwd = false;

    DToolTip *m_tooltip {nullptr};
    DFloatingWidget *m_frame {nullptr};
};
