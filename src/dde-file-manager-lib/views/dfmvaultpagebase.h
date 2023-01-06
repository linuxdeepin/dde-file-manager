// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
