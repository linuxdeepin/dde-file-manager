// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    explicit DFMVaultRemovePages(QWidget *parent = nullptr);
    ~DFMVaultRemovePages() override {}

    void showTop() override;

public slots:
    void onButtonClicked(int index);

    void onLockVault(int state);

    void onVualtRemoveFinish(bool result);

private slots:
    // 异步授权时，此函数接收授权完成的结果
    void slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result);

private:

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
