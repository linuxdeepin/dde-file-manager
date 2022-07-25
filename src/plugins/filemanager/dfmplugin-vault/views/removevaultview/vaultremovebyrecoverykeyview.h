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

#ifndef VAULTREMOVEBYRECOVERYKEYVIEW_H
#define VAULTREMOVEBYRECOVERYKEYVIEW_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>

#include <QWidget>

class QPlainTextEdit;

DWIDGET_BEGIN_NAMESPACE
class DToolTip;
class DFloatingWidget;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
class VaultRemoveByRecoverykeyView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultRemoveByRecoverykeyView(QWidget *parent = nullptr);
    ~VaultRemoveByRecoverykeyView() override;

    /*!
    * /brief    获取凭证
    */
    QString getRecoverykey();

    /*!
    * /brief    清空凭证
    */
    void clear();

    void showAlertMessage(const QString &text, int duration = 3000);
public slots:
    void onRecoveryKeyChanged();

private:
    //! 输入凭证后，对凭证添加“-”
    int afterRecoveryKeyChanged(QString &str);

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QPlainTextEdit *keyEdit { nullptr };

    DTK_WIDGET_NAMESPACE::DToolTip *tooltip { nullptr };
    DTK_WIDGET_NAMESPACE::DFloatingWidget *floatWidget { nullptr };
};
}
#endif   // VAULTREMOVEBYRECOVERYKEYVIEW_H
