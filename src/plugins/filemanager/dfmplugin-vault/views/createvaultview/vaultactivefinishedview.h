// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTACTIVEFINISHEDVIEW_H
#define VAULTACTIVEFINISHEDVIEW_H

#include "dfmplugin_vault_global.h"
#include "vaultbaseview.h"

#include <dtkwidget_global.h>
#include <DSuggestButton>

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
class VaultActiveFinishedView : public VaultBaseView
{
    Q_OBJECT
public:
    explicit VaultActiveFinishedView(QWidget *parent = nullptr);

    void setFinishedBtnEnabled(bool b);

Q_SIGNALS:
    void reqEncryptVault();

public slots:
    void encryptFinished(bool success, const QString &msg);
    void setProgressValue(int value);

private slots:
    void slotEncryptVault();
    void slotTimeout();
    //! 异步授权时，此函数接收授权完成的结果
    void slotCheckAuthorizationFinished(bool result);

private:
    void initUi();
    void initUiForSizeMode();
    void initConnect();

private:
    DTK_WIDGET_NAMESPACE::DLabel *titleLabel { Q_NULLPTR };
    QWidget *widgetOne { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *tipsLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *encryVaultImageLabel { nullptr };

    QWidget *widgetTow { nullptr };
    DTK_WIDGET_NAMESPACE::DWaterProgress *waterProgress { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *tipsLabelone { nullptr };

    QWidget *widgetThree { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *encryptFinishedImageLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *tipsThree { nullptr };

    DTK_WIDGET_NAMESPACE::DSuggestButton *finishedBtn { nullptr };

    QVBoxLayout *vLayout { nullptr };
};
}
#endif   // VAULTACTIVEFINISHEDVIEW_H
