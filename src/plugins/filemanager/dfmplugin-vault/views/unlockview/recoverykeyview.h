/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

DPVAULT_BEGIN_NAMESPACE
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
DPVAULT_END_NAMESPACE
#endif   //VAULTRECOVERYKEYPAGES_H
