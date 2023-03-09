// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTACTIVEFINISHEDVIEW_H
#define VAULTACTIVEFINISHEDVIEW_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>
#include <DPushButton>

#include <polkit-qt5-1/PolkitQt1/Authority>

#include <QWidget>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DWaterProgress;
class DLabel;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
class VaultActiveFinishedView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultActiveFinishedView(QWidget *parent = nullptr);

    void setFinishedBtnEnabled(bool b);

signals:
    void sigAccepted();

public slots:
    //! 连接创建保险箱返回信号
    void slotEncryptComplete(int nState);

private slots:
    void slotEncryptVault();
    void slotTimeout();
    //! 异步授权时，此函数接收授权完成的结果
    void slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result);

protected:
    void showEvent(QShowEvent *event) override;

private:
    void initUi();
    void initConnect();

private:
    QWidget *widgetOne { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *tipsLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *encryVaultImageLabel { nullptr };

    QWidget *widgetTow { nullptr };
    DTK_WIDGET_NAMESPACE::DWaterProgress *waterProgress { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *tipsLabelone { nullptr };

    QWidget *widgetThree { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *encryptFinishedImageLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *tipsThree { nullptr };

    DTK_WIDGET_NAMESPACE::DPushButton *finishedBtn { nullptr };

    QVBoxLayout *vLayout { nullptr };

    //! 辅助进度条，实现加密过程效果
    QTimer *timer { nullptr };
};
}
#endif   // VAULTACTIVEFINISHEDVIEW_H
