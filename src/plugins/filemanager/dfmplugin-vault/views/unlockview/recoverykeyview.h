// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTRECOVERYKEYPAGES_H
#define VAULTRECOVERYKEYPAGES_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>

#include <QScopedPointer>
#include <QFrame>

class QPlainTextEdit;

DWIDGET_BEGIN_NAMESPACE
class DToolTip;
class DFloatingWidget;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
class RecoveryKeyView : public QFrame
{
    Q_OBJECT
public:
    explicit RecoveryKeyView(QWidget *parent = nullptr);

    ~RecoveryKeyView() override;

    QStringList btnText();

    QString titleText();

    void buttonClicked(int index, const QString &text);

    void showAlertMessage(const QString &text, int duration = 3000);

signals:
    void sigBtnEnabled(const int &index, const bool &state);

    void sigCloseDialog();

private slots:

    void recoveryKeyChanged();

    void onUnlockVault(int state);

private:
    bool eventFilter(QObject *watched, QEvent *event) override;

    int afterRecoveryKeyChanged(QString &str);

    void showEvent(QShowEvent *event) override;

private:
    QPlainTextEdit *recoveryKeyEdit { nullptr };
    bool unlockByKey { false };

    DTK_WIDGET_NAMESPACE::DToolTip *tooltip { nullptr };
    DTK_WIDGET_NAMESPACE::DFloatingWidget *floatWidget { nullptr };
};
}
#endif   //VAULTRECOVERYKEYPAGES_H
