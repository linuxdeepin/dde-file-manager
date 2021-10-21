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

#ifndef DFMVAULTREMOVEPAGES_H
#define DFMVAULTREMOVEPAGES_H

#include "dfmvaultpagebase.h"

#include <polkit-qt5-1/PolkitQt1/Authority>

class QStackedWidget;

class DFMVaultRemoveProgressView;
class DFMVaultRemoveByRecoverykeyView;
class DFMVaultRemoveByPasswordView;
DWIDGET_BEGIN_NAMESPACE

class DLabel;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

class DFMVaultRemovePages : public DFMVaultPageBase
{
    Q_OBJECT
public:
    static DFMVaultRemovePages *instance();

    void showTop() override;

public slots:
    void onButtonClicked(int index);

    void onLockVault(int state);

    void onVualtRemoveFinish(bool result);

private slots:
    // 异步授权时，此函数接收授权完成的结果
    void slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result);

private:
    explicit DFMVaultRemovePages(QWidget *parent = nullptr);
    ~DFMVaultRemovePages() override {}

    void initConnect();

    void showVerifyWidget();

    void showRemoveWidget();
    void setInfo(const QString &info);

    void closeEvent(QCloseEvent *event) override;


private:
    DFMVaultRemoveByPasswordView *m_passwordView {nullptr};
    DFMVaultRemoveByRecoverykeyView *m_recoverykeyView {nullptr};
    DFMVaultRemoveProgressView *m_progressView {nullptr};

    QStackedWidget *m_stackedWidget {nullptr};    //用于页面切换
    bool m_bRemoveVault = false;

    QLabel              *m_pInfo;
};

#endif // DFMVAULTREMOVEPAGES_H
