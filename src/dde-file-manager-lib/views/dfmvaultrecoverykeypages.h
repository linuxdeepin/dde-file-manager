/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
    static DFMVaultRecoveryKeyPages *instance();

    void showAlertMessage(const QString &text, int duration = 3000);
private slots:
    void onButtonClicked(const int &index);

    void recoveryKeyChanged();

    void onUnlockVault(int state);
private:
    explicit DFMVaultRecoveryKeyPages(QWidget *parent = nullptr);
    ~DFMVaultRecoveryKeyPages() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

    int afterRecoveryKeyChanged(QString &str);

    void showEvent(QShowEvent *event) override;
private:
    QPlainTextEdit *m_recoveryKeyEdit {nullptr};
    bool m_bUnlockByKey = false;

    QScopedPointer<DFMVaultRecoveryKeyPagesPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFMVaultRecoveryKeyPages)
};
