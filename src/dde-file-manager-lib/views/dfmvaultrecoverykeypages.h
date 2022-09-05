// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dfmvaultpagebase.h"

#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class DFMVaultRecoveryKeyPagesPrivate;
class DFMVaultRecoveryKeyPages : public DFMVaultPageBase
{
    Q_OBJECT
public:
    explicit DFMVaultRecoveryKeyPages(QWidget *parent = nullptr);
    ~DFMVaultRecoveryKeyPages() override;

    void showAlertMessage(const QString &text, int duration = 3000);
private slots:
    void onButtonClicked(const int &index);

    void recoveryKeyChanged();

    void onUnlockVault(int state);
private:

    bool eventFilter(QObject *watched, QEvent *event) override;

    int afterRecoveryKeyChanged(QString &str);

    void showEvent(QShowEvent *event) override;
private:
    QPlainTextEdit *m_recoveryKeyEdit {nullptr};
    bool m_bUnlockByKey = false;

    QScopedPointer<DFMVaultRecoveryKeyPagesPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFMVaultRecoveryKeyPages)
};
