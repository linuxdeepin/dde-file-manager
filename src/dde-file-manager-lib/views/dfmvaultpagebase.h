/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include <dtkwidget_global.h>
#include <DDialog>

DWIDGET_USE_NAMESPACE

class DFMVaultPageBase : public DDialog
{
    Q_OBJECT
public:
    explicit DFMVaultPageBase(QWidget *parent = nullptr);

    void setWndPtr(QWidget *wnd);
    QWidget *getWndPtr() const;

    virtual void showTop();

public slots:
    void enterVaultDir();

protected:
    // 重载基类关闭事件
    void closeEvent(QCloseEvent *event) override;

protected:
    QWidget *m_wndptr = nullptr; //! current window pointer.
};
