// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTREMOVEPROGRESSVIEW_H
#define VAULTREMOVEPROGRESSVIEW_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>

#include <QWidget>
#include <QHBoxLayout>

DWIDGET_BEGIN_NAMESPACE
class DWaterProgress;
class DLabel;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
class VaultRemoveProgressView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultRemoveProgressView(QWidget *parent = nullptr);
    ~VaultRemoveProgressView();

    QStringList btnText();
    QString titleText();
    void buttonClicked(int index, const QString &text);
    void removeVault(const QString &basePath);

Q_SIGNALS:
    void sigCloseDialog();
    void setBtnEnable(int index, bool enable);

private Q_SLOTS:
    void handleVaultRemovedProgress(int value);

private:
    QWidget *deletingWidget { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DWaterProgress *vaultRmProgressBar { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *hintLabel { Q_NULLPTR };

    QWidget *deletedWidget { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *deleteFinishedImageLabel { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *finishedLabel { Q_NULLPTR };

    QVBoxLayout *layout { Q_NULLPTR };

    bool isExecuted { false };
};
}
#endif   // VAULTREMOVEPROGRESSVIEW_H
